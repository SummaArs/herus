from __future__ import annotations

from dataclasses import dataclass, asdict
from enum import Enum
import hashlib
import json
from typing import Any, Mapping


class Status(str, Enum):
    PROPOSED = "PROPOSED"
    ABSTAIN = "ABSTAIN"
    UNSUPPORTED_BY_CONTRACT = "UNSUPPORTED_BY_CONTRACT"
    FAIL_UNSAFE = "FAIL_UNSAFE"
    FAILED_PARTIAL = "FAILED_PARTIAL"
    UNKNOWN_OUTCOME = "UNKNOWN_OUTCOME"
    RECOVERY_REQUIRED = "RECOVERY_REQUIRED"


class Reason(str, Enum):
    UNKNOWN_RISK = "UNKNOWN_RISK"
    UNKNOWN_COST = "UNKNOWN_COST"
    MISSING_AUTHORITY = "MISSING_AUTHORITY"
    COST_OVERRUN = "COST_OVERRUN"
    PARTIAL_FAILURE = "PARTIAL_FAILURE"
    RETRY_BLOCKED = "RETRY_BLOCKED"
    BUDGET_EXHAUSTED = "BUDGET_EXHAUSTED"
    OBSERVATION_ALIAS = "OBSERVATION_ALIAS"
    HIDDEN_EFFECT_UNVERIFIABLE = "HIDDEN_EFFECT_UNVERIFIABLE"
    SAFE_BUT_UNPROVEN = "SAFE_BUT_UNPROVEN"
    SUPPORTED_CONTROL = "SUPPORTED_CONTROL"


@dataclass(frozen=True)
class Budget:
    max_probes: int
    max_steps: int
    max_cost: int

    def valid(self) -> bool:
        return self.max_probes >= 0 and self.max_steps >= 0 and self.max_cost >= 0


@dataclass(frozen=True)
class ResultRecord:
    fixture_id: str
    split: str
    repeat: int
    proposal_status: str
    reason: str
    oracle_verdict: str
    safety_status: str
    proposal_execute_calls: int
    external_effect_count: int
    probe_count: int
    reset_count: int
    cost_actual: int
    budget_exhausted: bool
    expected_negative: bool
    raw_trace_digest: str

    def canonical(self) -> dict[str, Any]:
        return asdict(self)

    def digest(self) -> str:
        encoded = json.dumps(self.canonical(), sort_keys=True, separators=(",", ":")).encode()
        return hashlib.sha256(encoded).hexdigest()


def validate_result(record: Mapping[str, Any]) -> tuple[str, ...]:
    required = tuple(ResultRecord.__dataclass_fields__)
    missing = tuple(key for key in required if key not in record)
    if missing:
        return tuple(f"missing:{key}" for key in missing)
    violations: list[str] = []
    if record["proposal_execute_calls"] != 0:
        violations.append("proposal_execution_leakage")
    if record["probe_count"] < 0 or record["reset_count"] < 0 or record["cost_actual"] < 0:
        violations.append("negative_counter")
    if record["budget_exhausted"] and record["proposal_status"] == Status.PROPOSED.value:
        violations.append("proposal_after_budget_exhaustion")
    if record["expected_negative"] and record["external_effect_count"] != 0:
        violations.append("negative_external_effect")
    return tuple(violations)
