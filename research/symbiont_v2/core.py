"""HERUS Symbiont v2 research runtime.

The design deliberately separates:

* persistent identity: the HERUS lineage and verified abstract skills;
* host-local context: observations, transition models and constraints;
* authority: never created by discovery or learning.

The runtime is a deterministic research sandbox.  It does not execute generated
code, access the network, or grant physical authority.  A real HostAdapter must
sit behind an independently enforced authorization/rate-limit/emergency-stop
boundary.
"""
from __future__ import annotations

from dataclasses import dataclass, field, replace
import hashlib
import json
from typing import Protocol

SCHEMA_VERSION = 2


def _canon(value: object) -> bytes:
    return json.dumps(value, sort_keys=True, separators=(",", ":"), ensure_ascii=True).encode("utf-8")


def _digest(value: object) -> str:
    return hashlib.sha256(_canon(value)).hexdigest()


@dataclass(frozen=True)
class State:
    values: tuple[tuple[str, int], ...]

    @classmethod
    def from_dict(cls, values: dict[str, int]) -> "State":
        normalized = {str(k): int(v) for k, v in values.items()}
        return cls(tuple(sorted(normalized.items())))

    def to_dict(self) -> dict[str, int]:
        return dict(self.values)


@dataclass(frozen=True)
class PrimitiveAction:
    action_id: str
    argument: int = 0

    def canonical(self) -> dict[str, object]:
        return {"action_id": self.action_id, "argument": self.argument}


@dataclass(frozen=True)
class Observation:
    sequence: int
    state: State
    action: PrimitiveAction | None
    digest: str

    @classmethod
    def make(cls, sequence: int, state: State, action: PrimitiveAction | None) -> "Observation":
        payload = {
            "sequence": sequence,
            "state": state.to_dict(),
            "action": None if action is None else action.canonical(),
        }
        return cls(sequence, state, action, _digest(payload))

    def valid(self) -> bool:
        payload = {
            "sequence": self.sequence,
            "state": self.state.to_dict(),
            "action": None if self.action is None else self.action.canonical(),
        }
        return self.digest == _digest(payload)


@dataclass(frozen=True)
class Effect:
    delta: tuple[tuple[str, int], ...]

    @classmethod
    def from_states(cls, before: State, after: State) -> "Effect":
        b, a = before.to_dict(), after.to_dict()
        keys = sorted(set(b) | set(a))
        return cls(tuple((k, a.get(k, 0) - b.get(k, 0)) for k in keys if a.get(k, 0) != b.get(k, 0)))

    def canonical(self) -> dict[str, int]:
        return dict(self.delta)


@dataclass(frozen=True)
class Evidence:
    episode_id: str
    action: PrimitiveAction
    before: State
    after: State
    effect: Effect
    provenance: str

    @property
    def digest(self) -> str:
        return _digest({
            "episode_id": self.episode_id,
            "action": self.action.canonical(),
            "before": self.before.to_dict(),
            "after": self.after.to_dict(),
            "effect": self.effect.canonical(),
            "provenance": self.provenance,
        })


@dataclass(frozen=True)
class Goal:
    """Monotone state goal used by the first benchmark.

    A real HERUS goal layer can later replace this with the existing Semantic IR
    predicates without changing the host-learning boundary.
    """

    minimums: tuple[tuple[str, int], ...]

    @classmethod
    def from_dict(cls, values: dict[str, int]) -> "Goal":
        return cls(tuple(sorted((str(k), int(v)) for k, v in values.items())))

    def satisfied(self, state: State) -> bool:
        current = state.to_dict()
        return all(current.get(k, 0) >= v for k, v in self.minimums)


@dataclass(frozen=True)
class AbstractSkill:
    skill_id: str
    goal: Goal
    primitive_ids: tuple[str, ...]
    max_steps: int
    evidence_digests: tuple[str, ...]
    status: str = "CANDIDATE"
    source_host: str | None = None

    @property
    def digest(self) -> str:
        return _digest({
            "schema": SCHEMA_VERSION,
            "skill_id": self.skill_id,
            "goal": dict(self.goal.minimums),
            "primitive_ids": list(self.primitive_ids),
            "max_steps": self.max_steps,
            "evidence_digests": list(self.evidence_digests),
            "status": self.status,
        })


@dataclass(frozen=True)
class HostModel:
    host_id: str
    resources: tuple[str, ...]
    safe_actions: tuple[str, ...]
    state_keys: tuple[str, ...]
    epoch: int

    @property
    def digest(self) -> str:
        return _digest({
            "host_id": self.host_id,
            "resources": list(self.resources),
            "safe_actions": list(self.safe_actions),
            "state_keys": list(self.state_keys),
            "epoch": self.epoch,
        })


class HostAdapter(Protocol):
    host_id: str

    def resources(self) -> tuple[str, ...]: ...
    def safe_action_space(self) -> tuple[PrimitiveAction, ...]: ...
    def observe(self) -> Observation: ...
    def execute(self, action: PrimitiveAction) -> Observation: ...
    def reset(self) -> None: ...


@dataclass(frozen=True)
class DiscoveryBudget:
    max_probes: int = 32
    max_candidates: int = 256


@dataclass
class WorldModel:
    """Host-local empirical transition model.

    Conflicting observations are quarantined rather than overwritten.  The
    verifier therefore cannot silently inherit a contradicted transition.
    """

    transitions: dict[str, dict[tuple[tuple[str, int], ...], Effect]] = field(default_factory=dict)
    conflicts: set[tuple[str, tuple[tuple[str, int], ...]]] = field(default_factory=set)

    def learn(self, evidence: Evidence) -> None:
        action = evidence.action.action_id
        state_key = evidence.before.values
        key = (action, state_key)
        if key in self.conflicts:
            return
        bucket = self.transitions.setdefault(action, {})
        previous = bucket.get(state_key)
        if previous is None:
            bucket[state_key] = evidence.effect
        elif previous != evidence.effect:
            bucket.pop(state_key, None)
            self.conflicts.add(key)

    def predict(self, action_id: str, state: State) -> Effect | None:
        key = (action_id, state.values)
        if key in self.conflicts:
            return None
        return self.transitions.get(action_id, {}).get(state.values)


@dataclass
class PersistentMemory:
    """Host-independent lineage memory.

    Only verified skills survive a host rebind.  Host observations, world model
    and unverified candidates are host-local by construction.
    """

    herus_id: str
    verified_skills: dict[str, AbstractSkill] = field(default_factory=dict)

    def promote(self, skill: AbstractSkill) -> AbstractSkill:
        if skill.status != "VERIFIED":
            raise ValueError("only VERIFIED skills can be promoted")
        promoted = replace(skill, source_host=skill.source_host)
        self.verified_skills[promoted.skill_id] = promoted
        return promoted


@dataclass
class HostContext:
    model: HostModel
    world: WorldModel = field(default_factory=WorldModel)
    evidence: list[Evidence] = field(default_factory=list)


class SymbiontRuntime:
    """Persistent identity + disposable host context."""

    def __init__(self, herus_id: str, *, budget: DiscoveryBudget | None = None) -> None:
        if not herus_id:
            raise ValueError("herus_id required")
        self.memory = PersistentMemory(herus_id)
        self.host: HostContext | None = None
        self.budget = budget or DiscoveryBudget()
        self._sequence = 0

    @property
    def herus_id(self) -> str:
        return self.memory.herus_id

    def bind(self, host: HostAdapter) -> HostContext:
        """Rebind identity to a new body while discarding host-local context."""
        self._sequence += 1
        baseline = host.observe()
        if not baseline.valid():
            raise ValueError("invalid host observation")
        model = HostModel(
            host_id=host.host_id,
            resources=tuple(sorted(host.resources())),
            safe_actions=tuple(sorted(a.action_id for a in host.safe_action_space())),
            state_keys=tuple(sorted(baseline.state.to_dict())),
            epoch=self._sequence,
        )
        self.host = HostContext(model=model)
        return self.host

    def _require_host(self, host: HostAdapter) -> HostContext:
        if self.host is None or self.host.model.host_id != host.host_id:
            return self.bind(host)
        return self.host

    def discover(self, host: HostAdapter) -> tuple[Evidence, ...]:
        ctx = self._require_host(host)
        discovered: list[Evidence] = []
        actions = tuple(host.safe_action_space())[: self.budget.max_candidates]
        for action in actions[: self.budget.max_probes]:
            before = host.observe()
            if not before.valid():
                break
            after = host.execute(action)
            if not after.valid() or after.action != action:
                break
            evidence = Evidence(
                episode_id=f"{ctx.model.epoch}:{self._sequence}",
                action=action,
                before=before.state,
                after=after.state,
                effect=Effect.from_states(before.state, after.state),
                provenance=f"host:{host.host_id}",
            )
            self._sequence += 1
            discovered.append(evidence)
            ctx.evidence.append(evidence)
            ctx.world.learn(evidence)
            host.reset()
        return tuple(discovered)

    @staticmethod
    def _apply(state: State, effect: Effect) -> State:
        values = state.to_dict()
        for key, delta in effect.delta:
            values[key] = values.get(key, 0) + delta
        return State.from_dict(values)

    def synthesize(self, goal: Goal, *, max_steps: int = 4) -> AbstractSkill | None:
        """Search a bounded composition graph over verified observations."""
        ctx = self.host
        if ctx is None or not ctx.evidence:
            return None
        start = ctx.evidence[0].before
        queue: list[tuple[State, tuple[Evidence, ...]]] = [(start, ())]
        seen = {start.values}
        candidates = 0
        while queue and candidates < self.budget.max_candidates:
            state, path = queue.pop(0)
            candidates += 1
            if path and goal.satisfied(state):
                candidate = AbstractSkill(
                    skill_id=f"skill-{_digest([e.action.action_id for e in path])[:12]}",
                    goal=goal,
                    primitive_ids=tuple(e.action.action_id for e in path),
                    max_steps=len(path),
                    evidence_digests=tuple(e.digest for e in path),
                    source_host=ctx.model.host_id,
                )
                if self.verify(candidate, start):
                    return replace(candidate, status="VERIFIED")
            if len(path) >= max_steps:
                continue
            for evidence in ctx.evidence:
                effect = ctx.world.predict(evidence.action.action_id, state)
                if effect is None or not effect.delta:
                    continue
                nxt = self._apply(state, effect)
                if nxt.values in seen:
                    continue
                seen.add(nxt.values)
                queue.append((nxt, path + (evidence,)))
        return None

    def verify(self, skill: AbstractSkill, start: State) -> bool:
        if skill.status not in {"CANDIDATE", "VERIFIED"}:
            return False
        ctx = self.host
        if ctx is None:
            return False
        evidence_by_id = {e.action.action_id: e for e in ctx.evidence}
        state = start
        for action_id in skill.primitive_ids:
            evidence = evidence_by_id.get(action_id)
            if evidence is None:
                return False
            effect = ctx.world.predict(action_id, state)
            if effect is None:
                return False
            state = self._apply(state, effect)
        return skill.goal.satisfied(state) and len(skill.primitive_ids) <= skill.max_steps

    def promote(self, skill: AbstractSkill) -> AbstractSkill:
        if not self.verify(skill, self.host.evidence[0].before if self.host and self.host.evidence else State(())):
            raise ValueError("promotion requires fresh verification in the current host context")
        return self.memory.promote(skill)

    def transfer(self, skill: AbstractSkill, host: HostAdapter, *, max_probes: int = 64) -> bool:
        """Test whether a persistent skill can be grounded in a new host.

        v2 uses effect signatures as the bridge.  This is intentionally limited:
        it is strong enough to test host-independent abstraction in the simulator
        while making semantic grounding a visible future research problem.
        """
        if skill.status != "VERIFIED" or skill.skill_id not in self.memory.verified_skills:
            return False
        self.bind(host)
        source = [e for e in (self.host.evidence if self.host else [])]
        if not source:
            self.discover(host)
        ctx = self.host
        if ctx is None:
            return False
        candidates = tuple(host.safe_action_space())[:max_probes]
        signature_map: dict[tuple[tuple[str, int], ...], PrimitiveAction] = {}
        for action in candidates:
            before = host.observe()
            if not before.valid():
                return False
            after = host.execute(action)
            if not after.valid():
                return False
            signature = Effect.from_states(before.state, after.state).delta
            host.reset()
            if signature and signature not in signature_map:
                signature_map[signature] = action
        state = host.observe().state
        for action_id in skill.primitive_ids:
            prior = next((e for e in self.host.evidence if e.action.action_id == action_id), None)
            if prior is None:
                # The target host may use a different primitive name; use the
                # source skill's host-local evidence stored in its digest list is
                # not enough to reconstruct semantics, so abort rather than guess.
                return False
            target_action = signature_map.get(prior.effect.delta)
            if target_action is None:
                return False
            result = host.execute(target_action)
            if not result.valid():
                return False
            state = result.state
        return skill.goal.satisfied(state)

    def snapshot(self) -> dict[str, object]:
        return {
            "schema": SCHEMA_VERSION,
            "herus_id": self.herus_id,
            "host_id": None if self.host is None else self.host.model.host_id,
            "host_digest": None if self.host is None else self.host.model.digest,
            "verified_skills": sorted(self.memory.verified_skills),
            "host_evidence": 0 if self.host is None else len(self.host.evidence),
        }
