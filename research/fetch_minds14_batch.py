"""Audit a small deterministic batch of official MInDS-14 records.

Audio is downloaded one record at a time into a temporary directory, validated
structurally, passed only through the production C parser's transcription path,
and deleted before exit. The result contains aggregates and a batch digest,
never sentences, labels by name, original paths, or signed URLs.
"""
from __future__ import annotations

import argparse
import hashlib
import json
import tempfile
import urllib.parse
from pathlib import Path
from typing import Any

from fetch_minds14_sample import (
    API_URL,
    EXPECTED_CONFIG,
    EXPECTED_DATASET,
    EXPECTED_SPLIT,
    fetch_json,
    fetch_limited,
    parse_wav_header,
    redacted_hash,
    run_c_parser,
    sha256_bytes,
)


MAX_BATCH = 16


def sample_url(offset: int) -> str:
    return (
        "https://datasets-server.huggingface.co/rows?"
        f"dataset=PolyAI%2Fminds14&config={urllib.parse.quote(EXPECTED_CONFIG)}"
        f"&split={EXPECTED_SPLIT}&offset={offset}&length=1"
    )


def audit(offsets: list[int], c_runner: Path) -> dict[str, Any]:
    if not offsets or len(offsets) > MAX_BATCH or any(offset < 0 for offset in offsets):
        raise ValueError("invalid_offsets")
    if len(set(offsets)) != len(offsets):
        raise ValueError("duplicate_offsets")
    records: list[dict[str, Any]] = []
    with tempfile.TemporaryDirectory(prefix="herus-minds14-") as directory:
        root = Path(directory)
        for offset in offsets:
            payload = fetch_json(sample_url(offset))
            features = payload.get("features")
            if not isinstance(features, list) or not features:
                raise ValueError("features_missing")
            feature_names = {feature.get("name") for feature in features if isinstance(feature, dict)}
            if not {"path", "audio", "transcription", "intent_class", "lang_id"}.issubset(feature_names):
                raise ValueError("feature_schema_mismatch")
            if not isinstance(payload.get("rows"), list) or len(payload["rows"]) != 1:
                raise ValueError("unexpected_row_count")
            if payload["rows"][0].get("row_idx") != offset:
                raise ValueError("row_offset_mismatch")
            row = payload["rows"][0].get("row")
            if not isinstance(row, dict):
                raise ValueError("row_not_object")
            for field in ("path", "transcription", "intent_class", "lang_id", "audio"):
                if field not in row:
                    raise ValueError(f"missing_field:{field}")
            if not isinstance(row["transcription"], str) or not row["transcription"]:
                raise ValueError("empty_transcription")
            audio_cells = row["audio"]
            if not isinstance(audio_cells, list) or len(audio_cells) != 1:
                raise ValueError("audio_cell_missing_or_ambiguous")
            audio_cell = audio_cells[0]
            asset_url = audio_cell.get("src") if isinstance(audio_cell, dict) else None
            if not isinstance(asset_url, str) or not asset_url.startswith("https://"):
                raise ValueError("audio_url_invalid")
            parsed = urllib.parse.urlparse(asset_url)
            if parsed.hostname != "datasets-server.huggingface.co":
                raise ValueError("audio_host_not_official_datasets_server")
            audio_path = root / f"sample-{offset}.wav"
            data = fetch_limited(asset_url, audio_path)
            wav = parse_wav_header(data)
            parser = run_c_parser(c_runner, row["transcription"])
            records.append(
                {
                    "offset": offset,
                    "path_hash": redacted_hash(str(row["path"])),
                    "audio_sha256": sha256_bytes(data),
                    "audio_bytes": len(data),
                    "wav": wav,
                    "parser": parser,
                }
            )
    audio_hashes = [record["audio_sha256"] for record in records]
    path_hashes = [record["path_hash"] for record in records]
    parser_counts: dict[str, int] = {}
    for record in records:
        for key, value in record["parser"].items():
            if key.startswith("parser_"):
                output_key = key
            elif key in ("automatic_label_mapping", "herus_command_authority"):
                output_key = "parser_" + key
            else:
                continue
            parser_counts[output_key] = parser_counts.get(output_key, 0) + value
    batch_digest = hashlib.sha256(
        "\n".join(
            f"{record['audio_sha256']}:{record['audio_bytes']}:{record['wav']['audio_format']}"
            for record in records
        ).encode("ascii")
    ).hexdigest()
    result: dict[str, Any] = {
        "dataset": EXPECTED_DATASET,
        "config": EXPECTED_CONFIG,
        "split": EXPECTED_SPLIT,
        "api_url_template": "https://datasets-server.huggingface.co/rows?dataset=PolyAI%2Fminds14&config=pt-PT&split=train&offset={offset}&length=1",
        "hub_revision": "40ce77cb32a384e4d50a568e1ec39ac804019d33",
        "offsets_requested": offsets,
        "records": len(records),
        "records_with_audio": len(records),
        "records_with_transcription": len(records),
        "records_with_intent_class": len(records),
        "valid_wav_records": len(records),
        "unique_path_hashes": len(set(path_hashes)),
        "duplicate_path_hashes": len(path_hashes) - len(set(path_hashes)),
        "unique_audio_hashes": len(set(audio_hashes)),
        "duplicate_audio_hashes": len(audio_hashes) - len(set(audio_hashes)),
        "audio_sha256_batch_digest": batch_digest,
        "audio_sha256_values_published": False,
        "path_hash_values_published": False,
        "audio_deleted_after_audit": True,
        "audio_decode_performed": False,
        "external_intent_used_as_parser_input": False,
        "intent_mapped_to_herus": False,
        "herus_convergence_proven": False,
        **parser_counts,
        "verdict": "small_official_audio_text_intent_batch_integrity_checked; semantic_herus_convergence_not_proven",
    }
    return result


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--c-runner", type=Path, required=True)
    parser.add_argument("--offsets", default="0,100,1000,5000")
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()
    offsets = [int(value) for value in args.offsets.split(",") if value]
    result = audit(offsets, args.c_runner)
    args.output.write_text(json.dumps(result, indent=2) + "\n", encoding="utf-8")
    print(json.dumps(result, indent=2))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
