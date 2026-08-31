"""Independent finite-state safety verifier for the HERUS research track."""
from __future__ import annotations

from dataclasses import dataclass
from enum import Enum
from typing import Iterable


class Verdict(str, Enum):
    VERIFIED = "VERIFIED"
    COUNTEREXAMPLE = "COUNTEREXAMPLE"
    UNKNOWN = "UNKNOWN"
    INVALID_SPEC = "INVALID_SPEC"


@dataclass(frozen=True)
class Transition:
    state: str
    input_symbol: str
    action: str
    next_state: str


@dataclass(frozen=True)
class PolicyRule:
    state: str
    input_symbol: str
    action: str


@dataclass(frozen=True)
class StateMachineSpec:
    initial_state: str
    states: frozenset[str]
    inputs: frozenset[str]
    actions: frozenset[str]
    transitions: tuple[Transition, ...]
    forbidden_states: frozenset[str]
    forbidden_actions: frozenset[str]
    max_steps: int


@dataclass(frozen=True)
class Certificate:
    verdict: Verdict
    explored: int
    path: tuple[Transition, ...]
    reason: str


def _validate_spec(spec: StateMachineSpec, policy: tuple[PolicyRule, ...]) -> str | None:
    if not spec.states or spec.initial_state not in spec.states:
        return "invalid_initial_state"
    if spec.max_steps < 0:
        return "invalid_step_bound"
    if not spec.forbidden_states.issubset(spec.states):
        return "forbidden_state_outside_spec"
    if not spec.forbidden_actions.issubset(spec.actions):
        return "forbidden_action_outside_spec"
    seen_transitions: set[tuple[str, str, str]] = set()
    for transition in spec.transitions:
        if transition.state not in spec.states or transition.next_state not in spec.states:
            return "transition_state_outside_spec"
        if transition.input_symbol not in spec.inputs or transition.action not in spec.actions:
            return "transition_symbol_outside_spec"
        key = (transition.state, transition.input_symbol, transition.action)
        if key in seen_transitions:
            return "duplicate_transition"
        seen_transitions.add(key)
    seen_policy: set[tuple[str, str]] = set()
    for rule in policy:
        if rule.state not in spec.states or rule.input_symbol not in spec.inputs:
            return "policy_key_outside_spec"
        if rule.action not in spec.actions:
            return "policy_action_outside_spec"
        key = (rule.state, rule.input_symbol)
        if key in seen_policy:
            return "duplicate_policy_rule"
        seen_policy.add(key)
    return None


def verify(spec: StateMachineSpec, policy: Iterable[PolicyRule]) -> Certificate:
    """Exhaustively verify bounded traces; unknown is never promoted."""
    rules = tuple(policy)
    invalid = _validate_spec(spec, rules)
    if invalid is not None:
        return Certificate(Verdict.INVALID_SPEC, 0, (), invalid)

    policy_map = {(rule.state, rule.input_symbol): rule.action for rule in rules}
    transition_map: dict[tuple[str, str, str], Transition] = {
        (item.state, item.input_symbol, item.action): item for item in spec.transitions
    }
    frontier: list[tuple[str, tuple[Transition, ...]]] = [(spec.initial_state, ())]
    explored = 0
    for _depth in range(spec.max_steps + 1):
        next_frontier: list[tuple[str, tuple[Transition, ...]]] = []
        for state, path in frontier:
            if state in spec.forbidden_states:
                return Certificate(Verdict.COUNTEREXAMPLE, explored, path, "forbidden_state")
            for input_symbol in sorted(spec.inputs):
                action = policy_map.get((state, input_symbol))
                if action is None:
                    return Certificate(Verdict.UNKNOWN, explored, path, "missing_policy_rule")
                if action in spec.forbidden_actions:
                    return Certificate(Verdict.COUNTEREXAMPLE, explored, path, "forbidden_action")
                transition = transition_map.get((state, input_symbol, action))
                if transition is None:
                    return Certificate(Verdict.UNKNOWN, explored, path, "unmodeled_transition")
                explored += 1
                new_path = path + (transition,)
                if transition.next_state in spec.forbidden_states:
                    return Certificate(Verdict.COUNTEREXAMPLE, explored, new_path, "forbidden_state")
                if _depth < spec.max_steps:
                    next_frontier.append((transition.next_state, new_path))
        frontier = next_frontier
        if not frontier:
            break
    return Certificate(Verdict.VERIFIED, explored, (), "all_bounded_traces_safe")
