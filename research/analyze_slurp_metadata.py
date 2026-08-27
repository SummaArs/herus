"""Audit the bounded SLURP metadata member without exposing identifiers.

The input is a locally extracted metadata.json member obtained via HTTP Range.
This script reports schema and aggregate counts only. It does not read audio,
execute archive members, infer HERUS commands, or publish raw IDs.
"""
from __future__ import annotations

import argparse
import collections
import hashlib
import json
from pathlib import Path
from typing import Any


HERUS_EVENTS = {"ARRIVE", "HELP", "CANCEL"}
SENSITIVE_KEY_FRAGMENTS = ("text", "transcript", "utterance", "sentence", "email", "name")


def sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as handle:
        for block in iter(lambda: handle.read(1024 * 1024), b""):
            digest.update(block)
    return digest.hexdigest()


def collect_keys(value: Any, counts: collections.Counter[str]) -> None:
    if isinstance(value, dict):
        for key, child in value.items():
            counts[str(key).lower()] += 1
            collect_keys(child, counts)
    elif isinstance(value, list):
        for child in value:
            collect_keys(child, counts)


def audit(path: Path) -> dict[str, object]:
    payload = json.loads(path.read_text(encoding="utf-8"))
    if not isinstance(payload, dict):
        raise ValueError("metadata_root_not_object")

    status_counts: collections.Counter[str] = collections.Counter()
    suffix_counts: collections.Counter[str] = collections.Counter()
    user_ids: set[str] = set()
    recording_ids: set[str] = set()
    answer_ids: set[str] = set()
    recordings = 0
    records_with_recordings = 0
    key_counts: collections.Counter[str] = collections.Counter()
    collect_keys(payload, key_counts)

    for entry in payload.values():
        if not isinstance(entry, dict):
            continue
        entry_recordings = entry.get("recordings")
        if not isinstance(entry_recordings, dict):
            continue
        records_with_recordings += 1
        for filename, metadata in entry_recordings.items():
            recordings += 1
            suffix_counts[Path(str(filename)).suffix.lower()] += 1
            if not isinstance(metadata, dict):
                continue
            status_counts[str(metadata.get("status", ""))] += 1
            if metadata.get("usrid") is not None:
                user_ids.add(str(metadata["usrid"]))
            if metadata.get("recid") is not None:
                recording_ids.add(str(metadata["recid"]))
            if metadata.get("ansid") is not None:
                answer_ids.add(str(metadata["ansid"]))

    sensitive_keys_present = sorted(
        key for key in key_counts
        if not key.isdigit() and not key.endswith(".flac")
        and any(fragment in key for fragment in SENSITIVE_KEY_FRAGMENTS)
    )
    field_key_counts = {
        key: count for key, count in sorted(key_counts.items())
        if not key.isdigit() and not key.endswith(".flac")
    }
    numeric_key_count = sum(count for key, count in key_counts.items() if key.isdigit())
    filename_key_count = sum(count for key, count in key_counts.items() if key.endswith(".flac"))
    return {
        "source": "SLURP Zenodo metadata member",
        "path": str(path),
        "bytes": path.stat().st_size,
        "sha256": sha256(path),
        "top_level_records": len(payload),
        "records_with_recordings": records_with_recordings,
        "recordings": recordings,
        "unique_user_ids": len(user_ids),
        "unique_recording_ids": len(recording_ids),
        "unique_answer_ids": len(answer_ids),
        "status_counts": dict(sorted(status_counts.items())),
        "file_suffix_counts": dict(sorted(suffix_counts.items())),
        "field_key_count": len(field_key_counts),
        "field_key_counts": field_key_counts,
        "numeric_key_count_redacted": numeric_key_count,
        "filename_key_count_redacted": filename_key_count,
        "sensitive_key_names_present": bool(sensitive_keys_present),
        "herus_automatic_mapping_count": 0,
        "audio_loaded": False,
        "verdict": {
            "metadata_schema_observed": True,
            "aligned_audio_transcript_intent_proven": False,
            "herus_convergence_proven": False,
            "reason": "The member exposes recording metadata and IDs but this audit did not load audio or a transcript/intent annotation package; no HERUS label mapping is inferred."
        },
    }


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("metadata", type=Path)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()
    result = audit(args.metadata)
    args.output.write_text(json.dumps(result, indent=2, ensure_ascii=False) + "\n", encoding="utf-8")
    print(json.dumps(result, indent=2, ensure_ascii=False))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
