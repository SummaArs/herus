"""Domain-specific readiness gates for HERUS real-world deployment."""
from __future__ import annotations

from dataclasses import dataclass
from enum import Enum


class Readiness(str, Enum):
    PROPOSAL = "PROPOSAL"
    SIMULATE = "SIMULATE"
    SHADOW = "SHADOW"
    CANARY = "CANARY"
    HUMAN_BOUND = "HUMAN_BOUND"
    PRODUCTION = "PRODUCTION"


@dataclass(frozen=True)
class Evidence:
    data_provenance: bool
    hidden_tests: bool
    adversarial_tests: bool
    drift_policy: bool
    rollback: bool
    human_review: bool
    external_authority: bool
    physical_validation: bool


def highest_allowed(domain: str, evidence: Evidence) -> Readiness:
    if not (evidence.data_provenance and evidence.hidden_tests and evidence.adversarial_tests):
        return Readiness.PROPOSAL
    if domain == "finance_sandbox":
        return Readiness.SIMULATE
    if domain == "critical":
        if evidence.drift_policy and evidence.human_review:
            return Readiness.SHADOW
        return Readiness.SIMULATE
    if domain == "robotics":
        if evidence.drift_policy and evidence.rollback:
            return Readiness.SHADOW
        return Readiness.SIMULATE
    if domain == "server":
        if evidence.drift_policy and evidence.rollback:
            return Readiness.CANARY
        return Readiness.SHADOW
    if domain == "pulse":
        if evidence.physical_validation and evidence.human_review:
            return Readiness.HUMAN_BOUND
        return Readiness.SIMULATE
    return Readiness.PROPOSAL


def production_is_forbidden(evidence: Evidence) -> bool:
    """Production cannot be promoted by adaptation alone."""
    return not (evidence.external_authority and evidence.human_review)
