"""Audit SLURP identity joins without publishing identifiers.

The auditor treats a filename repeated in metadata or a recording reference
repeated in text as an identity gate. It never joins by numeric IDs, labels,
file order or basename heuristics, and it emits no individual ID or filename.
"""
from __future__ import annotations

import argparse
import collections
import hashlib
import json
from pathlib import Path
from typing import Any


REQUIRED_TEXT_FIELDS = {"slurp_id", "sentence", "intent", "scenario", "recordings"}


def file_sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as handle:
        for block in iter(lambda: handle.read(1024 * 1024), b""):
            digest.update(block)
    return digest.hexdigest()


def identity_digest(value: str) -> str:
    return hashlib.sha256(value.encode("utf-8")).hexdigest()


def load_text_records(paths: dict[str, Path]) -> tuple[list[tuple[str, dict[str, Any]]], dict[str, Any]]:
    records: list[tuple[str, dict[str, Any]]] = []
    split_meta: dict[str, Any] = {}
    for split, path in paths.items():
        rows = 0
        fields: set[str] = set()
        with path.open("r", encoding="utf-8") as handle:
            for line_number, line in enumerate(handle, start=1):
                if not line.strip():
                    raise ValueError(f"blank_line:{split}:{line_number}")
                row = json.loads(line)
                if not isinstance(row, dict):
                    raise ValueError(f"row_not_object:{split}:{line_number}")
                missing = REQUIRED_TEXT_FIELDS - set(row)
                if missing:
                    raise ValueError(f"missing_fields:{split}:{line_number}")
                if not isinstance(row["slurp_id"], int) or isinstance(row["slurp_id"], bool):
                    raise ValueError(f"invalid_slurp_id:{split}:{line_number}")
                if not isinstance(row["sentence"], str) or not row["sentence"].strip():
                    raise ValueError(f"empty_sentence:{split}:{line_number}")
                if not isinstance(row["intent"], str) or not row["intent"]:
                    raise ValueError(f"invalid_intent:{split}:{line_number}")
                if not isinstance(row["scenario"], str) or not row["scenario"]:
                    raise ValueError(f"invalid_scenario:{split}:{line_number}")
                recordings = row["recordings"]
                if not isinstance(recordings, list) or not recordings:
                    raise ValueError(f"invalid_recordings:{split}:{line_number}")
                for recording in recordings:
                    if not isinstance(recording, dict) or not isinstance(recording.get("file"), str):
                        raise ValueError(f"invalid_recording:{split}:{line_number}")
                    if not recording["file"]:
                        raise ValueError(f"empty_recording_file:{split}:{line_number}")
                fields.update(row)
                records.append((split, row))
                rows += 1
        if not rows:
            raise ValueError(f"empty_split:{split}")
        split_meta[split] = {"rows": rows, "sha256": file_sha256(path), "bytes": path.stat().st_size, "fields": sorted(fields)}
    return records, split_meta


def audit_identity(
    records: list[tuple[str, dict[str, Any]]],
    metadata: dict[str, Any],
    split_meta: dict[str, Any] | None = None,
    metadata_sha256: str | None = None,
) -> dict[str, Any]:
    if not records:
        raise ValueError("empty_text_records")
    if not isinstance(metadata, dict) or not metadata:
        raise ValueError("empty_or_invalid_metadata")

    text_refs: collections.Counter[str] = collections.Counter()
    text_id_counts: collections.Counter[int] = collections.Counter()
    for _, row in records:
        text_id_counts[row["slurp_id"]] += 1
        for recording in row["recordings"]:
            text_refs[recording["file"]] += 1

    metadata_occurrences: collections.Counter[str] = collections.Counter()
    metadata_signatures: dict[str, set[str]] = collections.defaultdict(set)
    for entry in metadata.values():
        if not isinstance(entry, dict):
            raise ValueError("metadata_entry_not_object")
        recordings = entry.get("recordings")
        if not isinstance(recordings, dict):
            continue
        for filename, recording_metadata in recordings.items():
            if not isinstance(filename, str) or not filename:
                raise ValueError("metadata_filename_invalid")
            metadata_occurrences[filename] += 1
            metadata_signatures[filename].add(
                json.dumps(recording_metadata, sort_keys=True, ensure_ascii=False)
            )

    duplicate_metadata = {name for name, count in metadata_occurrences.items() if count > 1}
    conflict_metadata = {
        name for name in duplicate_metadata if len(metadata_signatures[name]) > 1
    }
    duplicate_text_refs = {name for name, count in text_refs.items() if count > 1}
    missing_refs = {name for name in text_refs if name not in metadata_occurrences}
    duplicate_slurp_ids = {value for value, count in text_id_counts.items() if count > 1}
    unambiguous = not duplicate_metadata and not duplicate_text_refs and not duplicate_slurp_ids
    if missing_refs:
        identity_status = "CONFLICT"
    elif not unambiguous:
        identity_status = "AMBIGUOUS" if not conflict_metadata else "CONFLICT"
    else:
        identity_status = "VERIFIED"

    return {
        "source": "SLURP official textual annotations plus metadata",
        "text_split_count": len(split_meta or {}),
        "text_rows": len(records),
        "text_unique_recording_references": len(text_refs),
        "metadata_entries": len(metadata),
        "metadata_unique_recording_names": len(metadata_occurrences),
        "metadata_duplicate_filename_count": len(duplicate_metadata),
        "metadata_duplicate_filename_conflict_count": len(conflict_metadata),
        "text_repeated_reference_count": len(duplicate_text_refs),
        "text_repeated_reference_occurrences": sum(text_refs[name] - 1 for name in duplicate_text_refs),
        "missing_reference_count": len(missing_refs),
        "duplicate_numeric_id_count": len(duplicate_slurp_ids),
        "numeric_id_join_used": False,
        "label_join_used": False,
        "order_join_used": False,
        "basename_join_used": False,
        "individual_identifiers_published": False,
        "individual_filenames_published": False,
        "text_split_metadata": split_meta or {},
        "metadata_sha256": metadata_sha256,
        "identity_status": identity_status,
        "identity_unambiguous": unambiguous and not missing_refs,
        "identity_gate_passed": unambiguous and not missing_refs,
        "herus_automatic_mapping_count": 0,
        "herus_convergence_proven": False,
        "verdict": "identity_verified" if unambiguous and not missing_refs else "identity_blocked; no pair promotion",
    }


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--train", type=Path, required=True)
    parser.add_argument("--devel", type=Path, required=True)
    parser.add_argument("--test", type=Path, required=True)
    parser.add_argument("--metadata", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()
    paths = {"train": args.train, "devel": args.devel, "test": args.test}
    records, split_meta = load_text_records(paths)
    metadata = json.loads(args.metadata.read_text(encoding="utf-8"))
    result = audit_identity(records, metadata, split_meta, file_sha256(args.metadata))
    args.output.write_text(json.dumps(result, indent=2, ensure_ascii=False) + "\n", encoding="utf-8")
    print(json.dumps(result, indent=2, ensure_ascii=False))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
