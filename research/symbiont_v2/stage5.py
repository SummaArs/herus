from __future__ import annotations

from dataclasses import dataclass, asdict
from enum import Enum
import hashlib
import json
from typing import Any, Mapping


class DecisionMode(str, Enum):
    STRICT = "STRICT"
    COMPATIBILITY = "COMPATIBILITY"


class ProposalStatus(str, Enum):
    PROPOSED = "PROPOSED"
    ABSTAIN = "ABSTAIN"
    UNSUPPORTED_BY_CONTRACT = "UNSUPPORTED_BY_CONTRACT"


class SafetyClaim(str, Enum):
    NONE = "NONE"
    SAFE_BUT_UNPROVEN = "SAFE_BUT_UNPROVEN"
    SUPPORTED = "SUPPORTED"


class FieldStatus(str, Enum):
    PRESENT = "PRESENT"
    MISSING = "MISSING"
    STALE = "STALE"
    HIDDEN = "HIDDEN"
    UNDECLARED = "UNDECLARED"


class Coverage(str, Enum):
    COMPLETE_DECLARED = "COMPLETE_DECLARED"
    PARTIAL = "PARTIAL"
    UNDECLARED = "UNDECLARED"


class ProbeMode(str, Enum):
    NON_MUTATING = "NON_MUTATING"
    MUTATING_AUTHORIZED = "MUTATING_AUTHORIZED"
    UNKNOWN = "UNKNOWN"


class CostStatus(str, Enum):
    EXACT = "EXACT"
    BOUNDED_UNKNOWN = "BOUNDED_UNKNOWN"
    UNKNOWN = "UNKNOWN"
    OVERRUN = "OVERRUN"


class BudgetState(str, Enum):
    OPEN = "OPEN"
    EXHAUSTED = "EXHAUSTED"
    OVERRUN = "OVERRUN"
    UNKNOWN = "UNKNOWN"


@dataclass(frozen=True)
class BudgetLimits:
    probe_max: int
    reset_max: int = 0
    step_max: int = 0
    cost_max: int | None = None
    retry_max: int = 0
    recovery_max: int = 0

    def validate(self) -> tuple[str, ...]:
        values = asdict(self)
        errors: list[str] = []
        for name, value in values.items():
            if isinstance(value, bool) or not isinstance(value, int) and value is not None:
                errors.append(f"invalid:{name}")
            elif value is not None and value < 0:
                errors.append(f"negative:{name}")
        return tuple(errors)


@dataclass(frozen=True)
class BudgetLedger:
    budget_scope_id: str
    limits: BudgetLimits
    probe_count: int = 0
    reset_count: int = 0
    step_count: int = 0
    cost_count: int = 0
    retry_count: int = 0
    recovery_count: int = 0
    cost_status: CostStatus = CostStatus.UNKNOWN
    cost_actual: int | None = None
    budget_state: BudgetState = BudgetState.UNKNOWN
    event_seq: int = 0
    parent_ledger_digest: str | None = None

    def digest(self) -> str:
        payload = asdict(self)
        payload["limits"] = asdict(self.limits)
        payload["cost_status"] = self.cost_status.value
        payload["budget_state"] = self.budget_state.value
        raw = json.dumps(payload, sort_keys=True, separators=(",", ":")).encode()
        return hashlib.sha256(raw).hexdigest()

    def canonical(self) -> dict[str, Any]:
        result = asdict(self)
        result["limits"] = asdict(self.limits)
        result["cost_status"] = self.cost_status.value
        result["budget_state"] = self.budget_state.value
        result["ledger_digest"] = self.digest()
        return result

    def with_probe(self) -> "BudgetLedger":
        if self.probe_count >= self.limits.probe_max:
            return self._state(BudgetState.EXHAUSTED)
        return self._replace(probe_count=self.probe_count + 1)

    def with_reset(self) -> "BudgetLedger":
        if self.reset_count >= self.limits.reset_max:
            return self._state(BudgetState.EXHAUSTED)
        return self._replace(reset_count=self.reset_count + 1)

    def _state(self, state: BudgetState) -> "BudgetLedger":
        return self._replace(budget_state=state)

    def _replace(self, **changes: Any) -> "BudgetLedger":
        from dataclasses import replace
        return replace(self, **changes, event_seq=self.event_seq + 1)


@dataclass(frozen=True)
class SkillObservabilityContract:
    schema: str = "herus-skill-observability-v1"
    coverage_requirement: Coverage = Coverage.COMPLETE_DECLARED
    effect_closure: str = "EXTERNAL_ATTESTATION_REQUIRED"
    closure_evidence_digest: str | None = None
    contract_digest: str = ""

    def valid_for_strict(self) -> bool:
        return (
            self.schema == "herus-skill-observability-v1"
            and self.coverage_requirement == Coverage.COMPLETE_DECLARED
            and self.effect_closure == "EXTERNAL_ATTESTATION_REQUIRED"
            and bool(self.closure_evidence_digest)
            and bool(self.contract_digest)
        )


@dataclass(frozen=True)
class HostObservabilityContract:
    schema: str
    coverage: Coverage
    probe_mode: ProbeMode
    effect_closure: str
    closure_evidence_digest: str | None
    contract_digest: str

    def valid_for_strict(self) -> bool:
        return (
            self.schema == "herus-host-observability-v1"
            and self.coverage == Coverage.COMPLETE_DECLARED
            and self.probe_mode == ProbeMode.NON_MUTATING
            and self.effect_closure == "EXTERNAL_ATTESTATION_REQUIRED"
            and bool(self.closure_evidence_digest)
            and bool(self.contract_digest)
        )


@dataclass(frozen=True)
class Diagnostic:
    phase: str
    code: str
    detail: str


@dataclass(frozen=True)
class TransferDecision:
    proposal: Any
    proposal_status: ProposalStatus
    safety_claim: SafetyClaim
    runtime_reason: str
    diagnostic: Diagnostic
    budget_ledger: BudgetLedger
    planned_steps: int
    host_id: str
    observation_digest: str | None
    skill_contract_digest: str | None
    probe_execute_calls: int
    proposal_execute_calls: int
    mode: DecisionMode

    @property
    def reason(self) -> str:
        return self.runtime_reason

    def canonical(self) -> dict[str, Any]:
        value = asdict(self)
        value["proposal_status"] = self.proposal_status.value
        value["safety_claim"] = self.safety_claim.value
        value["diagnostic"] = asdict(self.diagnostic)
        value["budget_ledger"] = self.budget_ledger.canonical()
        value["mode"] = self.mode.value
        value["proposal"] = None if self.proposal is None else {"skill_id": self.proposal.skill_id, "host_id": self.proposal.host_id}
        return value


def initial_ledger(scope: str, limits: BudgetLimits) -> BudgetLedger:
    errors = limits.validate()
    if errors:
        raise ValueError("invalid budget: " + ",".join(errors))
    return BudgetLedger(budget_scope_id=scope, limits=limits, budget_state=BudgetState.OPEN, cost_status=CostStatus.UNKNOWN)
