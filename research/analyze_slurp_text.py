"""Audit the official textual SLURP annotations and metadata join.

The repository's textual data is a real CC BY 4.0 artifact. This script emits
only aggregate counts and hashes. It does not load audio, expose sentences or
IDs, execute repository code, or map external intents to HERUS events.
"""
from __future__ import annotations

import argparse
import collections
import hashlib
import json
from pathlib import Path
from typing import Any


EXPECTED_FIELDS = {
    "slurp_id",
    "sentence",
    "sentence_annotation",
    "intent",
    "action",
    "tokens",
    "scenario",
    "recordings",
    "entities",
}


def sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as handle:
        for block in iter(lambda: handle.read(1024 * 1024), b""):
            digest.update(block)
    return digest.hexdigest()


def load_jsonl(path: Path) -> tuple[list[dict[str, Any]], dict[str, Any]]:
    rows: list[dict[str, Any]] = []
    fields_seen: set[str] = set()
    with path.open("r", encoding="utf-8") as handle:
        for line_number, line in enumerate(handle, start=1):
            if not line.strip():
                raise ValueError(f"blank_line:{path}:{line_number}")
            row = json.loads(line)
            if not isinstance(row, dict):
                raise ValueError(f"row_not_object:{path}:{line_number}")
            fields_seen.update(row)
            missing = EXPECTED_FIELDS - set(row)
            if missing:
                raise ValueError(f"missing_fields:{path}:{line_number}:{sorted(missing)}")
            if not isinstance(row["slurp_id"], int) or isinstance(row["slurp_id"], bool):
                raise ValueError(f"invalid_slurp_id:{path}:{line_number}")
            if not isinstance(row["sentence"], str) or not row["sentence"].strip():
                raise ValueError(f"empty_sentence:{path}:{line_number}")
            if not isinstance(row["intent"], str) or not row["intent"]:
                raise ValueError(f"invalid_intent:{path}:{line_number}")
            if not isinstance(row["scenario"], str) or not row["scenario"]:
                raise ValueError(f"invalid_scenario:{path}:{line_number}")
            if not isinstance(row["recordings"], list) or not row["recordings"]:
                raise ValueError(f"invalid_recordings:{path}:{line_number}")
            for recording in row["recordings"]:
                if not isinstance(recording, dict) or not isinstance(recording.get("file"), str) or not recording["file"]:
                    raise ValueError(f"invalid_recording:{path}:{line_number}")
            rows.append(row)
    if not rows:
        raise ValueError(f"empty_jsonl:{path}")
    return rows, {"columns": sorted(fields_seen), "rows": len(rows), "sha256": sha256(path), "bytes": path.stat().st_size}


def load_metadata(path: Path) -> dict[str, dict[str, Any]]:
    payload = json.loads(path.read_text(encoding="utf-8"))
    if not isinstance(payload, dict):
        raise ValueError("metadata_root_not_object")
    return payload


def audit(split_paths: dict[str, Path], metadata_path: Path) -> dict[str, object]:
    split_meta: dict[str, dict[str, Any]] = {}
    all_rows: list[tuple[str, dict[str, Any]]] = []
    by_id: dict[int, list[str]] = collections.defaultdict(list)
    intents: collections.Counter[str] = collections.Counter()
    scenarios: collections.Counter[str] = collections.Counter()
    actions: collections.Counter[str] = collections.Counter()
    recording_refs: set[str] = set()
    recording_ref_counts: collections.Counter[str] = collections.Counter()
    for split, path in split_paths.items():
        rows, meta = load_jsonl(path)
        split_meta[split] = meta
        for row in rows:
            slurp_id = row["slurp_id"]
            all_rows.append((split, row))
            by_id[slurp_id].append(split)
            intents[row["intent"]] += 1
            scenarios[row["scenario"]] += 1
            actions[row["action"]] += 1
            row_recording_names = [recording["file"] for recording in row["recordings"]]
            recording_refs.update(row_recording_names)
            recording_ref_counts.update(row_recording_names)

    metadata = load_metadata(metadata_path)
    metadata_recording_owners: dict[str, int] = {}
    metadata_recording_occurrences: collections.Counter[str] = collections.Counter()
    metadata_recording_signatures: dict[str, set[str]] = collections.defaultdict(set)
    metadata_recording_duplicates = 0
    metadata_records_with_recordings = 0
    for entry in metadata.values():
        if not isinstance(entry, dict):
            continue
        metadata_recordings = entry.get("recordings")
        if not isinstance(metadata_recordings, dict):
            continue
        metadata_records_with_recordings += 1
        for filename, recording_metadata in metadata_recordings.items():
            metadata_recording_occurrences[filename] += 1
            metadata_recording_signatures[filename].add(
                json.dumps(recording_metadata, sort_keys=True, ensure_ascii=False)
            )
            if filename in metadata_recording_owners:
                metadata_recording_duplicates += 1
            else:
                metadata_recording_owners[filename] = metadata_records_with_recordings

    missing_recordings = 0
    joined_rows = 0
    joined_recordings = 0
    for _, row in all_rows:
        row_recordings = [recording["file"] for recording in row["recordings"]]
        row_missing = sum(1 for filename in row_recordings if filename not in metadata_recording_owners)
        missing_recordings += row_missing
        joined_recordings += len(row_recordings) - row_missing
        if row_missing == 0:
            joined_rows += 1

    duplicates = {str(slurp_id): splits for slurp_id, splits in by_id.items() if len(splits) > 1}
    result = {
        "source": "SLURP official textual repository plus bounded metadata member",
        "repository": "https://github.com/pswietojanski/slurp",
        "repository_commit": "8eb16545762be97ace75334109d73824217311f1",
        "text_license": "CC BY 4.0",
        "audio_license": "CC BY-NC 4.0",
        "split_files": split_meta,
        "metadata_file": {
            "path": str(metadata_path),
            "bytes": metadata_path.stat().st_size,
            "sha256": sha256(metadata_path),
            "top_level_entries": len(metadata),
        },
        "text_rows": len(all_rows),
        "unique_slurp_ids": len(by_id),
        "duplicate_slurp_ids_across_splits": len(duplicates),
        "unique_recording_references": len(recording_refs),
        "metadata_top_level_entries": len(metadata),
        "metadata_records_with_recordings": metadata_records_with_recordings,
        "metadata_unique_recording_names": len(metadata_recording_owners),
        "metadata_duplicate_recording_names": sum(1 for count in metadata_recording_occurrences.values() if count > 1),
        "metadata_duplicate_recording_occurrences_redacted": metadata_recording_duplicates,
        "metadata_duplicate_recording_conflicts": sum(
            1 for name, signatures in metadata_recording_signatures.items()
            if metadata_recording_occurrences[name] > 1 and len(signatures) > 1
        ),
        "text_duplicate_recording_references": sum(
            count - 1 for count in recording_ref_counts.values() if count > 1
        ),
        "metadata_joined_rows_by_recording_filename": joined_rows,
        "metadata_joined_recording_references": joined_recordings,
        "missing_recording_references": missing_recordings,
        "intent_count": len(intents),
        "scenario_count": len(scenarios),
        "action_count": len(actions),
        "intent_label_counts": dict(sorted(intents.items())),
        "herus_automatic_mapping_count": 0,
        "sentences_published_in_result": False,
        "individual_ids_published_in_result": False,
        "audio_loaded": False,
        "verdict": {
            "text_schema_integrity": not missing_recordings,
            "text_recording_metadata_alignment_verified": (
                not missing_recordings
                and not metadata_recording_duplicates
                and not any(count > 1 for count in recording_ref_counts.values())
            ),
            "audio_transcript_content_verified": False,
            "herus_convergence_proven": False,
            "reason": "Text annotations join by recordings[].file, but duplicate metadata or text references keep alignment blocked; audio bytes were not loaded and external intents are not HERUS events."
        },
    }
    return result


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--train", type=Path, required=True)
    parser.add_argument("--devel", type=Path, required=True)
    parser.add_argument("--test", type=Path, required=True)
    parser.add_argument("--metadata", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()
    result = audit(
        {"train": args.train, "devel": args.devel, "test": args.test},
        args.metadata,
    )
    args.output.write_text(json.dumps(result, indent=2, ensure_ascii=False) + "\n", encoding="utf-8")
    print(json.dumps(result, indent=2, ensure_ascii=False))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
