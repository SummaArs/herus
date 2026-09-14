"""Finite domain contracts for HERUS multi-host adaptation."""
from __future__ import annotations

from dataclasses import dataclass
from enum import Enum
from typing import FrozenSet


class Domain(str, Enum):
    PULSE = "pulse"
    ROBOTICS = "robotics"
    FINANCE_SANDBOX = "finance_sandbox"
    SERVER = "server"
    CRITICAL = "critical"


class EffectLevel(str, Enum):
    OBSERVE = "OBSERVE"
    PROPOSE = "PROPOSE"
    SIMULATE = "SIMULATE"
    SHADOW = "SHADOW"
    CANARY = "CANARY"
    HUMAN_BOUND = "HUMAN_BOUND"
    ACTIVE = "ACTIVE"


@dataclass(frozen=True)
class DomainContract:
    domain: Domain
    maximum_effect: EffectLevel
    vocabulary: FrozenSet[str]
    required_evidence: FrozenSet[str]
    forbidden_effects: FrozenSet[str]

    def permits(self, requested_effect: EffectLevel, evidence: set[str]) -> bool:
        order = list(EffectLevel)
        if order.index(requested_effect) > order.index(self.maximum_effect):
            return False
        if not self.required_evidence.issubset(evidence):
            return False
        return requested_effect is not EffectLevel.ACTIVE


DEFAULT_CONTRACTS = {
    Domain.PULSE: DomainContract(Domain.PULSE, EffectLevel.HUMAN_BOUND, frozenset({"ALERT", "CONFIRM", "STATUS"}), frozenset({"host_digest", "skill_digest"}), frozenset({"EXECUTE_ACTUATOR", "TRANSFER_FUNDS"})),
    Domain.ROBOTICS: DomainContract(Domain.ROBOTICS, EffectLevel.SIMULATE, frozenset({"POSE", "STOP", "ZONE", "PLAN"}), frozenset({"host_digest", "world_digest", "safety_check"}), frozenset({"MOVE_ROBOT", "ENTER_HAZARD_ZONE"})),
    Domain.FINANCE_SANDBOX: DomainContract(Domain.FINANCE_SANDBOX, EffectLevel.SIMULATE, frozenset({"QUOTE", "REGIME", "RISK", "PROPOSAL"}), frozenset({"host_digest", "data_digest", "sandbox_id"}), frozenset({"PLACE_ORDER", "TRANSFER_FUNDS"})),
    Domain.SERVER: DomainContract(Domain.SERVER, EffectLevel.CANARY, frozenset({"DEPLOY", "ROLLBACK", "HEALTH", "PLAN"}), frozenset({"host_digest", "artifact_digest", "rollback_plan"}), frozenset({"IRREVERSIBLE_MIGRATION", "DELETE_DATA"})),
    Domain.CRITICAL: DomainContract(Domain.CRITICAL, EffectLevel.SHADOW, frozenset({"ALERT", "RISK", "STATUS", "RECOMMEND"}), frozenset({"host_digest", "sensor_digest", "human_review"}), frozenset({"CONTROL_PROCESS", "BYPASS_INTERLOCK"})),
}


def contract_for(domain: Domain) -> DomainContract:
    return DEFAULT_CONTRACTS[domain]
