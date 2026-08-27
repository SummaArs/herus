"""Analyze the real MIntRec metadata without downloading heavy media.

This is a data-audit tool, not an intent mapper. MIntRec labels are kept as
source labels; no automatic mapping to HERUS actions is allowed.
"""
from __future__ import annotations

import csv
import hashlib
import json
from collections import Counter, defaultdict
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
DATA = ROOT / "research" / "source_data" / "MIntRec" / "metadata"
OUT = ROOT / "research" / "evidence" / "wide_cycle_04" / "mintrec_metadata_audit.json"
EXPECTED_COLUMNS = ["season", "episode", "clip", "text", "label"]
HERUS_EVENTS = {"ARRIVE", "HELP", "CANCEL"}


def sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as handle:
        for block in iter(lambda: handle.read(1024 * 1024), b""):
            digest.update(block)
    return digest.hexdigest()


def read_split(path: Path) -> tuple[list[dict[str, str]], list[str]]:
    with path.open("r", encoding="utf-8", newline="") as handle:
        reader = csv.DictReader(handle, delimiter="\t")
        columns = reader.fieldnames or []
        rows = list(reader)
    return rows, columns


def main() -> None:
    split_data: dict[str, list[dict[str, str]]] = {}
    split_meta: dict[str, dict[str, object]] = {}
    all_ids: dict[str, list[str]] = defaultdict(list)
    labels = Counter()
    empty_text: list[dict[str, str]] = []

    required_paths = [DATA / "train.tsv", DATA / "test.tsv"]
    missing = [str(path) for path in required_paths if not path.exists()]
    if missing:
        raise SystemExit("missing required MIntRec metadata: " + ", ".join(missing))

    for split in ("train", "dev", "test"):
        path = DATA / f"{split}.tsv"
        if not path.exists():
            continue
        rows, columns = read_split(path)
        split_data[split] = rows
        split_meta[split] = {
            "path": str(path.relative_to(ROOT)),
            "bytes": path.stat().st_size,
            "sha256": sha256(path),
            "rows": len(rows),
            "columns": columns,
            "columns_match": columns == EXPECTED_COLUMNS,
        }
        for row in rows:
            sample_id = f"{row.get('season')}:{row.get('episode')}:{row.get('clip')}"
            all_ids[sample_id].append(split)
            labels[row.get("label", "")] += 1
            if not row.get("text", "").strip():
                empty_text.append({"split": split, "id": sample_id})

    duplicate_ids = {sample_id: splits for sample_id, splits in all_ids.items() if len(splits) > 1}
    source_labels = sorted(labels)
    unsupported_labels = sorted(set(source_labels) - HERUS_EVENTS)
    result = {
        "source": {
            "name": "MIntRec",
            "repository": "https://github.com/thuiar/MIntRec",
            "storage": "https://drive.google.com/drive/folders/18iLqmUYDDOwIiiRbgwLpzw76BD62PK0p",
            "policy": "metadata-only audit; no automatic label mapping",
        },
        "splits": split_meta,
        "total_rows": sum(len(rows) for rows in split_data.values()),
        "unique_segment_ids": len(all_ids),
        "duplicate_segment_ids_across_splits": duplicate_ids,
        "label_counts": dict(sorted(labels.items())),
        "source_labels": source_labels,
        "unsupported_for_herus_without_annotation": unsupported_labels,
        "empty_text_rows": empty_text,
        "herus_event_vocabulary": sorted(HERUS_EVENTS),
        "automatic_mapping_count": 0,
        "verdict": {
            "metadata_integrity": all(meta["columns_match"] for meta in split_meta.values()) and not empty_text,
            "split_leakage": bool(duplicate_ids),
            "herus_convergence_proven": False,
            "reason": "TSVs prove real text/label metadata and split integrity only; aligned audio/video files were not downloaded or mapped to the finite HERUS vocabulary.",
        },
    }
    OUT.write_text(json.dumps(result, indent=2, ensure_ascii=False) + "\n", encoding="utf-8")
    print(json.dumps(result, indent=2, ensure_ascii=False))


if __name__ == "__main__":
    main()
