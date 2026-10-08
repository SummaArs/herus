"""Leakage-resistant fit/holdout harness for HERUS objective weights."""
from __future__ import annotations
from dataclasses import dataclass
from typing import Sequence
from symbiotic_learning import Feedback, OptimizationResult, SymbioticLearner, UtilityWeights

@dataclass(frozen=True)
class HoldoutResult:
    status: str
    fit_count: int
    holdout_count: int
    fit_objective: float
    holdout_objective: float
    generalization_gap: float
    weights: UtilityWeights
    reason: str = ""


def _ids(rows: Sequence[Feedback]) -> set[str]:
    ids = [row.example_id for row in rows]
    if not ids or any(not value for value in ids) or len(ids) != len(set(ids)):
        raise ValueError("fit_or_holdout_ids_invalid")
    return set(ids)


def evaluate_fit_holdout(learner: SymbioticLearner, fit: Sequence[Feedback], holdout: Sequence[Feedback], *, grid: Sequence[float] = (0.0, 0.5, 1.0, 2.0, 4.0)) -> HoldoutResult:
    """Fit weights on fit rows and evaluate once on disjoint holdout rows."""
    fit_ids = _ids(fit)
    holdout_ids = _ids(holdout)
    if fit_ids.intersection(holdout_ids):
        return HoldoutResult("BLOCK", len(fit), len(holdout), 0.0, 0.0, 0.0, UtilityWeights(), "fit_holdout_overlap")
    optimized: OptimizationResult = learner.optimize_weights(fit, grid=grid)
    if optimized.status != "OPTIMIZED":
        return HoldoutResult("BLOCK", len(fit), len(holdout), 0.0, 0.0, 0.0, optimized.weights, "fit_optimization_unavailable")
    fit_score = sum(learner.objective(utility=row.utility, risk=row.risk, cost=row.cost, authority_violation=row.authority_violation, evidence_deficit=row.evidence_deficit, weights=optimized.weights) for row in fit) / len(fit)
    holdout_score = sum(learner.objective(utility=row.utility, risk=row.risk, cost=row.cost, authority_violation=row.authority_violation, evidence_deficit=row.evidence_deficit, weights=optimized.weights) for row in holdout) / len(holdout)
    return HoldoutResult("EVALUATED", len(fit), len(holdout), fit_score, holdout_score, fit_score - holdout_score, optimized.weights)
