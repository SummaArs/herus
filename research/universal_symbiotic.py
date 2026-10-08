"""Universal Symbiotic Learner: one decision algorithm over many ML paradigms."""
from __future__ import annotations
from dataclasses import dataclass
from hashlib import sha256
from typing import Sequence


def _id(value: object) -> str:
    return sha256(repr(value).encode()).hexdigest()


@dataclass(frozen=True)
class ParadigmCandidate:
    example_id: str
    paradigm: str
    label: str
    score: float
    correct: bool | None
    risk: float = 0.0
    cost: float = 0.0
    evidence: int = 0
    host_id: str = ""


@dataclass(frozen=True)
class UniversalContract:
    host_id: str
    allowed_paradigms: tuple[str, ...] = ("supervised", "unsupervised", "self_supervised", "reinforcement", "symbiotic")
    max_risk: float = 0.0
    max_cost: float = 1.0
    min_evidence: int = 1
    min_precision: float = 0.95


@dataclass(frozen=True)
class UniversalFit:
    status: str
    thresholds: tuple[tuple[str, float], ...]
    calibration_examples: int
    reason: str


@dataclass(frozen=True)
class UniversalDecision:
    example_id: str
    paradigm: str | None
    label: str | None
    status: str
    reason: str
    score: float
    evidence_id: str


class UniversalSymbioticLearner:
    """A single acceptance/routing algorithm, not a claim of five neural engines."""

    def __init__(self) -> None:
        self.thresholds: dict[str, float] = {}
        self.contract: UniversalContract | None = None

    def fit(self, calibration: Sequence[ParadigmCandidate], contract: UniversalContract) -> UniversalFit:
        self.contract = contract
        if not contract.host_id or contract.max_risk < 0 or contract.max_cost < 0 or not 0 <= contract.min_precision <= 1:
            return UniversalFit("BLOCKED", (), len(calibration), "invalid_contract")
        allowed = [x for x in calibration if x.paradigm in contract.allowed_paradigms and x.host_id in ("", contract.host_id) and x.correct is not None]
        if not allowed:
            return UniversalFit("BLOCKED", (), len(calibration), "no_calibration_candidates")
        thresholds: dict[str, float] = {}
        for paradigm in contract.allowed_paradigms:
            rows=[x for x in allowed if x.paradigm == paradigm]
            safe=[x.score for x in rows if x.score >= 0 and x.correct and x.risk <= contract.max_risk and x.cost <= contract.max_cost and x.evidence >= contract.min_evidence]
            if safe:
                thresholds[paradigm]=min(safe)
        self.thresholds=thresholds
        return UniversalFit("FITTED" if thresholds else "BLOCKED", tuple(sorted(thresholds.items())), len(calibration), "per_paradigm_safe_thresholds" if thresholds else "no_safe_paradigm")

    def decide(self, candidates: Sequence[ParadigmCandidate], *, example_id: str) -> UniversalDecision:
        if self.contract is None or not self.thresholds:
            return UniversalDecision(example_id, None, None, "ABSTAIN", "policy_not_fitted", 0.0, _id((example_id, "not-fitted")))
        valid=[x for x in candidates if x.example_id == example_id and x.paradigm in self.thresholds and x.host_id in ("", self.contract.host_id) and x.risk <= self.contract.max_risk and x.cost <= self.contract.max_cost and x.evidence >= self.contract.min_evidence and x.score >= self.thresholds[x.paradigm]]
        if not valid:
            return UniversalDecision(example_id, None, None, "ABSTAIN", "no_candidate_meets_contract", 0.0, _id((example_id, "abstain")))
        valid=sorted(valid, key=lambda x:(x.score, x.evidence, -x.risk, -x.cost), reverse=True)
        top=valid[0]
        ties=[x for x in valid if x.score == top.score and x.label != top.label]
        if ties:
            return UniversalDecision(example_id, None, None, "ABSTAIN", "cross_paradigm_conflict", top.score, _id(tuple(valid)))
        return UniversalDecision(example_id, top.paradigm, top.label, "ACCEPT", "best_contract_satisfying_candidate", top.score, _id((top, self.contract)))

    def inspect(self) -> dict[str, object]:
        return {"thresholds": dict(self.thresholds), "contract": self.contract, "authority": "none", "paradigm_router": "bounded"}

    def promote_policy(self, validation_records: Sequence[dict[str, object]], *, alternative: str = "score_calibrated", default: str = "universal_default", margin: float = 0.01, min_hosts: int = 3) -> dict[str, object]:
        """Promote a policy only after the independent multi-host stability gate passes."""
        from policy_stability_gate import evaluate
        decision = evaluate(validation_records, alternative=alternative, default=default, margin=margin, min_hosts=min_hosts)
        return {"status": decision.status, "policy": decision.policy, "reason": decision.reason, "hosts": decision.hosts, "passing_hosts": decision.passing_hosts, "minimum_margin": decision.minimum_margin, "deltas": dict(decision.deltas)}
