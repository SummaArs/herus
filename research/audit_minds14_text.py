"""Audit all MInDS-14 pt-PT train rows through the real C parser.

The Datasets Server rows endpoint is used for text/transcription metadata only.
Audio URLs are checked for official host and presence but no audio bytes are
fetched. Sentences, paths, signed URLs and external label names are never
written to the output.
"""
from __future__ import annotations

import argparse
import collections
import json
import urllib.parse
from pathlib import Path
from typing import Any

from fetch_minds14_sample import (
    EXPECTED_CONFIG,
    EXPECTED_DATASET,
    EXPECTED_SPLIT,
    fetch_json,
    run_c_parser,
)


ROWS_BASE = "https://datasets-server.huggingface.co/rows"
REVISION = "40ce77cb32a384e4d50a568e1ec39ac804019d33"
PAGE_SIZE = 100
MAX_ROWS = 2000


def rows_url(offset: int) -> str:
    query = urllib.parse.urlencode(
        {
            "dataset": EXPECTED_DATASET,
            "config": EXPECTED_CONFIG,
            "split": EXPECTED_SPLIT,
            "offset": offset,
            "length": PAGE_SIZE,
        }
    )
    return ROWS_BASE + "?" + query


def audit(c_runner: Path) -> dict[str, Any]:
    counts: collections.Counter[str] = collections.Counter()
    intent_ids: set[int] = set()
    lang_ids: set[int] = set()
    rows_seen = 0
    expected_total: int | None = None
    page_count = 0
    row_indices: list[int] = []
    for offset in range(0, MAX_ROWS, PAGE_SIZE):
        payload = fetch_json(rows_url(offset))
        features = payload.get("features")
        if not isinstance(features, list):
            raise ValueError("features_missing")
        feature_names = {feature.get("name") for feature in features if isinstance(feature, dict)}
        required = {"path", "audio", "transcription", "intent_class", "lang_id"}
        if not required.issubset(feature_names):
            raise ValueError("feature_schema_mismatch")
        total = payload.get("num_rows_total")
        if not isinstance(total, int) or total <= 0 or total > MAX_ROWS:
            raise ValueError("invalid_total_rows")
        if expected_total is None:
            expected_total = total
        elif total != expected_total:
            raise ValueError("total_rows_changed")
        rows = payload.get("rows")
        if not isinstance(rows, list) or not rows:
            raise ValueError("rows_missing_or_empty")
        page_count += 1
        for item in rows:
            if not isinstance(item, dict) or item.get("row_idx") != rows_seen:
                raise ValueError("row_index_not_sequential")
            row = item.get("row")
            if not isinstance(row, dict):
                raise ValueError("row_not_object")
            if not isinstance(row.get("path"), str) or not row["path"]:
                raise ValueError("path_missing")
            sentence = row.get("transcription")
            if not isinstance(sentence, str) or not sentence.strip():
                raise ValueError("transcription_missing")
            if not isinstance(row.get("intent_class"), int) or not isinstance(row.get("lang_id"), int):
                raise ValueError("class_or_language_invalid")
            audio = row.get("audio")
            if not isinstance(audio, list) or len(audio) != 1:
                raise ValueError("audio_cell_missing_or_ambiguous")
            asset = audio[0]
            if not isinstance(asset, dict) or asset.get("type") != "audio/wav":
                raise ValueError("audio_type_invalid")
            source_url = asset.get("src")
            if not isinstance(source_url, str) or urllib.parse.urlparse(source_url).hostname != "datasets-server.huggingface.co":
                raise ValueError("audio_host_not_official")
            parser = run_c_parser(c_runner, sentence)
            for key, value in parser.items():
                if key.startswith("parser_") or key in ("automatic_label_mapping", "herus_command_authority"):
                    counts[key] += value
            intent_ids.add(row["intent_class"])
            lang_ids.add(row["lang_id"])
            row_indices.append(item["row_idx"])
            rows_seen += 1
        if rows_seen >= expected_total:
            break
    if expected_total is None or rows_seen != expected_total:
        raise ValueError("incomplete_dataset")
    if row_indices != list(range(expected_total)):
        raise ValueError("row_indices_not_complete")
    return {
        "dataset": EXPECTED_DATASET,
        "config": EXPECTED_CONFIG,
        "split": EXPECTED_SPLIT,
        "hub_revision": REVISION,
        "rows_total": expected_total,
        "rows_seen": rows_seen,
        "pages": page_count,
        "audio_cells_present_and_official_host": rows_seen,
        "audio_bytes_fetched": 0,
        "audio_decode_performed": False,
        "transcriptions_present": rows_seen,
        "distinct_external_intent_ids": len(intent_ids),
        "distinct_language_ids": len(lang_ids),
        "individual_paths_published": False,
        "individual_sentences_published": False,
        "external_labels_used_as_parser_input": False,
        "automatic_label_mapping": 0,
        "herus_command_authority": 0,
        "herus_convergence_proven": False,
        **dict(counts),
        "verdict": "real_reference_transcription_corpus_measured; audio_presence_only; semantic_herus_convergence_not_proven",
    }


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--c-runner", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()
    result = audit(args.c_runner)
    args.output.write_text(json.dumps(result, indent=2) + "\n", encoding="utf-8")
    print(json.dumps(result, indent=2))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
