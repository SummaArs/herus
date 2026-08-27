"""Fail-closed structural audit for an audio/transcript/intent package.

The auditor never downloads data, never maps external intents to HERUS, and
never treats a label as proof that two files belong to the same sample. It is
usable with a locally obtained package only after the dataset manifest allows
local research use.
"""
from __future__ import annotations

import argparse
import csv
import hashlib
import json
import sys
import wave
from dataclasses import asdict, dataclass
from pathlib import Path
from typing import Iterable


class GateError(RuntimeError):
    def __init__(self, code: str, detail: str):
        super().__init__(f"{code}: {detail}")
        self.code = code
        self.detail = detail


@dataclass(frozen=True)
class AudioRecord:
    sample_id: str
    speaker_id: str
    audio_path: str
    transcript: str
    action: str
    object: str
    location: str


@dataclass(frozen=True)
class AudioAudit:
    dataset: str
    csv_path: str
    data_root: str
    csv_sha256: str
    audio_sha256_aggregate: str
    records_checked: int
    unique_sample_ids: int
    unique_speakers: int
    wav_mono_16khz: int
    herus_mapping_count: int
    verdict: str
    reason: str


ALIASES = {
    "speaker_id": ("speaker_id", "speakerId", "speaker", "speaker-id"),
    "audio_path": ("path", "file_path", "filepath", "wav", "filename", "audio"),
    "transcript": ("transcription", "transcript", "text", "utterance"),
    "action": ("action",),
    "object": ("object",),
    "location": ("location",),
}


def sha256_file(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as handle:
        for block in iter(lambda: handle.read(1024 * 1024), b""):
            digest.update(block)
    return digest.hexdigest()


def _column(fieldnames: Iterable[str], aliases: tuple[str, ...], role: str) -> str:
    normalised = {name.strip().lower(): name for name in fieldnames if name}
    for alias in aliases:
        if alias.lower() in normalised:
            return normalised[alias.lower()]
    raise GateError("schema_mismatch", f"missing column for {role}")


def _safe_relative(path_text: str) -> str:
    path = Path(path_text.replace("\\", "/"))
    if not path_text or path.is_absolute() or ".." in path.parts:
        raise GateError("unsafe_audio_path", path_text or "empty path")
    return path.as_posix()


def load_records(csv_path: Path, limit: int | None) -> list[AudioRecord]:
    try:
        handle = csv_path.open("r", encoding="utf-8", newline="")
    except FileNotFoundError as exc:
        raise GateError("missing", str(csv_path)) from exc
    with handle:
        reader = csv.DictReader(handle)
        fields = reader.fieldnames or []
        columns = {
            role: _column(fields, aliases, role)
            for role, aliases in ALIASES.items()
        }
        records: list[AudioRecord] = []
        for row_number, row in enumerate(reader, start=2):
            if limit is not None and len(records) >= limit:
                break
            values = {role: (row.get(column) or "").strip() for role, column in columns.items()}
            if any(not value for value in values.values()):
                raise GateError("empty_field", f"row={row_number}")
            audio_path = _safe_relative(values["audio_path"])
            records.append(
                AudioRecord(
                    sample_id=audio_path,
                    speaker_id=values["speaker_id"],
                    audio_path=audio_path,
                    transcript=values["transcript"],
                    action=values["action"],
                    object=values["object"],
                    location=values["location"],
                )
            )
    if not records:
        raise GateError("empty_package", str(csv_path))
    return records


def verify_package(
    manifest_path: Path,
    dataset_id: str,
    csv_path: Path,
    data_root: Path,
    limit: int | None = None,
) -> AudioAudit:
    try:
        manifest = json.loads(manifest_path.read_text(encoding="utf-8"))
    except FileNotFoundError as exc:
        raise GateError("missing", str(manifest_path)) from exc
    dataset = next((item for item in manifest.get("datasets", []) if item.get("id") == dataset_id), None)
    if dataset is None:
        raise GateError("unknown_dataset", dataset_id)
    if not dataset.get("license_confirmed_for_raw_data", False):
        raise GateError("license_unverified", dataset_id)
    if not csv_path.exists():
        raise GateError("missing", str(csv_path))
    records = load_records(csv_path, limit)
    sample_ids = [record.sample_id for record in records]
    if len(set(sample_ids)) != len(sample_ids):
        raise GateError("duplicate_sample_id", dataset_id)

    valid_wav = 0
    audio_digests: list[str] = []
    for record in records:
        wav_path = data_root / record.audio_path
        if not wav_path.exists():
            raise GateError("missing_audio", str(wav_path))
        try:
            with wave.open(str(wav_path), "rb") as wav:
                if wav.getnchannels() != 1 or wav.getframerate() != 16000:
                    raise GateError("wav_format_mismatch", record.sample_id)
                if wav.getnframes() <= 0:
                    raise GateError("empty_audio", record.sample_id)
        except wave.Error as exc:
            raise GateError("wav_decode_error", f"{record.sample_id}: {exc}") from exc
        audio_digests.append(sha256_file(wav_path))
        valid_wav += 1

    aggregate = hashlib.sha256("\n".join(audio_digests).encode("ascii")).hexdigest()
    return AudioAudit(
        dataset=dataset_id,
        csv_path=str(csv_path),
        data_root=str(data_root),
        csv_sha256=sha256_file(csv_path),
        audio_sha256_aggregate=aggregate,
        records_checked=len(records),
        unique_sample_ids=len(set(sample_ids)),
        unique_speakers=len({record.speaker_id for record in records}),
        wav_mono_16khz=valid_wav,
        herus_mapping_count=0,
        verdict="paired_structure_verified",
        reason="Same CSV audio paths resolved to non-empty mono 16 kHz WAV files; external slots remain benchmark-only.",
    )


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--manifest", type=Path, required=True)
    parser.add_argument("--dataset", required=True)
    parser.add_argument("--csv", type=Path, required=True)
    parser.add_argument("--data-root", type=Path, required=True)
    parser.add_argument("--limit", type=int, default=None)
    args = parser.parse_args(argv)
    try:
        result = verify_package(args.manifest, args.dataset, args.csv, args.data_root, args.limit)
    except GateError as exc:
        print(json.dumps({"verdict": "blocked", "code": exc.code, "detail": exc.detail}, ensure_ascii=False))
        return 2
    print(json.dumps(asdict(result), ensure_ascii=False, indent=2))
    return 0


if __name__ == "__main__":
    sys.exit(main())
