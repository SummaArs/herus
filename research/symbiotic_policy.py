"""Symbiotic decision learning: a model-agnostic, host-constrained acceptance policy.

This is not a replacement classifier. It learns a bounded policy over candidate
predictions produced by any host model, using calibration feedback and explicit
risk/cost/evidence constraints. The research claim remains a hypothesis.
"""
from __future__ import annotations
from dataclasses import dataclass
from hashlib import sha256
from typing import Sequence


def _id(value: object) -> str:
    return sha256(repr(value).encode()).hexdigest()


@dataclass(frozen=True)
class HostBudget:
    host_id: str
    max_cost: float = 1.0
    max_risk: float = 0.0
    min_evidence: int = 1


@dataclass(frozen=True)
class Candidate:
    example_id: str
    label: str
    score: float
    correct: bool | None
    cost: float = 0.0
    risk: float = 0.0
    evidence_count: int = 0
    host_id: str = ""


@dataclass(frozen=True)
class PolicyFit:
    threshold: float | None
    calibration_examples: int
    accepted: int
    precision: float
    coverage: float
    status: str
    reason: str


@dataclass(frozen=True)
class Decision:
    example_id: str
    label: str | None
    status: str
    reason: str
    score: float
    threshold: float | None
    evidence_id: str


@dataclass(frozen=True)
class VerifiedFeedback:
    example_id: str
    accepted: bool
    correct: bool
    host_id: str
    provenance: str


class SymbioticDecisionPolicy:
    """Learn when to trust a host candidate, never what the host should execute."""

    def __init__(self, *, min_precision: float = 0.95, max_updates: int = 128) -> None:
        if not 0.0 <= min_precision <= 1.0:
            raise ValueError("min_precision_out_of_range")
        self.min_precision = min_precision
        self.max_updates = max_updates
        self.threshold: float | None = None
        self._feedback: list[VerifiedFeedback] = []

    def fit(self, calibration: Sequence[Candidate], host: HostBudget) -> PolicyFit:
        """Choose a threshold on calibration only; no holdout labels are accepted here."""
        if not calibration:
            return PolicyFit(None, 0, 0, 0.0, 0.0, "BLOCKED", "no_calibration_data")
        if not host.host_id or host.max_cost < 0 or host.max_risk < 0 or host.min_evidence < 0:
            return PolicyFit(None, len(calibration), 0, 0.0, 0.0, "BLOCKED", "invalid_host_budget")
        usable = [x for x in calibration if x.correct is not None]
        if not usable:
            return PolicyFit(None, len(calibration), 0, 0.0, 0.0, "BLOCKED", "calibration_labels_missing")
        thresholds = sorted({max(0.0, min(1.0, x.score)) for x in usable}, reverse=True)
        best: tuple[float, float, float] | None = None
        for threshold in thresholds:
            chosen = [x for x in usable if x.score >= threshold and x.cost <= host.max_cost and x.risk <= host.max_risk and x.evidence_count >= host.min_evidence]
            if not chosen:
                continue
            precision = sum(bool(x.correct) for x in chosen) / len(chosen)
            coverage = len(chosen) / len(usable)
            if precision < self.min_precision:
                continue
            # Maximize coverage first, then precision, then prefer lower threshold.
            key = (coverage, precision, -threshold)
            if best is None or key > best:
                best = key
        if best is None:
            return PolicyFit(None, len(calibration), 0, 0.0, 0.0, "BLOCKED", "no_safe_operating_point")
        coverage, precision, neg_threshold = best
        self.threshold = -neg_threshold
        chosen_count = round(coverage * len(usable))
        return PolicyFit(self.threshold, len(calibration), chosen_count, precision, coverage, "FITTED", "bounded_calibration")

    def decide(self, candidate: Candidate, host: HostBudget) -> Decision:
        evidence_id = _id((candidate.example_id, candidate.label, candidate.score, host.host_id, self.threshold))
        if self.threshold is None:
            return Decision(candidate.example_id, None, "ABSTAIN", "policy_not_fitted", candidate.score, None, evidence_id)
        if candidate.host_id and candidate.host_id != host.host_id:
            return Decision(candidate.example_id, None, "ABSTAIN", "host_mismatch", candidate.score, self.threshold, evidence_id)
        if candidate.cost > host.max_cost or candidate.risk > host.max_risk:
            return Decision(candidate.example_id, None, "ABSTAIN", "host_budget", candidate.score, self.threshold, evidence_id)
        if candidate.evidence_count < host.min_evidence:
            return Decision(candidate.example_id, None, "ABSTAIN", "evidence_deficit", candidate.score, self.threshold, evidence_id)
        if candidate.score < self.threshold:
            return Decision(candidate.example_id, None, "ABSTAIN", "below_threshold", candidate.score, self.threshold, evidence_id)
        return Decision(candidate.example_id, candidate.label, "ACCEPT", "calibrated_candidate", candidate.score, self.threshold, evidence_id)

    def update(self, feedback: VerifiedFeedback) -> bool:
        """Accept only verified, provenance-bearing feedback within a finite budget."""
        if not feedback.example_id or not feedback.host_id or not feedback.provenance or not isinstance(feedback.correct, bool):
            return False
        if len(self._feedback) >= self.max_updates:
            return False
        if any(x.example_id == feedback.example_id for x in self._feedback):
            return False
        self._feedback.append(feedback)
        return True

    def inspect(self) -> dict[str, object]:
        return {"threshold": self.threshold, "feedback_count": len(self._feedback), "authority": "none", "updates_bounded": True}
