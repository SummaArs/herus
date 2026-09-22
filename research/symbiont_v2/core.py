"""HERUS Symbiont v2 research runtime.

Persistent identity is separated from disposable host context. Verified skills
carry an abstract effect contract so they can be grounded on another host.
The package is a deterministic sandbox: no generated code, networking, or
implicit physical authority is ever executed.
"""
from __future__ import annotations

from dataclasses import dataclass, field, replace
import hashlib
import json
from typing import Protocol

SCHEMA_VERSION = 3


def _canon(value: object) -> bytes:
    return json.dumps(value, sort_keys=True, separators=(",", ":"), ensure_ascii=True).encode("utf-8")


def _digest(value: object) -> str:
    return hashlib.sha256(_canon(value)).hexdigest()


@dataclass(frozen=True)
class State:
    values: tuple[tuple[str, int], ...]

    @classmethod
    def from_dict(cls, values: dict[str, int]) -> "State":
        return cls(tuple(sorted((str(k), int(v)) for k, v in values.items())))

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
        return cls(sequence, state, action, _digest({
            "sequence": sequence,
            "state": state.to_dict(),
            "action": None if action is None else action.canonical(),
        }))

    def valid(self) -> bool:
        return self.digest == _digest({
            "sequence": self.sequence,
            "state": self.state.to_dict(),
            "action": None if self.action is None else self.action.canonical(),
        })


@dataclass(frozen=True)
class Effect:
    delta: tuple[tuple[str, int], ...]

    @classmethod
    def from_states(cls, before: State, after: State) -> "Effect":
        b, a = before.to_dict(), after.to_dict()
        keys = sorted(set(b) | set(a))
        return cls(tuple((k, a.get(k, 0) - b.get(k, 0)) for k in keys if a.get(k, 0) != b.get(k, 0)))


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
            "effect": dict(self.effect.delta),
            "provenance": self.provenance,
        })


@dataclass(frozen=True)
class Goal:
    minimums: tuple[tuple[str, int], ...]

    @classmethod
    def from_dict(cls, values: dict[str, int]) -> "Goal":
        return cls(tuple(sorted((str(k), int(v)) for k, v in values.items())))

    def satisfied(self, state: State) -> bool:
        current = state.to_dict()
        return all(current.get(k, 0) >= v for k, v in self.minimums)


@dataclass(frozen=True)
class AbstractSkill:
    """Verified capability contract independent of concrete action identifiers."""

    skill_id: str
    goal: Goal
    primitive_ids: tuple[str, ...]
    effects: tuple[Effect, ...]
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
            "effects": [dict(effect.delta) for effect in self.effects],
            "max_steps": self.max_steps,
            "evidence_digests": list(self.evidence_digests),
            "status": self.status,
        })


@dataclass(frozen=True)
class TransferProposal:
    """A host-grounded plan; constructing it never executes an action."""

    skill_id: str
    host_id: str
    actions: tuple[PrimitiveAction, ...]
    expected_final: State
    evidence_digests: tuple[str, ...]


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
    transitions: dict[str, dict[tuple[tuple[str, int], ...], Effect]] = field(default_factory=dict)
    conflicts: set[tuple[str, tuple[tuple[str, int], ...]]] = field(default_factory=set)

    def learn(self, evidence: Evidence) -> None:
        key = (evidence.action.action_id, evidence.before.values)
        if key in self.conflicts:
            return
        bucket = self.transitions.setdefault(evidence.action.action_id, {})
        previous = bucket.get(evidence.before.values)
        if previous is None:
            bucket[evidence.before.values] = evidence.effect
        elif previous != evidence.effect:
            bucket.pop(evidence.before.values, None)
            self.conflicts.add(key)

    def predict(self, action_id: str, state: State) -> Effect | None:
        key = (action_id, state.values)
        if key in self.conflicts:
            return None
        return self.transitions.get(action_id, {}).get(state.values)


@dataclass
class PersistentMemory:
    herus_id: str
    verified_skills: dict[str, AbstractSkill] = field(default_factory=dict)

    def promote(self, skill: AbstractSkill) -> AbstractSkill:
        if skill.status != "VERIFIED":
            raise ValueError("only VERIFIED skills can be promoted")
        self.verified_skills[skill.skill_id] = skill
        return skill


@dataclass
class HostContext:
    model: HostModel
    world: WorldModel = field(default_factory=WorldModel)
    evidence: list[Evidence] = field(default_factory=list)


class SymbiontRuntime:
    """A persistent identity moving through disposable host contexts."""

    def __init__(self, herus_id: str, *, budget: DiscoveryBudget | None = None) -> None:
        if not herus_id:
            raise ValueError("herus_id required")
        self.memory = PersistentMemory(herus_id)
        self.host: HostContext | None = None
        self.budget = budget or DiscoveryBudget()
        self._epoch = 0
        self._sequence = 0

    @property
    def herus_id(self) -> str:
        """Return the identity that survives host rebinding."""
        return self.memory.herus_id

    def bind(self, host: HostAdapter) -> HostContext:
        self._epoch += 1
        self._sequence = 0
        baseline = host.observe()
        if not baseline.valid():
            raise ValueError("invalid host observation")
        self.host = HostContext(HostModel(
            host_id=host.host_id,
            resources=tuple(sorted(host.resources())),
            safe_actions=tuple(sorted(a.action_id for a in host.safe_action_space())),
            state_keys=tuple(sorted(baseline.state.to_dict())),
            epoch=self._epoch,
        ))
        return self.host

    def discover(self, host: HostAdapter) -> tuple[Evidence, ...]:
        # Every discovery call starts a fresh public observation session. A
        # reused host_id is not evidence that the underlying host is the same;
        # retaining the old model here would turn an identifier into trust.
        self.bind(host)
        assert self.host is not None
        out: list[Evidence] = []
        for action in tuple(host.safe_action_space())[: self.budget.max_probes]:
            before = host.observe()
            if not before.valid():
                break
            after = host.execute(action)
            if not after.valid() or after.action != action:
                break
            effect = Effect.from_states(before.state, after.state)
            evidence = Evidence(
                episode_id=f"{self._epoch}:{self._sequence}",
                action=action,
                before=before.state,
                after=after.state,
                effect=effect,
                provenance=f"host:{host.host_id}",
            )
            self._sequence += 1
            self.host.evidence.append(evidence)
            self.host.world.learn(evidence)
            out.append(evidence)
            host.reset()
        return tuple(out)

    @staticmethod
    def _apply(state: State, effect: Effect) -> State:
        values = state.to_dict()
        for key, delta in effect.delta:
            values[key] = values.get(key, 0) + delta
        return State.from_dict(values)

    def synthesize(self, goal: Goal, *, max_steps: int = 4) -> AbstractSkill | None:
        ctx = self.host
        if ctx is None or not ctx.evidence:
            return None
        start = ctx.evidence[0].before
        queue: list[tuple[State, tuple[Evidence, ...]]] = [(start, ())]
        seen = {start.values}
        inspected = 0
        by_action: dict[str, Evidence] = {e.action.action_id: e for e in ctx.evidence}
        while queue and inspected < self.budget.max_candidates:
            state, path = queue.pop(0)
            inspected += 1
            if path and goal.satisfied(state):
                candidate = AbstractSkill(
                    skill_id=f"skill-{_digest([e.action.action_id for e in path])[:12]}",
                    goal=goal,
                    primitive_ids=tuple(e.action.action_id for e in path),
                    effects=tuple(e.effect for e in path),
                    max_steps=len(path),
                    evidence_digests=tuple(e.digest for e in path),
                    source_host=ctx.model.host_id,
                )
                if self.verify(candidate, start):
                    return replace(candidate, status="VERIFIED")
            if len(path) >= max_steps:
                continue
            for action_id in sorted(by_action):
                effect = ctx.world.predict(action_id, state)
                if effect is None or not effect.delta:
                    continue
                nxt = self._apply(state, effect)
                if nxt.values not in seen:
                    seen.add(nxt.values)
                    queue.append((nxt, path + (by_action[action_id],)))
        return None

    def verify(self, skill: AbstractSkill, start: State) -> bool:
        ctx = self.host
        if ctx is None or len(skill.primitive_ids) != len(skill.effects):
            return False
        state = start
        for action_id, expected_effect in zip(skill.primitive_ids, skill.effects):
            observed = ctx.world.predict(action_id, state)
            if observed is None or observed != expected_effect:
                return False
            state = self._apply(state, observed)
        return skill.goal.satisfied(state) and len(skill.primitive_ids) <= skill.max_steps

    def promote(self, skill: AbstractSkill) -> AbstractSkill:
        if skill.status != "VERIFIED" or self.host is None:
            raise ValueError("promotion requires a verified skill in an active host context")
        start = self.host.evidence[0].before if self.host.evidence else State(())
        if not self.verify(skill, start):
            raise ValueError("fresh verification failed")
        return self.memory.promote(skill)

    def transfer(self, skill_id: str, host: HostAdapter, *, max_probes: int = 64) -> bool:
        """Return whether a transfer proposal can be constructed.

        Compatibility wrapper retained for the stage-one API. It deliberately
        does not execute the proposal; callers needing its contents should use
        :meth:`propose_transfer`.
        """
        return self.propose_transfer(skill_id, host, max_probes=max_probes) is not None

    def propose_transfer(
        self, skill_id: str, host: HostAdapter, *, max_probes: int = 64
    ) -> TransferProposal | None:
        """Ground a persistent skill using only public observations.

        The returned plan is not an authorization and is never sent to the
        host's executor. Ambiguous effect matches are rejected instead of
        selecting an arbitrary action.
        """
        skill = self.memory.verified_skills.get(skill_id)
        if skill is None or skill.status != "VERIFIED":
            return None
        self.discover(host)
        assert self.host is not None
        candidates = self.host.evidence[:max_probes]
        if not candidates:
            return None
        mapping: dict[tuple[tuple[str, int], ...], list[Evidence]] = {}
        for evidence in candidates:
            mapping.setdefault(evidence.effect.delta, []).append(evidence)
        if len(mapping) < len(set(effect.delta for effect in skill.effects)):
            return None
        state = host.observe().state
        actions: list[PrimitiveAction] = []
        evidence_digests: list[str] = []
        for expected in skill.effects:
            matches = mapping.get(expected.delta, [])
            if len(matches) != 1:
                return None
            evidence = matches[0]
            actions.append(evidence.action)
            evidence_digests.append(evidence.digest)
            state = self._apply(state, expected)
        if not skill.goal.satisfied(state):
            return None
        return TransferProposal(
            skill_id=skill.skill_id,
            host_id=host.host_id,
            actions=tuple(actions),
            expected_final=state,
            evidence_digests=tuple(evidence_digests),
        )

    def snapshot(self) -> dict[str, object]:
        return {
            "schema": SCHEMA_VERSION,
            "herus_id": self.memory.herus_id,
            "host_id": None if self.host is None else self.host.model.host_id,
            "host_digest": None if self.host is None else self.host.model.digest,
            "verified_skills": sorted(self.memory.verified_skills),
            "host_evidence": 0 if self.host is None else len(self.host.evidence),
        }
