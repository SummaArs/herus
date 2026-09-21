"""Round 10: proof-carrying planner, verifier and simulator executor.

This module is intentionally simulator-only. It has no HostAdapter, network,
radio, subprocess or physical execution path. External/irreversible effects are
blocked before mutation.
"""
from __future__ import annotations

from dataclasses import dataclass
import hashlib
import hmac
import json
from typing import Any, Mapping

TRUST_ROOT = b"herus-round10-simulator-root-v1"


class ProtocolError(ValueError):
    pass


def _state(value: Mapping[str, int]) -> tuple[tuple[str, int], ...]:
    if not isinstance(value, Mapping) or any(isinstance(v, bool) or not isinstance(v, int) for v in value.values()):
        raise ProtocolError("STATE_TYPE")
    return tuple(sorted((str(k), v) for k, v in value.items()))


def _dict(value: tuple[tuple[str, int], ...]) -> dict[str, int]:
    return dict(value)


def _delta(before: Mapping[str, int], after: Mapping[str, int]) -> tuple[tuple[str, int], ...]:
    keys = set(before) | set(after)
    return tuple(sorted((key, int(after.get(key, 0) - before.get(key, 0))) for key in keys if after.get(key, 0) != before.get(key, 0)))


def _apply(before: Mapping[str, int], delta: tuple[tuple[str, int], ...]) -> dict[str, int]:
    result = dict(before)
    for key, change in delta:
        result[key] = result.get(key, 0) + change
    return result


def _pairs(value: Mapping[str, int]) -> tuple[tuple[str, int], ...]:
    return _state(value)


def _canonical(value: Any) -> bytes:
    return json.dumps(value, ensure_ascii=False, sort_keys=True, separators=(",", ":")).encode("utf-8")


@dataclass(frozen=True)
class ActionSignature:
    action_id: str
    argument: str
    preconditions: tuple[tuple[str, int], ...]
    postconditions: tuple[tuple[str, int], ...]
    delta: tuple[tuple[str, int], ...]
    irreversible: bool = False
    external_effect: bool = False

    def canonical(self) -> dict[str, Any]:
        return {
            "action_id": self.action_id,
            "argument": self.argument,
            "preconditions": list(self.preconditions),
            "postconditions": list(self.postconditions),
            "delta": list(self.delta),
            "irreversible": self.irreversible,
            "external_effect": self.external_effect,
        }

    def validate(self) -> None:
        if not self.action_id or not isinstance(self.action_id, str) or not isinstance(self.argument, str):
            raise ProtocolError("ACTION_ID_OR_ARGUMENT")
        if tuple(sorted(self.preconditions)) != self.preconditions or tuple(sorted(self.postconditions)) != self.postconditions or tuple(sorted(self.delta)) != self.delta:
            raise ProtocolError("ACTION_ORDER")
        if self.irreversible or self.external_effect:
            raise ProtocolError("EXTERNAL_EFFECT_BLOCKED")
        for key, value in self.preconditions + self.postconditions + self.delta:
            if not isinstance(key, str) or isinstance(value, bool) or not isinstance(value, int):
                raise ProtocolError("ACTION_VALUE_TYPE")


@dataclass(frozen=True)
class SkillContract:
    schema_version: int
    contract_version: int
    skill_id: str
    host_id: str
    host_digest: str
    epoch: int
    initial_state: tuple[tuple[str, int], ...]
    actions: tuple[ActionSignature, ...]
    goal: tuple[tuple[str, int], ...]
    max_steps: int
    expires_at: int
    nonce: str

    def canonical(self) -> dict[str, Any]:
        return {
            "schema_version": self.schema_version,
            "contract_version": self.contract_version,
            "skill_id": self.skill_id,
            "host_id": self.host_id,
            "host_digest": self.host_digest,
            "epoch": self.epoch,
            "initial_state": list(self.initial_state),
            "actions": [action.canonical() for action in self.actions],
            "goal": list(self.goal),
            "max_steps": self.max_steps,
            "expires_at": self.expires_at,
            "nonce": self.nonce,
        }

    @property
    def digest(self) -> str:
        return hashlib.sha256(_canonical(self.canonical())).hexdigest()

    def validate(self) -> None:
        if (self.schema_version, self.contract_version) != (1, 1):
            raise ProtocolError("VERSION")
        if not self.skill_id or not self.host_id or not self.host_digest or self.epoch < 0 or not self.nonce:
            raise ProtocolError("IDENTITY")
        if self.max_steps <= 0 or self.max_steps != len(self.actions) or self.expires_at < 0:
            raise ProtocolError("BUDGET")
        if len({a.action_id for a in self.actions}) != len(self.actions):
            raise ProtocolError("ACTION_COLLISION")
        if tuple(sorted(self.initial_state)) != self.initial_state or tuple(sorted(self.goal)) != self.goal:
            raise ProtocolError("STATE_ORDER")
        for action in self.actions:
            action.validate()


@dataclass(frozen=True)
class Evidence:
    host_id: str
    epoch: int
    sequence: int
    nonce: str
    action_id: str
    argument: str
    before: tuple[tuple[str, int], ...]
    after: tuple[tuple[str, int], ...]
    provenance: str
    signature: str

    @property
    def effect(self) -> tuple[tuple[str, int], ...]:
        return _delta(_dict(self.before), _dict(self.after))

    def unsigned(self) -> dict[str, Any]:
        return {
            "host_id": self.host_id, "epoch": self.epoch, "sequence": self.sequence,
            "nonce": self.nonce, "action_id": self.action_id, "argument": self.argument,
            "before": list(self.before), "after": list(self.after), "provenance": self.provenance,
            "effect": list(self.effect),
        }

    @property
    def digest(self) -> str:
        return hashlib.sha256(_canonical(self.unsigned())).hexdigest()

    @classmethod
    def issue(cls, *, host_id: str, epoch: int, sequence: int, nonce: str, action_id: str, argument: str, before: Mapping[str, int], after: Mapping[str, int], provenance: str = "sim-observer") -> "Evidence":
        draft = cls(host_id, epoch, sequence, nonce, action_id, argument, _state(before), _state(after), provenance, "")
        signature = hmac.new(TRUST_ROOT, _canonical(draft.unsigned()), hashlib.sha256).hexdigest()
        return cls(host_id, epoch, sequence, nonce, action_id, argument, draft.before, draft.after, provenance, signature)


@dataclass(frozen=True)
class Attestation:
    contract_digest: str
    host_id: str
    epoch: int
    nonce: str
    expires_at: int
    actions: tuple[tuple[str, str], ...]
    verifier_id: str
    proof: str


@dataclass(frozen=True)
class CapabilityToken:
    contract_digest: str
    host_id: str
    epoch: int
    nonce: str
    expires_at: int
    actions: tuple[tuple[str, str], ...]


@dataclass(frozen=True)
class Verification:
    accepted: bool
    reason: str
    attestation: Attestation | None = None


@dataclass(frozen=True)
class Execution:
    accepted: bool
    code: str
    state: tuple[tuple[str, int], ...]
    rolled_back: bool
    steps: int


class Planner:
    """Pure planner: it only returns the ordered declarative action signatures."""

    def propose(self, contract: SkillContract) -> tuple[ActionSignature, ...]:
        contract.validate()
        return contract.actions


class Verifier:
    """Independent, no-execution verifier for simulator contracts."""

    def __init__(self, verifier_id: str = "round10-verifier") -> None:
        self.verifier_id = verifier_id

    def verify(self, contract: SkillContract, evidence: tuple[Evidence, ...], now: int) -> Verification:
        try:
            contract.validate()
            if now > contract.expires_at:
                raise ProtocolError("EXPIRED")
            if len(evidence) != len(contract.actions):
                raise ProtocolError("EVIDENCE_COUNT")
            expected_state = _dict(contract.initial_state)
            for index, (action, item) in enumerate(zip(contract.actions, evidence), start=1):
                if item.host_id != contract.host_id or item.epoch != contract.epoch or item.nonce != contract.nonce or item.sequence != index:
                    raise ProtocolError("HOST_EPOCH_SEQUENCE")
                if (item.action_id, item.argument) != (action.action_id, action.argument):
                    raise ProtocolError("ACTION_MISMATCH")
                if _dict(item.before) != expected_state:
                    raise ProtocolError("BEFORE_MISMATCH")
                expected_signature = hmac.new(TRUST_ROOT, _canonical(item.unsigned()), hashlib.sha256).hexdigest()
                if not hmac.compare_digest(expected_signature, item.signature):
                    raise ProtocolError("EVIDENCE_SIGNATURE")
                if any(expected_state.get(key) != value for key, value in action.preconditions):
                    raise ProtocolError("PRECONDITION")
                if item.effect != action.delta or _apply(expected_state, action.delta) != _dict(item.after):
                    raise ProtocolError("EFFECT_DERIVATION")
                if any(_dict(item.after).get(key) != value for key, value in action.postconditions):
                    raise ProtocolError("POSTCONDITION")
                expected_state = _dict(item.after)
            if any(expected_state.get(key) != value for key, value in contract.goal):
                raise ProtocolError("GOAL")
            actions = tuple((a.action_id, a.argument) for a in contract.actions)
            proof_data = {"contract": contract.digest, "host": contract.host_id, "epoch": contract.epoch, "nonce": contract.nonce, "actions": list(actions), "expires_at": contract.expires_at, "verifier": self.verifier_id}
            proof = hmac.new(TRUST_ROOT, _canonical(proof_data), hashlib.sha256).hexdigest()
            return Verification(True, "ACCEPTED", Attestation(contract.digest, contract.host_id, contract.epoch, contract.nonce, contract.expires_at, actions, self.verifier_id, proof))
        except ProtocolError as error:
            return Verification(False, str(error))


class Simulator:
    def __init__(self, state: Mapping[str, int], *, fail_at: int | None = None, timeout_at: int | None = None, reentrant: bool = False, diverge_at: int | None = None) -> None:
        self.state = _state(state)
        self.fail_at = fail_at
        self.timeout_at = timeout_at
        self.reentrant = reentrant
        self.diverge_at = diverge_at


class SimulatorExecutor:
    """The only executor in Round 10; it cannot reach physical adapters."""

    def run(self, contract: SkillContract, attestation: Attestation | None, token: CapabilityToken | None, simulator: Simulator, now: int) -> Execution:
        snapshot = simulator.state
        try:
            contract.validate()
            if attestation is None or token is None:
                raise ProtocolError("ATTESTATION_REQUIRED")
            actions = tuple((a.action_id, a.argument) for a in contract.actions)
            if attestation.contract_digest != contract.digest or token.contract_digest != contract.digest or attestation.actions != actions or token.actions != actions:
                raise ProtocolError("SCOPE")
            if (attestation.host_id, token.host_id, attestation.epoch, token.epoch, attestation.nonce, token.nonce) != (contract.host_id, contract.host_id, contract.epoch, contract.epoch, contract.nonce, contract.nonce):
                raise ProtocolError("IDENTITY")
            if now > min(attestation.expires_at, token.expires_at):
                raise ProtocolError("EXPIRED")
            proof_data = {"contract": contract.digest, "host": contract.host_id, "epoch": contract.epoch, "nonce": contract.nonce, "actions": list(actions), "expires_at": contract.expires_at, "verifier": attestation.verifier_id}
            expected_proof = hmac.new(TRUST_ROOT, _canonical(proof_data), hashlib.sha256).hexdigest()
            if not hmac.compare_digest(expected_proof, attestation.proof):
                raise ProtocolError("ATTESTATION_PROOF")
            expected = _dict(contract.initial_state)
            if _dict(simulator.state) != expected:
                raise ProtocolError("STATE_DIGEST")
            for index, action in enumerate(contract.actions):
                if simulator.reentrant:
                    raise ProtocolError("REENTRANT")
                if simulator.timeout_at == index:
                    raise ProtocolError("TIMEOUT")
                if simulator.fail_at == index:
                    raise ProtocolError("PARTIAL_FAILURE")
                current = _dict(simulator.state)
                if any(current.get(key) != value for key, value in action.preconditions):
                    raise ProtocolError("STATE_DIVERGED")
                if action.external_effect or action.irreversible:
                    raise ProtocolError("EXTERNAL_EFFECT_BLOCKED")
                after = _apply(current, action.delta)
                if any(after.get(key) != value for key, value in action.postconditions):
                    raise ProtocolError("POSTCONDITION")
                simulator.state = _state(after)
                if simulator.diverge_at == index:
                    simulator.state = _state({**after, "__diverged__": 1})
                if _dict(simulator.state) != after:
                    raise ProtocolError("STATE_DIVERGED")
            if any(_dict(simulator.state).get(key) != value for key, value in contract.goal):
                raise ProtocolError("GOAL")
            return Execution(True, "EXECUTED_SIMULATOR_ONLY", simulator.state, False, len(contract.actions))
        except ProtocolError as error:
            simulator.state = snapshot
            return Execution(False, str(error), simulator.state, True, 0)


def capability_for(contract: SkillContract) -> CapabilityToken:
    contract.validate()
    return CapabilityToken(contract.digest, contract.host_id, contract.epoch, contract.nonce, contract.expires_at, tuple((a.action_id, a.argument) for a in contract.actions))


def sample_contract() -> SkillContract:
    actions = (
        ActionSignature("arm", "none", (("armed", 0),), (("armed", 1),), (("armed", 1),)),
        ActionSignature("commit", "none", (("armed", 1),), (("armed", 1), ("committed", 1)), (("committed", 1),)),
    )
    return SkillContract(1, 1, "skill-round10-sample", "sim-host", "sim-host-digest", 3, (("armed", 0), ("committed", 0)), actions, (("committed", 1),), 2, 100, "nonce-round10")
