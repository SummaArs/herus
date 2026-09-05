"""Finite causal-surprise monitor with a fail-closed safety filter.

Surprise is treated as a diagnostic signal. It is not a truth score and it is
never allowed to override a safety invariant or authorize an actuator.
"""
from __future__ import annotations

from dataclasses import dataclass
from enum import Enum


class ModelStatus(str, Enum):
    PREDICTED = "predicted"
    SURPRISE = "surprise"
    UNKNOWN = "unknown"


@dataclass(frozen=True)
class Transition:
    state: str
    action: str
    next_state: str


@dataclass(frozen=True)
class SurpriseReport:
    status: ModelStatus
    expected: str | None
    observed: str
    score: int
    reason: str


@dataclass(frozen=True)
class ActionProposal:
    action: str
    expected_next: str | None
    surprise_score: int
    admissible: bool
    reason: str


@dataclass(frozen=True)
class CausalModel:
    transitions: tuple[Transition, ...]
    safe_states: frozenset[str]
    forbidden_actions: frozenset[str] = frozenset()

    def predict(self, state: str, action: str) -> str | None:
        matches = {t.next_state for t in self.transitions if t.state == state and t.action == action}
        return next(iter(matches)) if len(matches) == 1 else None

    def observe(self, state: str, action: str, observed: str) -> SurpriseReport:
        expected = self.predict(state, action)
        if expected is None:
            return SurpriseReport(ModelStatus.UNKNOWN, None, observed, 2, "unmodeled_transition")
        if expected != observed:
            return SurpriseReport(ModelStatus.SURPRISE, expected, observed, 1, "prediction_mismatch")
        return SurpriseReport(ModelStatus.PREDICTED, expected, observed, 0, "predicted_transition")

    def propose(self, state: str, action: str) -> ActionProposal:
        expected = self.predict(state, action)
        if action in self.forbidden_actions:
            return ActionProposal(action, expected, 2, False, "forbidden_action")
        if expected is None:
            return ActionProposal(action, None, 2, False, "unmodeled_transition")
        if expected not in self.safe_states:
            return ActionProposal(action, expected, 1, False, "unsafe_predicted_state")
        return ActionProposal(action, expected, 0, True, "modeled_safe_transition")

    def choose_least_surprising_safe(self, state: str, actions: tuple[str, ...]) -> ActionProposal | None:
        """Choose only among modeled safe transitions; otherwise abstain."""
        proposals = [self.propose(state, action) for action in actions]
        admissible = [proposal for proposal in proposals if proposal.admissible]
        if not admissible:
            return None
        return min(admissible, key=lambda proposal: (proposal.surprise_score, proposal.action))


def model_from_transitions(
    transitions: tuple[Transition, ...],
    safe_states: tuple[str, ...],
    forbidden_actions: tuple[str, ...] = (),
) -> CausalModel:
    """Construct a finite model and reject contradictory causal edges."""
    seen: dict[tuple[str, str], str] = {}
    for transition in transitions:
        key = (transition.state, transition.action)
        prior = seen.get(key)
        if prior is not None and prior != transition.next_state:
            raise ValueError("contradictory_transition_model")
        seen[key] = transition.next_state
    return CausalModel(tuple(transitions), frozenset(safe_states), frozenset(forbidden_actions))
