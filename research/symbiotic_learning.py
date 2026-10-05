"""Symbiotic Learning v2: bounded induction of transferable effects.

Research implementation. It learns proposals from public observations only;
execution and authority are intentionally absent. The field claim remains a
hypothesis until independent baselines and holdouts show a measurable gain.
"""
from __future__ import annotations
from dataclasses import dataclass
from hashlib import sha256
from typing import Mapping, Sequence

State = tuple[tuple[str, int], ...]

def _state(value: Mapping[str, int] | State) -> State:
    return tuple(sorted((str(k), int(v)) for k, v in dict(value).items()))

def _delta(before: State, after: State) -> State:
    left, right = dict(before), dict(after)
    return tuple(sorted((k, right.get(k, 0) - left.get(k, 0)) for k in set(left) | set(right) if right.get(k, 0) - left.get(k, 0)))

def _digest(value: object) -> str:
    return sha256(repr(value).encode()).hexdigest()

def _context_matches(required: State, observed: State) -> bool:
    actual = dict(observed)
    return all(actual.get(k) == v for k, v in required)

@dataclass(frozen=True)
class Episode:
    before: State
    action: str
    after: State
    cost: int = 1
    risk: int = 0
    provenance: str = "public"
    context: State = ()
    step: int = 0
    outcome: str = "observed"

    @classmethod
    def from_maps(cls, before: Mapping[str, int], action: str, after: Mapping[str, int], **kw: int | str | Mapping[str, int]) -> "Episode":
        if "context" in kw and isinstance(kw["context"], Mapping):
            kw["context"] = _state(kw["context"])
        return cls(_state(before), action, _state(after), **kw)  # type: ignore[arg-type]

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
    context: State = ()
    stability_milli: int = 0
    drift: bool = False

@dataclass(frozen=True)
class Proposal:
    skill_id: str
    action: str | None
    confidence_milli: int
    status: str
    reason: str
    cost: int
    evidence_count: int = 0
    drift: bool = False

class SymbioticLearner:
    """Incremental, finite, context-aware and reversible effect learner."""
    def __init__(self, *, max_observations: int = 32, max_cost: int = 32, max_risk: int = 0, max_age: int = 8) -> None:
        self.max_observations, self.max_cost, self.max_risk, self.max_age = max_observations, max_cost, max_risk, max_age
        self._episodes: list[Episode] = []
        self._version = 0

    @property
    def version(self) -> int:
        return self._version

    def snapshot(self) -> tuple[tuple[Episode, ...], int]:
        return tuple(self._episodes), self._version

    def rollback(self, snapshot: tuple[tuple[Episode, ...], int]) -> None:
        self._episodes, self._version = list(snapshot[0]), snapshot[1]

    def observe(self, episode: Episode) -> bool:
        if episode.cost < 0 or episode.risk > self.max_risk or episode.outcome not in {"observed", "negative"}:
            return False
        if len(self._episodes) >= self.max_observations or sum(e.cost for e in self._episodes) + episode.cost > self.max_cost:
            return False
        self._episodes.append(episode)
        self._version += 1
        return True

    def induce(self) -> tuple[SkillHypothesis, ...]:
        groups: dict[tuple[State, State], list[Episode]] = {}
        for episode in self._episodes:
            if episode.outcome == "observed":
                groups.setdefault((episode.context, episode.effect), []).append(episode)
        result: list[SkillHypothesis] = []
        for (context, effect), episodes in sorted(groups.items(), key=lambda item: repr(item[0])):
            actions = {e.action for e in episodes}
            steps = [e.step for e in episodes]
            drift = bool(steps and max(steps) - min(steps) > self.max_age)
            if len(actions) != 1:
                result.append(SkillHypothesis(_digest((context, effect)), effect, "", 0, len(episodes), "ABSTAIN", "action_alias", context, 0, drift))
                continue
            confidence = min(1000, 250 * len(episodes) - (300 if drift else 0))
            stability = min(1000, 500 + 250 * min(len(episodes), 2) - (500 if drift else 0))
            status = "ABSTAIN" if drift else "CANDIDATE"
            result.append(SkillHypothesis(_digest((context, effect)), effect, next(iter(actions)), max(0, confidence), len(episodes), status, "temporal_drift" if drift else "observable_effect", context, max(0, stability), drift))
        return tuple(result)

    def propose(self, target_effect: State, candidates: Sequence[Episode], *, cost_budget: int = 4, context: State = (), current_step: int = 0) -> Proposal:
        skill_id = _digest((context, target_effect))
        if cost_budget <= 0:
            return Proposal(skill_id, None, 0, "ABSTAIN", "budget_exhausted", 0)
        matches = [e for e in candidates if e.outcome == "observed" and e.effect == target_effect and _context_matches(context, e.context) and e.cost <= cost_budget and e.risk <= self.max_risk]
        if not matches:
            return Proposal(skill_id, None, 0, "ABSTAIN", "effect_not_observed", 0)
        if any(e.step and current_step and abs(e.step - current_step) > self.max_age for e in matches):
            return Proposal(skill_id, None, 0, "ABSTAIN", "temporal_drift", min(e.cost for e in matches), len(matches), True)
        actions = {e.action for e in matches}
        if len(actions) != 1:
            return Proposal(skill_id, None, 0, "ABSTAIN", "ambiguous_effect", min(e.cost for e in matches), len(matches))
        chosen = matches[0]
        return Proposal(skill_id, chosen.action, min(1000, 500 + 250 * min(len(matches), 2)), "PROPOSE", "unique_effect_match", chosen.cost, len(matches))

    def export(self) -> dict[str, object]:
        return {"algorithm": "symbiotic-learning-v2", "version": self._version, "episodes": len(self._episodes), "skills": [h.__dict__ for h in self.induce()]}
