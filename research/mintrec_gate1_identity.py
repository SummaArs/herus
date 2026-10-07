"""Materialize a verifiable identity manifest for the MIntRec S06 holdout.

The API loader does not expose a stable example ID in the current benchmark
adapter. We therefore derive a canonical content identity and fail closed on
collisions, duplicate identities or missing fields. This is an identity
manifest, not a claim that the remote dataset is immutable.
"""
from __future__ import annotations
import hashlib
import json
from pathlib import Path
from typing import Iterable, Mapping

from real_data_baseline_benchmark import DATASET, fetch_split

OUT = Path(__file__).parent / "evidence" / "mintrec_s06_identity_manifest_v1.json"


def canonical_row(row: Mapping[str, str]) -> dict[str, str]:
    required = ("season", "episode", "clip", "label", "text")
    if any(not isinstance(row.get(key), str) or not row[key] for key in required):
        raise ValueError("row_missing_identity_field")
    return {key: row[key] for key in required}


def example_id(row: Mapping[str, str]) -> str:
    payload = json.dumps(canonical_row(row), ensure_ascii=False, sort_keys=True, separators=(",", ":")).encode("utf-8")
    return hashlib.sha256(payload).hexdigest()


def digest_rows(rows: Iterable[Mapping[str, str]]) -> str:
    payload = "\n".join(json.dumps(canonical_row(row), ensure_ascii=False, sort_keys=True, separators=(",", ":")) for row in rows)
    return hashlib.sha256(payload.encode("utf-8")).hexdigest()


def build(rows: list[Mapping[str, str]]) -> dict[str, object]:
    holdout = [row for row in rows if row.get("season") == "S06"]
    if not holdout:
        raise ValueError("s06_holdout_empty")
    records = []
    ids: set[str] = set()
    for row in holdout:
        clean = canonical_row(row)
        ident = example_id(clean)
        if ident in ids:
            raise ValueError("duplicate_example_id")
        ids.add(ident)
        records.append({"example_id": ident, **clean})
    records.sort(key=lambda item: item["example_id"])
    labels = [item["label"] for item in records]
    return {
        "schema_version": "herus.mintrec.identity.v1",
        "dataset": DATASET,
        "source": "https://datasets-server.huggingface.co/rows",
        "source_revision": "not_exposed_by_rows_api",
        "source_rows": len(rows),
        "holdout_season": "S06",
        "holdout_rows": len(records),
        "identity_key": "example_id",
        "identity_formula": "sha256(canonical JSON of season, episode, clip, label, text)",
        "source_rows_sha256": digest_rows(rows),
        "holdout_manifest_sha256": hashlib.sha256(json.dumps(records, ensure_ascii=False, sort_keys=True, separators=(",", ":")).encode("utf-8")).hexdigest(),
        "labels": sorted(set(labels)),
        "records": records,
        "limits": [
            "content-derived identity is not a provider-issued immutable ID",
            "rows API revision is not exposed in this response",
            "text/label metadata only; no audio/video claim",
        ],
    }


def run(total: int = 2224) -> dict[str, object]:
    result = build(fetch_split("train", total))
    OUT.write_text(json.dumps(result, ensure_ascii=False, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    return result


if __name__ == "__main__":
    result = run()
    print(json.dumps({key: result[key] for key in result if key != "records"}, ensure_ascii=False, indent=2, sort_keys=True))
