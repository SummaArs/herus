"""HERUS Symbiont v2: a small, dependency-free research runtime.

This module turns the existing finite host/skill ideas into one executable loop:

    observe -> hypothesize -> experiment -> learn -> compose -> verify -> reuse

Important boundary: this package is a research sandbox.  A HostAdapter never
receives authority from the learner and `execute()` is only usable against the
explicitly supplied adapter.  The implementation never executes downloaded
code or writes to external systems.
"""
from __future__ import annotations

from dataclasses import dataclass, field
import hashlib
import json
from typing import Iterable, Protocol


SCHEMA_VERSION = 1


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
    """A safe, simulator-visible action candidate.

    The learner only sees an opaque action identifier and optional scalar
    argument.  Effect names are not supplied by the host.
    """

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
class ActionEffect:
    """An abstract, host-independent transition."""

    delta: tuple[tuple[str, int], ...]

    @classmethod
    def from_states(cls, before: State, after: State) -> "ActionEffect":
        keys = sorted(set(before.to_dict()) | set(after.to_dict()))
        b, a = before.to_dict(), after.to_dict()
        return cls(tuple((k, a.get(k, 0) - b.get(k, 0)) for k in keys if a.get(k, 0) != b.get(k, 0)))

    def to_dict(self) -> dict[str, int]:
        return dict(self.delta)


@dataclass(frozen=True)
class Evidence:
    episode_id: str
    action: PrimitiveAction
    before: State
    after: State
    effect: ActionEffect
    novelty: int
    provenance: str = "sandbox"

    def canonical(self) -> dict[str, object]:
        return {
            "episode_id": self.episode_id,
            "action": self.action.canonical(),
            "before": self.before.to_dict(),
            "after": self.after.to_dict(),
            "effect": self.effect.to_dict(),
            "novelty": self.novelty,
            "provenance": self.provenance,
        }

    @property
    def digest(self) -> str:
        return _digest(self.canonical())


@dataclass(frozen=True)
class AbstractSkill:
    """A verified semantic capability independent of concrete host names."""

    skill_id: str
    goal: tuple[tuple[str, int], ...]
    primitive_ids: tuple[str, ...]
    max_steps: int
    evidence_digests: tuple[str, ...]
    status: str = "CANDIDATE"

    def canonical(self) -> dict[str, object]:
        return {
            "schema": SCHEMA_VERSION,
            "skill_id": self.skill_id,
            "goal": dict(self.goal),
            "primitive_ids": list(self.primitive_ids),
            "max_steps": self.max_steps,
            "evidence_digests": list(self.evidence_digests),
            "status": self.status,
        }

    @property
    def digest(self) -> str:
        return _digest(self.canonical())


@dataclass(frozen=True)
class HostModel:
    host_id: str
    resources: tuple[str, ...]
    safe_actions: tuple[str, ...]
    state_keys: tuple[str, ...]
    revision: int = 1

    def canonical(self) -> dict[str, object]:
        return {
            "host_id": self.host_id,
            "resources": list(self.resources),
            "safe_actions": list(self.safe_actions),
            "state_keys": list(self.state_keys),
            "revision": self.revision,
        }

    @property
    def digest(self) -> str:
        return _digest(self.canonical())


class HostAdapter(Protocol):
    """Minimal research boundary for an unknown host."""

    host_id: str

    def resources(self) -> tuple[str, ...]:
        ...

    def safe_action_space(self) -> tuple[PrimitiveAction, ...]:
        ...

    def observe(self) -> Observation:
        ...

    def execute(self, action: PrimitiveAction) -> Observation:
        ...

    def reset(self) -> None:
        ...


@dataclass
class DiscoveryBudget:
    max_probes: int = 32
    max_steps: int = 64
    max_candidates: int = 256


@dataclass
class WorldModel:
    """Evidence-backed transition model.  No inferred transition becomes truth
    until repeated observations agree."""

    transitions: dict[str, dict[str, ActionEffect]] = field(default_factory=dict)

    def learn(self, evidence: Evidence) -> None:
        action = evidence.action.action_id
        state_key = repr(evidence.before.to_dict())
        bucket = self.transitions.setdefault(action, {})
        previous = bucket.get(state_key)
        if previous is None:
            bucket[state_key] = evidence.effect
            return
        # Contradictory transitions are represented by refusal to overwrite.
        if previous != evidence.effect:
            bucket[state_key] = ActionEffect(())

    def predictable(self, action: PrimitiveAction, before: State) -> ActionEffect | None:
        effect = self.transitions.get(action.action_id, {}).get(repr(before.to_dict()))
        return effect


@dataclass
class SkillLibrary:
    verified: dict[str, AbstractSkill] = field(default_factory=dict)

    def add(self, skill: AbstractSkill) -> None:
        if skill.status != "VERIFIED":
            raise ValueError("only VERIFIED skills may enter the active library")
        self.verified[skill.skill_id] = skill

    def get(self, skill_id: str) -> AbstractSkill | None:
        return self.verified.get(skill_id)


@dataclass
class SymbiontState:
    herus_id: str
    host: HostModel | None = None
    world: WorldModel = field(default_factory=WorldModel)
    evidence: list[Evidence] = field(default_factory=list)
    library: SkillLibrary = field(default_factory=SkillLibrary)
    epoch: int = 0


class SymbiontRuntime:
    """Host-independent cognitive loop for the research sandbox."""

    def __init__(self, herus_id: str, *, budget: DiscoveryBudget | None = None) -> None:
        if not herus_id:
            raise ValueError("herus_id required")
        self.state = SymbiontState(herus_id=herus_id)
        self.budget = budget or DiscoveryBudget()
        self._sequence = 0

    def rebind(self, host: HostAdapter) -> HostModel:
        """Bind the same identity to a new host and clear host-local state."""
        self.state.epoch += 1
        self.state.host = None
        self.state.world = WorldModel()
        self.state.evidence = []
        self.state.library = SkillLibrary()
        self._sequence = 0
        actions = tuple(sorted({a.action_id for a in host.safe_action_space()}))
        model = HostModel(
            host_id=host.host_id,
            resources=tuple(sorted(host.resources())),
            safe_actions=actions,
            state_keys=tuple(sorted(host.observe().state.values.__class__(()))),
            revision=self.state.epoch,
        )
        # `state_keys` above cannot be inferred from the tuple type.  Replace it
        # with the actual observed keys while keeping the operation deterministic.
        observed = host.observe()
        self.state.host = HostModel(
            host_id=host.host_id,
            resources=tuple(sorted(host.resources())),
            safe_actions=actions,
            state_keys=tuple(sorted(observed.state.to_dict())),
            revision=self.state.epoch,
        )
        if not observed.valid():
            raise ValueError("initial observation failed integrity check")
        return self.state.host

    @staticmethod
    def _novelty(before: State, after: State) -> int:
        b, a = before.to_dict(), after.to_dict()
        return sum(1 for k in set(b) | set(a) if b.get(k, 0) != a.get(k, 0))

    def discover(self, host: HostAdapter) -> tuple[Evidence, ...]:
        if self.state.host is None or self.state.host.host_id != host.host_id:
            self.rebind(host)
        baseline = host.observe()
        if not baseline.valid():
            raise ValueError("invalid baseline observation")
        discovered: list[Evidence] = []
        candidates = list(host.safe_action_space())[: self.budget.max_candidates]
        for action in candidates[: self.budget.max_probes]:
            before = host.observe()
            if not before.valid():
                break
            after = host.execute(action)
            if not after.valid() or after.action != action:
                break
            effect = ActionEffect.from_states(before.state, after.state)
            evidence = Evidence(
                episode_id=f"{self.state.epoch}:{self._sequence}",
                action=action,
                before=before.state,
                after=after.state,
                effect=effect,
                novelty=self._novelty(before.state, after.state),
            )
            self._sequence += 1
            discovered.append(evidence)
            self.state.evidence.append(evidence)
            self.state.world.learn(evidence)
            host.reset()
        return tuple(discovered)

    def _goal_apply(self, state: State, effect: ActionEffect) -> State:
        values = state.to_dict()
        for key, delta in effect.delta:
            values[key] = values.get(key, 0) + delta
        return State.from_dict(values)

    def compose_skill(self, goal: dict[str, int], *, max_steps: int = 3) -> AbstractSkill | None:
        """BFS over learned primitive transitions, then keep only a verified plan."""
        if not self.state.evidence:
            return None
        goal_state = dict(goal)
        start = self.state.evidence[0].before
        queue: list[tuple[State, tuple[Evidence, ...]]] = [(start, ())]
        seen = {repr(start.to_dict())}
        candidates = 0
        while queue and candidates < self.budget.max_candidates:
            state, path = queue.pop(0)
            candidates += 1
            if all(state.to_dict().get(k, 0) >= v for k, v in goal_state.items()):
                if not path:
                    continue
                skill = AbstractSkill(
                    skill_id=f"skill-{_digest([e.action.action_id for e in path])[:12]}",
                    goal=tuple(sorted(goal_state.items())),
                    primitive_ids=tuple(e.action.action_id for e in path),
                    max_steps=len(path),
                    evidence_digests=tuple(e.digest for e in path),
                    status="CANDIDATE",
                )
                if self.verify_skill(skill, start):
                    verified = AbstractSkill(**{**skill.__dict__, "status": "VERIFIED"})
                    self.state.library.add(verified)
                    return verified
                continue
            if len(path) >= max_steps:
                continue
            for evidence in self.state.evidence:
                effect = self.state.world.predictable(evidence.action, state)
                if effect is None or not effect.delta:
                    continue
                nxt = self._goal_apply(state, effect)
                key = repr(nxt.to_dict())
                if key in seen:
                    continue
                seen.add(key)
                queue.append((nxt, path + (evidence,)))
        return None

    def verify_skill(self, skill: AbstractSkill, start: State) -> bool:
        by_id = {e.action.action_id: e for e in self.state.evidence}
        state = start
        target = dict(skill.goal)
        for action_id in skill.primitive_ids:
            evidence = by_id.get(action_id)
            if evidence is None:
                return False
            effect = self.state.world.predictable(evidence.action, state)
            if effect is None or not effect.delta:
                return False
            state = self._goal_apply(state, effect)
        return all(state.to_dict().get(k, 0) >= v for k, v in target.items())

    def transfer(self, skill: AbstractSkill, host: HostAdapter, *, max_probes: int = 32) -> bool:
        """Transfer an abstract skill by effect signature, not action name."""
        if self.state.host is None:
            raise ValueError("runtime must have a bound source host")
        source_by_signature: dict[tuple[tuple[str, int], ...], str] = {}
        for evidence in self.state.evidence:
            source_by_signature[evidence.effect.delta] = evidence.action.action_id
        current = host.observe()
        for abstract_action in skill.primitive_ids:
            source = next((e for e in self.state.evidence if e.action.action_id == abstract_action), None)
            if source is None:
                return False
            matched: PrimitiveAction | None = None
            for candidate in host.safe_action_space()[:max_probes]:
                before = host.observe()
                after = host.execute(candidate)
                if not after.valid():
                    return False
                effect = ActionEffect.from_states(before.state, after.state)
                host.reset()
                if effect.delta == source.effect.delta:
                    matched = candidate
                    break
            if matched is None:
                return False
            result = host.execute(matched)
            if not result.valid():
                return False
            current = result
        return all(current.state.to_dict().get(k, 0) >= v for k, v in skill.goal)


def runtime_snapshot(runtime: SymbiontRuntime) -> dict[str, object]:
    """Stable audit snapshot for benchmark evidence."""
    host = runtime.state.host
    return {
        "schema": SCHEMA_VERSION,
        "herus_id": runtime.state.herus_id,
        "epoch": runtime.state.epoch,
        "host_id": None if host is None else host.host_id,
        "host_digest": None if host is None else host.digest,
        "evidence": len(runtime.state.evidence),
        "verified_skills": sorted(runtime.state.library.verified),
    }
