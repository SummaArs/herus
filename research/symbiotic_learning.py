"""Symbiotic Learning v1: bounded induction of transferable effects.

This is a research algorithm, not a claim that a new ML field is established.
It learns abstract effect prototypes from public before/after observations and
returns proposals only. Authority and execution deliberately do not exist here.
"""
from __future__ import annotations
from dataclasses import dataclass
from hashlib import sha256
from typing import Mapping, Sequence

State = tuple[tuple[str, int], ...]

def _state(value: Mapping[str, int]) -> State:
    return tuple(sorted((str(k), int(v)) for k, v in value.items()))

def _delta(before: State, after: State) -> State:
    left, right = dict(before), dict(after)
    keys = set(left) | set(right)
    return tuple(sorted((k, right.get(k, 0) - left.get(k, 0)) for k in keys if right.get(k, 0) != left.get(k, 0)))

def _digest(value: object) -> str:
    return sha256(repr(value).encode()).hexdigest()

@dataclass(frozen=True)
class Episode:
    before: State
    action: str
    after: State
    cost: int = 1
    risk: int = 0
    provenance: str = "public"

    @classmethod
    def from_maps(cls, before: Mapping[str, int], action: str, after: Mapping[str, int], **kw: int | str) -> "Episode":
        return cls(_state(before), action, _state(after), **kw)

    @property
    def effect(self) -> State:
        return _delta(self.before, self.after)

@dataclass(frozen=True)
class SkillHypothesis:
    skill_id: str
    effect: State
    action: str
    confidence_milli: int
    observations: int
    status: str
    reason: str

@dataclass(frozen=True)
class Proposal:
    skill_id: str
    action: str | None
    confidence_milli: int
    status: str
    reason: str
    cost: int

class SymbioticLearner:
    """Incremental, finite and reversible learner over observable effects."""
    def __init__(self, *, max_observations: int = 32, max_cost: int = 32, max_risk: int = 0) -> None:
        self.max_observations = max_observations
        self.max_cost = max_cost
        self.max_risk = max_risk
        self._episodes: list[Episode] = []
        self._version = 0

    @property
    def version(self) -> int:
        return self._version

    def snapshot(self) -> tuple[tuple[Episode, ...], int]:
        return tuple(self._episodes), self._version

    def rollback(self, snapshot: tuple[tuple[Episode, ...], int]) -> None:
        self._episodes = list(snapshot[0])
        self._version = snapshot[1]

    def observe(self, episode: Episode) -> bool:
        if episode.cost < 0 or episode.risk > self.max_risk:
            return False
        if len(self._episodes) >= self.max_observations or sum(e.cost for e in self._episodes) + episode.cost > self.max_cost:
            return False
        self._episodes.append(episode)
        self._version += 1
        return True

    def induce(self) -> tuple[SkillHypothesis, ...]:
        groups: dict[State, list[Episode]] = {}
        for episode in self._episodes:
            groups.setdefault(episode.effect, []).append(episode)
        result: list[SkillHypothesis] = []
        for effect, episodes in sorted(groups.items(), key=lambda item: repr(item[0])):
            actions = {e.action for e in episodes}
            if len(actions) != 1:
                result.append(SkillHypothesis(_digest(effect), effect, "", 0, len(episodes), "ABSTAIN", "action_alias"))
                continue
            action = next(iter(actions))
            confidence = min(1000, 250 * len(episodes))
            result.append(SkillHypothesis(_digest(effect), effect, action, confidence, len(episodes), "CANDIDATE", "observable_effect"))
        return tuple(result)

    def propose(self, target_effect: State, candidates: Sequence[Episode], *, cost_budget: int = 4) -> Proposal:
        if cost_budget <= 0:
            return Proposal(_digest(target_effect), None, 0, "ABSTAIN", "budget_exhausted", 0)
        matches = [e for e in candidates if e.effect == target_effect and e.cost <= cost_budget and e.risk <= self.max_risk]
        if not matches:
            return Proposal(_digest(target_effect), None, 0, "ABSTAIN", "effect_not_observed", 0)
        actions = {e.action for e in matches}
        if len(actions) != 1:
            return Proposal(_digest(target_effect), None, 0, "ABSTAIN", "ambiguous_effect", min(e.cost for e in matches))
        chosen = matches[0]
        return Proposal(_digest(target_effect), chosen.action, min(1000, 500 + 250 * min(len(matches), 2)), "PROPOSE", "unique_effect_match", chosen.cost)

    def export(self) -> dict[str, object]:
        return {"algorithm": "symbiotic-learning-v1", "version": self._version, "episodes": len(self._episodes), "skills": [h.__dict__ for h in self.induce()]}
