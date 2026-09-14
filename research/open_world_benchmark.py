"""Finite open-world benchmark for the HERUS ASA/GOFAI comparison.

The benchmark is intentionally small and auditable. Hidden hosts expose one
capability outside the baseline vocabulary. The ASA may enumerate, inspect and
verify that capability, but it never executes an effect during evaluation.

The hostile cases are part of the contract: unstable descriptions, stale or
forged evidence, and missing authority must produce refusal rather than a
best-effort action.
"""
from __future__ import annotations

from dataclasses import dataclass
from enum import Enum
import hashlib
import json
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
    revision: int = 1


@dataclass(frozen=True)
class CapabilityEvidence:
    """Bounded, replayable evidence; it does not contain execution authority."""

    host: str
    capability: str
    task: str
    description: str
    revision: int
    source: str
    source_digest: str

    def unsigned(self) -> dict[str, object]:
        return {
            "host": self.host,
            "capability": self.capability,
            "task": self.task,
            "description": self.description,
            "revision": self.revision,
            "source": self.source,
        }

    def canonical(self) -> bytes:
        return json.dumps(self.unsigned(), sort_keys=True, separators=(",", ":")).encode()


@dataclass(frozen=True)
class Proposal:
    host: str
    capability: str
    task: str
    verified: bool
    executed: bool = False
    authority: str = "NONE"
    execution_authorized: bool = False
    evidence_digest: str = ""


@dataclass(frozen=True)
class Evaluation:
    decision: Decision
    proposal: Proposal | None
    probes: int
    reason: str


class HiddenOpenWorldOracle:
    """Test oracle; its capability is unavailable until queried."""

    SOURCE = "hidden-oracle-v1"

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

    def collect_evidence(self, capability: str) -> CapabilityEvidence:
        description = self.describe(capability)
        unsigned = {
            "host": self.case.name,
            "capability": capability,
            "task": self.case.task,
            "description": description,
            "revision": self.case.revision,
            "source": self.SOURCE,
        }
        source_digest = hashlib.sha256(
            json.dumps(unsigned, sort_keys=True, separators=(",", ":")).encode()
        ).hexdigest()
        return CapabilityEvidence(**unsigned, source_digest=source_digest)

    def verify(self, capability: str, task: str) -> bool:
        """Independent test oracle for the hidden ground truth."""
        return (
            self._enumerated
            and capability == self.case.capability
            and task == self.case.task
            and bool(self.describe(capability))
        )


BASELINE_VOCABULARY = frozenset({"telemetry", "haptic", "serial"})


def _verify_evidence(evidence: CapabilityEvidence, expected_task: str) -> bool:
    """Verify integrity and task binding without granting authority."""
    expected_digest = hashlib.sha256(evidence.canonical()).hexdigest()
    return (
        evidence.revision > 0
        and evidence.source == HiddenOpenWorldOracle.SOURCE
        and evidence.task == expected_task
        and bool(evidence.description)
        and evidence.source_digest == expected_digest
        and evidence.description == f"bounded capability for task={expected_task}"
    )


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
    if len(candidates) != 1 or not candidates[0]:
        return Evaluation(Decision.REFUSED, None, probes, "ambiguous_capability_catalog")
    capability = candidates[0]

    probes += 1
    try:
        first_evidence = oracle.collect_evidence(capability)
        second_evidence = oracle.collect_evidence(capability)
    except (TypeError, ValueError, AttributeError) as exc:
        return Evaluation(Decision.REFUSED, None, probes, str(exc))
    if first_evidence != second_evidence:
        return Evaluation(Decision.REFUSED, None, probes, "evidence_conflict")

    probes += 1
    if not _verify_evidence(first_evidence, oracle.case.task):
        return Evaluation(Decision.REFUSED, None, probes, "evidence_integrity_or_binding_failed")
    if not oracle.verify(capability, oracle.case.task):
        return Evaluation(Decision.REFUSED, None, probes, "independent_skill_verification_failed")

    proposal = Proposal(
        oracle.case.name,
        capability,
        oracle.case.task,
        verified=True,
        executed=False,
        authority="NONE",
        execution_authorized=False,
        evidence_digest=first_evidence.source_digest,
    )
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
