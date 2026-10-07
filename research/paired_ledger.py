"""Fail-closed validation for the Gate 1 paired prediction ledger."""
from __future__ import annotations
import json
from collections import Counter
from pathlib import Path
from typing import Iterable, Mapping

IDENTITY = Path(__file__).parent / "evidence" / "mintrec_s06_identity_manifest_v1.json"
REQUIRED_MODELS = ("herus_full", "multinomial_naive_bayes", "distilbert_multilingual")


def expected_ids(path: Path = IDENTITY) -> tuple[set[str], dict[str, str]]:
    data = json.loads(path.read_text(encoding="utf-8"))
    records = data.get("records", [])
    ids = [row.get("example_id") for row in records]
    if len(ids) != len(set(ids)):
        raise ValueError("identity_manifest_has_duplicate_ids")
    return set(ids), {row["example_id"]: row["label"] for row in records}


def validate_rows(rows: Iterable[Mapping[str, object]], *, seeds: Iterable[int] = (17, 29, 43), models: Iterable[str] = REQUIRED_MODELS, identity_path: Path = IDENTITY) -> dict[str, object]:
    expected, labels = expected_ids(identity_path)
    seed_set = {int(seed) for seed in seeds}
    model_set = set(models)
    rows = list(rows)
    errors: list[str] = []
    seen: Counter[tuple[int, str, str]] = Counter()
    for row in rows:
        seed = row.get("seed")
        ident = row.get("example_id")
        model = row.get("model")
        if not isinstance(seed, int) or seed not in seed_set:
            errors.append("invalid_seed")
        if not isinstance(ident, str) or ident not in expected:
            errors.append("unknown_or_missing_example_id")
        if not isinstance(model, str) or model not in model_set:
            errors.append("unknown_or_missing_model")
        if isinstance(seed, int) and isinstance(ident, str) and isinstance(model, str):
            seen[(seed, ident, model)] += 1
        if isinstance(ident, str) and ident in labels and row.get("y_true") != labels[ident]:
            errors.append("label_mismatch")
        if "accepted" not in row or not isinstance(row.get("accepted"), bool):
            errors.append("accepted_flag_missing_or_invalid")
        if "prediction" not in row:
            errors.append("prediction_missing")
    expected_keys = {(seed, ident, model) for seed in seed_set for ident in expected for model in model_set}
    actual_keys = set(seen)
    if any(count > 1 for count in seen.values()):
        errors.append("duplicate_seed_id_model_rows")
    if actual_keys != expected_keys:
        errors.append("ledger_not_complete_for_all_seeds_ids_models")
    unique_errors = sorted(set(errors))
    return {
        "decision": "PASS_GATE1_LEDGER" if not unique_errors else "BLOCK",
        "rows": len(rows),
        "expected_rows": len(expected_keys),
        "expected_examples": len(expected),
        "expected_seeds": sorted(seed_set),
        "expected_models": sorted(model_set),
        "errors": unique_errors,
        "execution_claim_authorized": False,
    }


if __name__ == "__main__":
    print(json.dumps(validate_rows([]), ensure_ascii=False, indent=2, sort_keys=True))
