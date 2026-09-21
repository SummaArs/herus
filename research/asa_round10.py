"""Round 10/11: proof-carrying, simulator-only ASA executor.

The protocol is deliberately bounded to an in-process deterministic simulator.
The default keys are test fixtures, not production authentication. There is no
HostAdapter, network, radio, subprocess or physical execution path.
"""
from __future__ import annotations

from dataclasses import dataclass
import hashlib
import hmac
import json
import math
import threading
from typing import Any, Mapping

TEST_HOST_KEY = b"herus-round11-test-host-key-v1"
TEST_VERIFIER_KEY = b"herus-round11-test-verifier-key-v1"
TEST_AUTHORITY_KEY = b"herus-round11-test-authority-key-v1"


class ProtocolError(ValueError):
    pass


def _canonical(value: Any) -> bytes:
    return json.dumps(value, ensure_ascii=False, sort_keys=True, separators=(",", ":")).encode("utf-8")


def _strict_int(value: Any, reason: str) -> int:
    if isinstance(value, bool) or not isinstance(value, int):
        raise ProtocolError(reason)
    return value


def _pairs(value: Any, reason: str = "PAIR_TYPE") -> tuple[tuple[str, int], ...]:
    if not isinstance(value, (tuple, list)):
        raise ProtocolError(reason)
    result: list[tuple[str, int]] = []
    seen: set[str] = set()
    for pair in value:
        if not isinstance(pair, (tuple, list)) or len(pair) != 2:
            raise ProtocolError("PAIR_ARITY")
        key, number = pair
        if not isinstance(key, str) or not key:
            raise ProtocolError("KEY_TYPE")
        number = _strict_int(number, "VALUE_TYPE")
        if key in seen:
            raise ProtocolError("DUPLICATE_KEY")
        seen.add(key)
        result.append((key, number))
    result_tuple = tuple(result)
    if result_tuple != tuple(sorted(result_tuple)):
        raise ProtocolError("PAIR_ORDER")
    return result_tuple


def _state(value: Mapping[str, int]) -> tuple[tuple[str, int], ...]:
    if not isinstance(value, Mapping):
        raise ProtocolError("STATE_TYPE")
    return _pairs(tuple(value.items()), "STATE_TYPE")


def _dict(value: tuple[tuple[str, int], ...]) -> dict[str, int]:
    return dict(value)


def _delta(before: Mapping[str, int], after: Mapping[str, int]) -> tuple[tuple[str, int], ...]:
    keys = set(before) | set(after)
    return tuple(sorted((key, after.get(key, 0) - before.get(key, 0)) for key in keys if after.get(key, 0) != before.get(key, 0)))


def _apply(before: Mapping[str, int], delta: tuple[tuple[str, int], ...]) -> dict[str, int]:
    result = dict(before)
    for key, change in delta:
        result[key] = result.get(key, 0) + change
    return result


def _mac(key: bytes, domain: str, value: Any) -> str:
    return hmac.new(key, domain.encode() + b":" + _canonical(value), hashlib.sha256).hexdigest()


@dataclass(frozen=True)
class TrustStore:
    host_keys: tuple[tuple[str, bytes], ...] = (("sim-host-key", TEST_HOST_KEY),)
    verifier_keys: tuple[tuple[str, bytes], ...] = (("sim-verifier-key", TEST_VERIFIER_KEY),)
    authority_keys: tuple[tuple[str, bytes], ...] = (("sim-authority-key", TEST_AUTHORITY_KEY),)
    allowed_verifiers: tuple[str, ...] = ("sim-verifier-key",)

    def _lookup(self, entries: tuple[tuple[str, bytes], ...], key_id: str, reason: str) -> bytes:
        for candidate, key in entries:
            if candidate == key_id:
                return key
        raise ProtocolError(reason)

    def host(self, key_id: str) -> bytes:
        return self._lookup(self.host_keys, key_id, "HOST_KEY")

    def verifier(self, key_id: str) -> bytes:
        if key_id not in self.allowed_verifiers:
            raise ProtocolError("VERIFIER_NOT_ALLOWED")
        return self._lookup(self.verifier_keys, key_id, "VERIFIER_KEY")

    def authority(self, key_id: str) -> bytes:
        return self._lookup(self.authority_keys, key_id, "AUTHORITY_KEY")


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
        return {"action_id": self.action_id, "argument": self.argument, "preconditions": list(self.preconditions), "postconditions": list(self.postconditions), "delta": list(self.delta), "irreversible": self.irreversible, "external_effect": self.external_effect}

    def validate(self, state_keys: tuple[str, ...]) -> None:
        if not isinstance(self.action_id, str) or not self.action_id or not isinstance(self.argument, str):
            raise ProtocolError("ACTION_ID_OR_ARGUMENT")
        for collection in (self.preconditions, self.postconditions, self.delta):
            checked = _pairs(collection)
            if any(key not in state_keys for key, _ in checked):
                raise ProtocolError("UNKNOWN_STATE_KEY")
        if not isinstance(self.irreversible, bool) or not isinstance(self.external_effect, bool):
            raise ProtocolError("ACTION_FLAG_TYPE")
        if self.irreversible or self.external_effect:
            raise ProtocolError("EXTERNAL_EFFECT_BLOCKED")


@dataclass(frozen=True)
class SkillContract:
    schema_version: int
    contract_version: int
    skill_id: str
    host_id: str
    host_digest: str
    host_key_id: str
    epoch: int
    state_keys: tuple[str, ...]
    initial_state: tuple[tuple[str, int], ...]
    actions: tuple[ActionSignature, ...]
    goal: tuple[tuple[str, int], ...]
    max_steps: int
    expires_at: int
    nonce: str

    def canonical(self) -> dict[str, Any]:
        return {"schema_version": self.schema_version, "contract_version": self.contract_version, "skill_id": self.skill_id, "host_id": self.host_id, "host_digest": self.host_digest, "host_key_id": self.host_key_id, "epoch": self.epoch, "state_keys": list(self.state_keys), "initial_state": list(self.initial_state), "actions": [a.canonical() for a in self.actions], "goal": list(self.goal), "max_steps": self.max_steps, "expires_at": self.expires_at, "nonce": self.nonce}

    @property
    def digest(self) -> str:
        return hashlib.sha256(_canonical(self.canonical())).hexdigest()

    def validate(self) -> None:
        _strict_int(self.schema_version, "VERSION_TYPE")
        _strict_int(self.contract_version, "VERSION_TYPE")
        if (self.schema_version, self.contract_version) != (1, 1):
            raise ProtocolError("VERSION")
        if any(not isinstance(v, str) or not v for v in (self.skill_id, self.host_id, self.host_digest, self.host_key_id, self.nonce)):
            raise ProtocolError("IDENTITY")
        _strict_int(self.epoch, "EPOCH_TYPE")
        _strict_int(self.max_steps, "BUDGET_TYPE")
        _strict_int(self.expires_at, "EXPIRY_TYPE")
        if self.epoch < 0 or self.max_steps <= 0 or self.max_steps != len(self.actions) or self.expires_at < 0:
            raise ProtocolError("BUDGET")
        if not isinstance(self.state_keys, tuple) or not self.state_keys or len(set(self.state_keys)) != len(self.state_keys) or self.state_keys != tuple(sorted(self.state_keys)):
            raise ProtocolError("STATE_KEYS")
        initial = _pairs(self.initial_state)
        goal = _pairs(self.goal)
        if tuple(key for key, _ in initial) != self.state_keys or any(key not in self.state_keys for key, _ in goal):
            raise ProtocolError("STATE_SHAPE")
        if len({a.action_id for a in self.actions}) != len(self.actions):
            raise ProtocolError("ACTION_COLLISION")
        for action in self.actions:
            action.validate(self.state_keys)


@dataclass(frozen=True)
class Evidence:
    contract_digest: str
    skill_id: str
    host_id: str
    host_digest: str
    host_key_id: str
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
        return {"contract_digest": self.contract_digest, "skill_id": self.skill_id, "host_id": self.host_id, "host_digest": self.host_digest, "host_key_id": self.host_key_id, "epoch": self.epoch, "sequence": self.sequence, "nonce": self.nonce, "action_id": self.action_id, "argument": self.argument, "before": list(self.before), "after": list(self.after), "provenance": self.provenance, "effect": list(self.effect)}

    @classmethod
    def issue(cls, *, contract: SkillContract, sequence: int, action_id: str, argument: str, before: Mapping[str, int], after: Mapping[str, int], store: TrustStore | None = None, provenance: str = "sim-observer") -> "Evidence":
        store = store or TrustStore()
        draft = cls(contract.digest, contract.skill_id, contract.host_id, contract.host_digest, contract.host_key_id, contract.epoch, sequence, contract.nonce, action_id, argument, _state(before), _state(after), provenance, "")
        signature = _mac(store.host(contract.host_key_id), "evidence-v2", draft.unsigned())
        return cls(draft.contract_digest, draft.skill_id, draft.host_id, draft.host_digest, draft.host_key_id, draft.epoch, draft.sequence, draft.nonce, draft.action_id, draft.argument, draft.before, draft.after, draft.provenance, signature)


@dataclass(frozen=True)
class Attestation:
    contract_digest: str
    host_id: str
    host_digest: str
    epoch: int
    nonce: str
    expires_at: int
    actions: tuple[tuple[str, str], ...]
    verifier_id: str
    proof: str

    def unsigned(self) -> dict[str, Any]:
        return {"contract_digest": self.contract_digest, "host_id": self.host_id, "host_digest": self.host_digest, "epoch": self.epoch, "nonce": self.nonce, "expires_at": self.expires_at, "actions": list(self.actions), "verifier_id": self.verifier_id}


@dataclass(frozen=True)
class CapabilityToken:
    contract_digest: str
    host_id: str
    host_digest: str
    epoch: int
    nonce: str
    expires_at: int
    actions: tuple[tuple[str, str], ...]
    authority_id: str
    proof: str

    def unsigned(self) -> dict[str, Any]:
        return {"contract_digest": self.contract_digest, "host_id": self.host_id, "host_digest": self.host_digest, "epoch": self.epoch, "nonce": self.nonce, "expires_at": self.expires_at, "actions": list(self.actions), "authority_id": self.authority_id}


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
    def propose(self, contract: SkillContract) -> tuple[ActionSignature, ...]:
        contract.validate()
        return contract.actions


class Verifier:
    def __init__(self, store: TrustStore | None = None, verifier_id: str = "sim-verifier-key") -> None:
        self.store = store or TrustStore()
        self.verifier_id = verifier_id

    def verify(self, contract: SkillContract, evidence: tuple[Evidence, ...], now: int) -> Verification:
        try:
            contract.validate()
            now = _strict_int(now, "NOW_TYPE")
            if now >= contract.expires_at:
                raise ProtocolError("EXPIRED")
            if not isinstance(evidence, tuple) or len(evidence) != len(contract.actions):
                raise ProtocolError("EVIDENCE_COUNT")
            expected_state = _dict(contract.initial_state)
            for index, (action, item) in enumerate(zip(contract.actions, evidence), start=1):
                if not isinstance(item, Evidence):
                    raise ProtocolError("EVIDENCE_TYPE")
                if (item.contract_digest, item.skill_id, item.host_id, item.host_digest, item.host_key_id, item.epoch, item.sequence, item.nonce) != (contract.digest, contract.skill_id, contract.host_id, contract.host_digest, contract.host_key_id, contract.epoch, index, contract.nonce):
                    raise ProtocolError("IDENTITY_SEQUENCE")
                if (item.action_id, item.argument) != (action.action_id, action.argument):
                    raise ProtocolError("ACTION_MISMATCH")
                if _dict(item.before) != expected_state:
                    raise ProtocolError("BEFORE_MISMATCH")
                if not hmac.compare_digest(_mac(self.store.host(contract.host_key_id), "evidence-v2", item.unsigned()), item.signature):
                    raise ProtocolError("EVIDENCE_SIGNATURE")
                if any(expected_state.get(key) != value for key, value in action.preconditions):
                    raise ProtocolError("PRECONDITION")
                if item.effect != action.delta or _apply(expected_state, action.delta) != _dict(item.after):
                    raise ProtocolError("EFFECT_DERIVATION")
                if any(_dict(item.after).get(key) != value for key, value in action.postconditions):
                    raise ProtocolError("POSTCONDITION")
                if tuple(key for key, _ in item.after) != contract.state_keys:
                    raise ProtocolError("STATE_SHAPE")
                expected_state = _dict(item.after)
            if any(expected_state.get(key) != value for key, value in contract.goal):
                raise ProtocolError("GOAL")
            actions = tuple((a.action_id, a.argument) for a in contract.actions)
            attestation = Attestation(contract.digest, contract.host_id, contract.host_digest, contract.epoch, contract.nonce, contract.expires_at, actions, self.verifier_id, "")
            proof = _mac(self.store.verifier(self.verifier_id), "attestation-v2", attestation.unsigned())
            return Verification(True, "ACCEPTED", Attestation(attestation.contract_digest, attestation.host_id, attestation.host_digest, attestation.epoch, attestation.nonce, attestation.expires_at, attestation.actions, attestation.verifier_id, proof))
        except (ProtocolError, TypeError, ValueError, KeyError, AttributeError) as error:
            return Verification(False, str(error) or "INVALID_INPUT")


class Simulator:
    def __init__(self, state: Mapping[str, int], *, fail_at: int | None = None, timeout_at: int | None = None, reentrant: bool = False, diverge_at: int | None = None) -> None:
        self.state = _state(state)
        self.fail_at = fail_at
        self.timeout_at = timeout_at
        self.reentrant = reentrant
        self.diverge_at = diverge_at


class SimulatorExecutor:
    def __init__(self, store: TrustStore | None = None) -> None:
        self.store = store or TrustStore()
        self._consumed: set[tuple[str, str, str]] = set()
        self._quarantined: set[str] = set()
        self._lock = threading.Lock()

    def run(self, contract: SkillContract, attestation: Attestation | None, token: CapabilityToken | None, simulator: Simulator, now: int) -> Execution:
        if not self._lock.acquire(blocking=False):
            return Execution(False, "REENTRANT", getattr(simulator, "state", ()), True, 0)
        snapshot: tuple[tuple[str, int], ...] = ()
        try:
            if not isinstance(simulator, Simulator):
                raise ProtocolError("SIMULATOR_TYPE")
            contract.validate()
            now = _strict_int(now, "NOW_TYPE")
            snapshot = simulator.state
            if attestation is None or token is None:
                raise ProtocolError("ATTESTATION_REQUIRED")
            actions = tuple((a.action_id, a.argument) for a in contract.actions)
            if (attestation.contract_digest, token.contract_digest, attestation.host_id, token.host_id, attestation.host_digest, token.host_digest, attestation.epoch, token.epoch, attestation.nonce, token.nonce, attestation.expires_at, token.expires_at, attestation.actions, token.actions) != (contract.digest, contract.digest, contract.host_id, contract.host_id, contract.host_digest, contract.host_digest, contract.epoch, contract.epoch, contract.nonce, contract.nonce, contract.expires_at, contract.expires_at, actions, actions):
                raise ProtocolError("SCOPE")
            if now >= contract.expires_at:
                raise ProtocolError("EXPIRED")
            if not hmac.compare_digest(_mac(self.store.verifier(attestation.verifier_id), "attestation-v2", attestation.unsigned()), attestation.proof):
                raise ProtocolError("ATTESTATION_PROOF")
            if not hmac.compare_digest(_mac(self.store.authority(token.authority_id), "capability-v2", token.unsigned()), token.proof):
                raise ProtocolError("CAPABILITY_PROOF")
            invocation = (contract.digest, contract.nonce, token.authority_id)
            if invocation in self._consumed:
                raise ProtocolError("REPLAY")
            self._consumed.add(invocation)
            if _dict(simulator.state) != _dict(contract.initial_state):
                raise ProtocolError("STATE_DIGEST")
            for index, action in enumerate(contract.actions):
                if simulator.reentrant:
                    raise ProtocolError("REENTRANT")
                if now >= contract.expires_at:
                    raise ProtocolError("EXPIRED")
                if simulator.timeout_at == index:
                    raise ProtocolError("TIMEOUT")
                if simulator.fail_at == index:
                    raise ProtocolError("PARTIAL_FAILURE")
                current = _dict(simulator.state)
                if any(current.get(key) != value for key, value in action.preconditions):
                    raise ProtocolError("STATE_DIVERGED")
                after = _apply(current, action.delta)
                if tuple(sorted(after)) != contract.state_keys or any(after.get(key) != value for key, value in action.postconditions):
                    raise ProtocolError("POSTCONDITION")
                simulator.state = _state(after)
                if simulator.diverge_at == index:
                    divergent = dict(after)
                    divergent[contract.state_keys[0]] += 1
                    simulator.state = _state(divergent)
                if simulator.state != _state(after):
                    raise ProtocolError("STATE_DIVERGED")
            if any(_dict(simulator.state).get(key) != value for key, value in contract.goal):
                raise ProtocolError("GOAL")
            return Execution(True, "EXECUTED_SIMULATOR_ONLY", simulator.state, False, len(contract.actions))
        except (ProtocolError, TypeError, ValueError, KeyError, AttributeError) as error:
            if isinstance(simulator, Simulator):
                simulator.state = snapshot
            self._quarantined.add(contract.digest if isinstance(contract, SkillContract) else "invalid")
            return Execution(False, str(error) or "INVALID_INPUT", snapshot, True, 0)
        finally:
            self._lock.release()


def capability_for(contract: SkillContract, store: TrustStore | None = None, authority_id: str = "sim-authority-key") -> CapabilityToken:
    store = store or TrustStore()
    contract.validate()
    actions = tuple((a.action_id, a.argument) for a in contract.actions)
    unsigned = CapabilityToken(contract.digest, contract.host_id, contract.host_digest, contract.epoch, contract.nonce, contract.expires_at, actions, authority_id, "")
    proof = _mac(store.authority(authority_id), "capability-v2", unsigned.unsigned())
    return CapabilityToken(unsigned.contract_digest, unsigned.host_id, unsigned.host_digest, unsigned.epoch, unsigned.nonce, unsigned.expires_at, unsigned.actions, unsigned.authority_id, proof)


def sample_contract() -> SkillContract:
    actions = (ActionSignature("arm", "none", (("armed", 0),), (("armed", 1),), (("armed", 1),)), ActionSignature("commit", "none", (("armed", 1),), (("armed", 1), ("committed", 1)), (("committed", 1),)))
    return SkillContract(1, 1, "skill-round10-sample", "sim-host", "sim-host-digest", "sim-host-key", 3, ("armed", "committed"), (("armed", 0), ("committed", 0)), actions, (("committed", 1),), 2, 100, "nonce-round10")
