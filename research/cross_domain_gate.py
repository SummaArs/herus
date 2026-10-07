"""Fail-closed audit for external cross-domain evidence.

A dataset may be real and still be insufficient to prove transfer of the
HERUS algorithm. This gate checks the evidence contract before allowing a
cross-domain claim into the scorecard.
"""
from __future__ import annotations
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parent
EVIDENCE = ROOT / "evidence" / "minds14_real_benchmark_v1.json"


def audit(evidence: dict[str, object]) -> dict[str, object]:
    dataset = evidence.get("dataset", {})
    methods = evidence.get("methods", {})
    limits = set(evidence.get("limits", []))
    blockers: list[str] = []
    if "no speaker-independent split verified" in limits:
        blockers.append("speaker_independent_split_missing")
    if "MInDS labels are not HERUS events" in limits:
        blockers.append("label_contract_missing")
    if "consensus_selective" not in methods:
        blockers.append("symbiotic_selective_method_missing")
    if not dataset.get("id") or not dataset.get("holdout"):
        blockers.append("dataset_holdout_metadata_missing")
    return {
        "dataset": dataset.get("id"),
        "eligible_for_cross_domain_claim": not blockers,
        "blockers": blockers,
        "decision": "BLOCK" if blockers else "ELIGIBLE_FOR_REVIEW",
        "rule": "real dataset is necessary but not sufficient; transfer requires independent holdout, label alignment and an evaluated HERUS method",
    }


def run(path: Path = EVIDENCE) -> dict[str, object]:
    return audit(json.loads(path.read_text(encoding="utf-8")))


if __name__ == "__main__":
    print(json.dumps(run(), indent=2, ensure_ascii=False, sort_keys=True))
