from __future__ import annotations
from dataclasses import dataclass
from math import sqrt


def wilson_upper(errors: int, total: int, z: float = 1.96) -> float:
    if total <= 0:
        return 1.0
    p = errors / total
    d = 1 + z*z/total
    c = (p + z*z/(2*total) + z*sqrt(p*(1-p)/total + z*z/(4*total*total))) / d
    return min(1.0, c)


@dataclass(frozen=True)
class RiskCoverageDecision:
    agreement_threshold: int
    calibration_coverage: float
    calibration_risk: float
    risk_upper: float
    target_risk: float


class CalibratedRiskCoverage:
    """Choose the widest agreement region whose Wilson risk upper bound is safe."""
    def __init__(self, target_risk: float = 0.20):
        if not 0 < target_risk < 1:
            raise ValueError('target_risk_must_be_between_zero_and_one')
        self.target_risk = target_risk
        self.decision: RiskCoverageDecision | None = None

    def fit(self, agreements: list[int], correct: list[bool]) -> RiskCoverageDecision:
        if not agreements or len(agreements) != len(correct):
            raise ValueError('calibration_pairs_required')
        candidates = []
        for threshold in (1, 2, 3):
            selected = [i for i, score in enumerate(agreements) if score >= threshold]
            if not selected:
                continue
            errors = sum(not correct[i] for i in selected)
            upper = wilson_upper(errors, len(selected))
            if upper <= self.target_risk:
                candidates.append((len(selected), threshold, errors, upper))
        if not candidates:
            raise ValueError('no_safe_coverage_region')
        _, threshold, errors, upper = max(candidates, key=lambda x: (x[0], -x[1]))
        n = sum(score >= threshold for score in agreements)
        self.decision = RiskCoverageDecision(threshold, n/len(agreements), errors/n, upper, self.target_risk)
        return self.decision

    def accept(self, predictions: list[str]) -> bool:
        if self.decision is None:
            raise RuntimeError('policy_not_fitted')
        if len(predictions) != 3:
            raise ValueError('three_predictions_required')
        return sum(pred == predictions[0] for pred in predictions) >= self.decision.agreement_threshold
