"""Finite open-world benchmark for the HERUS ASA/GOFAI comparison.

The benchmark is intentionally small and auditable. Hidden hosts expose one
capability outside the baseline vocabulary. The ASA may enumerate, inspect and
verify that capability, but it never executes an effect during evaluation.
"""
from __future__ import annotations

from dataclasses import dataclass
from enum import Enum
from typing import Iterable


class Decision(str, Enum):
    ADAPTED = "ADAPTED"
    REFUSED = "REFUSED"
    BASELINE_BLOCKED = "BASELINE_BLOCKED"


@dataclass(frozen=True)
class HostCase:
    name: str
    capability: str
    task: str
    safe_effect: str


@dataclass(frozen=True)
class Proposal:
    host: str
    capability: str
    task: str
    verified: bool
    executed: bool = False


@dataclass(frozen=True)
class Evaluation:
    decision: Decision
    proposal: Proposal | None
    probes: int
    reason: str


class HiddenOpenWorldOracle:
    """Test oracle; its capability is unavailable until queried."""

    def __init__(self, case: HostCase):
        self.case = case
        self._enumerated = False

    def enumerate_capabilities(self) -> tuple[str, ...]:
        self._enumerated = True
        return (self.case.capability,)

    def describe(self, capability: str) -> str:
        if not self._enumerated:
            raise ValueError("enumeration_required")
        if capability != self.case.capability:
            raise ValueError("capability_unknown")
        return f"bounded capability for task={self.case.task}"

    def verify(self, capability: str, task: str) -> bool:
        return (
            self._enumerated
            and capability == self.case.capability
            and task == self.case.task
            and bool(self.describe(capability))
        )


BASELINE_VOCABULARY = frozenset({"telemetry", "haptic", "serial"})


def gofai_baseline(case: HostCase) -> Evaluation:
    """Closed-world planner: unknown capability blocks before execution."""
    if case.capability not in BASELINE_VOCABULARY:
        return Evaluation(Decision.BASELINE_BLOCKED, None, 0, "operator_not_in_domain")
    proposal = Proposal(case.name, case.capability, case.task, verified=True, executed=False)
    return Evaluation(Decision.ADAPTED, proposal, 0, "known_operator")


def asa_adapt(oracle: HiddenOpenWorldOracle) -> Evaluation:
    """Discover, inspect and verify a bounded skill without executing it."""
    probes = 0
    candidates = oracle.enumerate_capabilities()
    probes += 1
    if len(candidates) != 1:
        return Evaluation(Decision.REFUSED, None, probes, "ambiguous_capability_catalog")
    capability = candidates[0]
    probes += 1
    try:
        oracle.describe(capability)
    except ValueError as exc:
        return Evaluation(Decision.REFUSED, None, probes, str(exc))
    probes += 1
    if not oracle.verify(capability, oracle.case.task):
        return Evaluation(Decision.REFUSED, None, probes, "independent_skill_verification_failed")
    proposal = Proposal(oracle.case.name, capability, oracle.case.task, verified=True, executed=False)
    return Evaluation(Decision.ADAPTED, proposal, probes, "new_skill_verified_proposal_only")


def evaluate_suite(cases: Iterable[HostCase]) -> dict[str, tuple[Evaluation, Evaluation]]:
    results: dict[str, tuple[Evaluation, Evaluation]] = {}
    for case in cases:
        results[case.name] = (gofai_baseline(case), asa_adapt(HiddenOpenWorldOracle(case)))
    return results


def default_cases() -> tuple[HostCase, ...]:
    return (
        HostCase("pulse-v2", "context-sync", "sync-context", "write_context"),
        HostCase("notebook-gateway", "knowledge-gateway", "research-gap", "request_evidence"),
        HostCase("legacy-robot", "joint-limit-probe", "map-safe-motion", "propose_motion"),
    )
