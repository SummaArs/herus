"""Symbiotic Learning v2: bounded induction of transferable effects.

Research implementation. It learns proposals from public observations only;
execution and authority are intentionally absent. The field claim remains a
hypothesis until independent baselines and holdouts show a measurable gain.
"""
from __future__ import annotations
from dataclasses import dataclass
from hashlib import sha256
from typing import Mapping, Sequence
import math

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
    if not required:
        return not observed
    return all(actual.get(k) == v for k, v in required)

def _wilson_lower(successes: int, trials: int, z: float = 1.96) -> int:
    """Return a conservative agreement lower bound in milli-units.

    This is deliberately an evidence-stability score, not a calibrated claim
    that the action is universally correct. Small samples therefore remain
    conservative instead of receiving a perfect heuristic confidence.
    """
    if trials <= 0 or successes < 0 or successes > trials:
        return 0
    p = successes / trials
    denominator = 1 + (z * z / trials)
    centre = (p + z * z / (2 * trials)) / denominator
    half = z * math.sqrt((p * (1 - p) / trials) + (z * z / (4 * trials * trials))) / denominator
    return max(0, min(1000, int(math.floor(1000 * max(0.0, centre - half)))))

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
    target_effect: State | None = None

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
    evidence_ids: tuple[str, ...] = ()

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
    evidence_ids: tuple[str, ...] = ()
    explanation: str = ""

@dataclass(frozen=True)
class UtilityWeights:
    """Weights for the proposal objective; all penalties are non-negative."""
    risk: float = 1.0
    cost: float = 1.0
    authority: float = 1.0
    evidence: float = 1.0

@dataclass(frozen=True)
class Feedback:
    """Verified host feedback used by the bounded update rule."""
    before: State
    action: str
    after: State
    target_effect: State
    context: State = ()
    outcome: str = "positive"
    utility: float = 0.0
    risk: float = 0.0
    cost: float = 1.0
    authority_violation: float = 0.0
    evidence_deficit: float = 0.0
    provenance: str = "public"
    verifier: str = "unspecified"
    step: int = 0

@dataclass(frozen=True)
class UpdateResult:
    accepted: bool
    status: str
    reason: str
    objective: float
    version: int
    evidence_id: str = ""

@dataclass(frozen=True)
class OptimizationResult:
    weights: UtilityWeights
    objective_total: float
    evaluations: int
    status: str
    fit_count: int

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

    def objective(self, *, utility: float, risk: float, cost: float, authority_violation: float = 0.0, evidence_deficit: float = 0.0, weights: UtilityWeights = UtilityWeights()) -> float:
        """Score a feedback event under the host contract."""
        values = (risk, cost, authority_violation, evidence_deficit)
        if any(value < 0 for value in values):
            raise ValueError("objective_penalties_must_be_non_negative")
        return float(utility - weights.risk * risk - weights.cost * cost - weights.authority * authority_violation - weights.evidence * evidence_deficit)

    def optimize_weights(self, feedback: Sequence[Feedback], *, grid: Sequence[float] = (0.0, 0.5, 1.0, 2.0, 4.0)) -> OptimizationResult:
        """Select contract-penalty weights on fit feedback only.

        This is intentionally a bounded grid search: deterministic, inspectable,
        finite, and incapable of reading holdout labels or mutating learner state.
        The tie-break prefers stronger authority and evidence penalties.
        """
        if not feedback or not grid or any(value < 0 for value in grid):
            return OptimizationResult(UtilityWeights(), 0.0, 0, "NO_FIT_DATA", len(feedback))
        values = tuple(sorted(set(float(value) for value in grid)))
        best: tuple[float, tuple[float, float, float, float], UtilityWeights] | None = None
        evaluations = 0
        for risk in values:
            for cost in values:
                for authority in values:
                    for evidence in values:
                        weights = UtilityWeights(risk, cost, authority, evidence)
                        total = sum(self.objective(utility=f.utility, risk=f.risk, cost=f.cost, authority_violation=f.authority_violation, evidence_deficit=f.evidence_deficit, weights=weights) for f in feedback)
                        evaluations += 1
                        tie_break = (authority, evidence, risk, cost)
                        candidate = (total, tie_break, weights)
                        if best is None or (candidate[0], candidate[1]) > (best[0], best[1]):
                            best = candidate
        assert best is not None
        return OptimizationResult(best[2], best[0], evaluations, "OPTIMIZED", len(feedback))

    def update(self, feedback: Feedback, *, weights: UtilityWeights = UtilityWeights()) -> UpdateResult:
        """Apply one bounded, reversible feedback update; never grants authority."""
        if feedback.outcome not in {"positive", "negative"}:
            return UpdateResult(False, "REJECTED", "feedback_outcome_invalid", 0.0, self.version)
        if not feedback.action or feedback.risk < 0 or feedback.cost < 0:
            return UpdateResult(False, "REJECTED", "feedback_contract_invalid", 0.0, self.version)
        score = self.objective(utility=feedback.utility, risk=feedback.risk, cost=feedback.cost, authority_violation=feedback.authority_violation, evidence_deficit=feedback.evidence_deficit, weights=weights)
        if feedback.authority_violation > 0:
            return UpdateResult(False, "REJECTED", "authority_violation", score, self.version)
        episode = Episode(feedback.before, feedback.action, feedback.after, int(feedback.cost), int(feedback.risk), feedback.provenance, feedback.context, feedback.step, "observed" if feedback.outcome == "positive" else "negative", feedback.target_effect)
        snapshot = self.snapshot()
        if not self.observe(episode):
            self.rollback(snapshot)
            return UpdateResult(False, "REJECTED", "observation_budget_or_risk", score, self.version)
        evidence_id = _digest((episode.before, episode.action, episode.after, episode.context, episode.step, episode.target_effect, episode.outcome))
        return UpdateResult(True, "UPDATED", "feedback_accepted", score, self.version, evidence_id)

    def observe(self, episode: Episode) -> bool:
        if not episode.action or episode.cost < 0 or episode.risk < 0 or episode.risk > self.max_risk or episode.outcome not in {"observed", "negative"}:
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
            evidence_ids = tuple(_digest((e.before, e.action, e.after, e.context, e.step)) for e in episodes)
            if len(actions) != 1:
                result.append(SkillHypothesis(_digest((context, effect)), effect, "", 0, len(episodes), "ABSTAIN", "action_alias", context, 0, drift, evidence_ids))
                continue
            confidence = _wilson_lower(len(episodes), len(episodes)) if not drift else 0
            stability = confidence
            status = "ABSTAIN" if drift else "CANDIDATE"
            result.append(SkillHypothesis(_digest((context, effect)), effect, next(iter(actions)), max(0, confidence), len(episodes), status, "temporal_drift" if drift else "observable_effect", context, max(0, stability), drift, evidence_ids))
        return tuple(result)

    def propose(self, target_effect: State, candidates: Sequence[Episode], *, cost_budget: int = 4, context: State = (), current_step: int | None = 0, current_state: State | None = None) -> Proposal:
        skill_id = _digest((context, target_effect))
        if cost_budget <= 0:
            return Proposal(skill_id, None, 0, "ABSTAIN", "budget_exhausted", 0, explanation="Nenhuma proposta: o orçamento disponível é zero ou negativo.")
        matches = [e for e in candidates if e.outcome == "observed" and e.effect == target_effect and _context_matches(context, e.context) and (current_state is None or e.before == current_state) and e.cost <= cost_budget and e.risk <= self.max_risk]
        if not matches:
            return Proposal(skill_id, None, 0, "ABSTAIN", "effect_not_observed", 0, explanation="Nenhum episódio observado reproduz simultaneamente efeito, contexto, risco e custo.")
        evidence_ids = tuple(_digest((e.before, e.action, e.after, e.context, e.step)) for e in matches)
        if current_step is not None and any(abs(e.step - current_step) > self.max_age for e in matches):
            return Proposal(skill_id, None, 0, "ABSTAIN", "temporal_drift", min(e.cost for e in matches), len(matches), True, evidence_ids, "Abstenção: a evidência correspondente está fora da janela temporal permitida.")
        actions = {e.action for e in matches}
        if len(actions) != 1:
            return Proposal(skill_id, None, 0, "ABSTAIN", "ambiguous_effect", min(e.cost for e in matches), len(matches), False, evidence_ids, "Abstenção: o mesmo efeito/contexto foi observado com ações diferentes.")
        negative_actions = {e.action for e in candidates if e.outcome == "negative" and e.target_effect == target_effect and _context_matches(context, e.context) and (current_state is None or e.before == current_state)}
        if negative_actions.intersection(actions):
            return Proposal(skill_id, None, 0, "ABSTAIN", "negative_evidence", min(e.cost for e in matches), len(matches), False, evidence_ids, "Abstenção: existe contraevidência negativa para a única ação observada.")
        chosen = matches[0]
        confidence = _wilson_lower(len(matches), len(matches))
        return Proposal(skill_id, chosen.action, confidence, "PROPOSE", "unique_effect_match", chosen.cost, len(matches), False, evidence_ids, f"Proposta sustentada por {len(matches)} episódio(s) com efeito e contexto coincidentes; confiança é limite inferior de Wilson da estabilidade da evidência.")

    def export(self) -> dict[str, object]:
        return {"algorithm": "symbiotic-learning-v2", "version": self._version, "episodes": len(self._episodes), "skills": [h.__dict__ for h in self.induce()]}
