"""Audit whether benchmark claims have the evidence they require.

This gate does not score models. It prevents a partial ledger or missing cost
measurements from being presented as a complete paired or efficiency result.
"""
from __future__ import annotations
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parent
E = ROOT / "evidence"


def audit(transformer_path: Path = E / "transformer_multilingual_mintrec_v1.json",
          paired_path: Path = E / "paired_transformer_analysis_v1.json",
          matrix_path: Path = E / "final_ml_competition_v1.json") -> dict[str, object]:
    transformer = json.loads(transformer_path.read_text(encoding="utf-8"))
    paired = json.loads(paired_path.read_text(encoding="utf-8"))
    matrix = json.loads(matrix_path.read_text(encoding="utf-8"))
    blockers: list[str] = []
    holdout = int(transformer.get("dataset", {}).get("holdout_rows", 0))
    ledger = transformer.get("prediction_ledger", [])
    if len(ledger) != holdout or [row.get("index") for row in ledger] != list(range(holdout)):
        blockers.append("transformer_ledger_incomplete")
    models = paired.get("models", {})
    if not {"naive_bayes", "herus_context_memory", "distilbert_multilingual"}.issubset(models):
        blockers.append("paired_model_metrics_incomplete")
    # The published paired artifact contains aggregate comparisons only.
    if "paired_prediction_ledger" not in paired:
        blockers.append("paired_prediction_ledger_not_archived")
    mintrec_rows = [row for row in matrix.get("pareto", []) if row.get("dataset") == "MIntRec S06"]
    if any(row.get("infer_ms") is None or row.get("train_ms") is None for row in mintrec_rows):
        blockers.append("mintrec_costs_missing")
    return {
        "dataset": "THU-IAR/MIntRec",
        "decision": "BLOCK" if blockers else "READY_FOR_CLAIM_REVIEW",
        "eligible_for_complete_paired_claim": not any(x.startswith("paired_") or x.startswith("transformer_") for x in blockers),
        "eligible_for_efficiency_claim": "mintrec_costs_missing" not in blockers,
        "blockers": blockers,
        "requirements": [
            "archive one aligned row per holdout example for every compared method",
            "measure train and inference cost under the same environment and budget",
            "retain seeds, versions and resource definitions",
        ],
    }


if __name__ == "__main__":
    print(json.dumps(audit(), indent=2, ensure_ascii=False, sort_keys=True))
