"""Bounded stochastic optimization over the exact symbolic term space.

This module is deliberately not a language model and not an authority layer.
It proposes terms using a small discrete policy, scores them only against an
explicit finite example set, and returns an untrusted candidate. A caller must
run an independent exact proof before promotion.
"""
from __future__ import annotations

from dataclasses import dataclass
from fractions import Fraction
import random
from typing import Mapping, Sequence

from .free_reasoner import C, Term, V, variables


class OptimizationError(Exception):
    """Base error for bounded optimizer misuse."""


class InvalidExamples(OptimizationError):
    """Raised when examples are malformed or use unknown variables."""


@dataclass(frozen=True)
class Example:
    environment: Mapping[str, Fraction | int]
    expected: Fraction | int

    def normalized(self) -> tuple[dict[str, Fraction], Fraction]:
        env = {name: Fraction(value) for name, value in self.environment.items()}
        return env, Fraction(self.expected)


@dataclass(frozen=True)
class OptimizationBudget:
    episodes: int = 512
    steps_per_episode: int = 6
    max_size: int = 13
    epsilon: float = 0.18
    learning_rate: float = 0.20
    complexity_penalty: float = 0.002
    seed: int = 0

    def validate(self) -> None:
        if self.episodes <= 0 or self.steps_per_episode <= 0:
            raise ValueError("episodes and steps_per_episode must be positive")
        if self.max_size < 1:
            raise ValueError("max_size must be positive")
        if not 0.0 <= self.epsilon <= 1.0:
            raise ValueError("epsilon must be in [0, 1]")
        if not 0.0 < self.learning_rate <= 1.0:
            raise ValueError("learning_rate must be in (0, 1]")
        if self.complexity_penalty < 0.0:
            raise ValueError("complexity_penalty must be non-negative")


@dataclass(frozen=True)
class OptimizationResult:
    candidate: Term | None
    score: float
    exact_error: Fraction | None
    episodes: int
    evaluations: int
    policy_values: tuple[tuple[str, float], ...]
    exhausted: bool

    @property
    def found_zero_error(self) -> bool:
        return self.candidate is not None and self.exact_error == 0


def _validate_examples(
    examples: Sequence[Example], variables_set: Sequence[str]
) -> tuple[tuple[dict[str, Fraction], Fraction], ...]:
    if not examples:
        raise InvalidExamples("at least one example is required")
    names = set(variables_set)
    if not names or any(not name or name.startswith("#") for name in names):
        raise InvalidExamples("variables must be non-empty symbolic names")
    normalized: list[tuple[dict[str, Fraction], Fraction]] = []
    for example in examples:
        env, expected = example.normalized()
        if set(env) != names:
            raise InvalidExamples("every example must define exactly the variable set")
        normalized.append((env, expected))
    return tuple(normalized)


def _evaluate(term: Term, examples: Sequence[tuple[dict[str, Fraction], Fraction]]) -> Fraction:
    from .free_reasoner import normalize

    poly = normalize(term)
    return sum(
        abs(poly.evaluate(env) - expected)
        for env, expected in examples
    )


def _actions(variables_set: Sequence[str]) -> tuple[str, ...]:
    # The action vocabulary is finite and fixed by the task, never by input text.
    return tuple([*(f"add:{name}" for name in variables_set),
                  *(f"mul:{name}" for name in variables_set), "neg"])


def _apply(term: Term, action: str) -> Term:
    op, _, operand = action.partition(":")
    if op == "neg":
        from .free_reasoner import Neg

        return Neg(term)
    other = V(operand)
    from .free_reasoner import Add, Mul

    return Add(term, other) if op == "add" else Mul(term, other)


def _choose_action(
    rng: random.Random,
    actions: Sequence[str],
    values: dict[str, float],
    epsilon: float,
) -> str:
    if rng.random() < epsilon:
        return actions[rng.randrange(len(actions))]
    best = max(values[action] for action in actions)
    choices = [action for action in actions if values[action] == best]
    return choices[rng.randrange(len(choices))]


def optimize_examples(
    examples: Sequence[Example],
    variables_set: Sequence[str] = ("x",),
    budget: OptimizationBudget | None = None,
) -> OptimizationResult:
    """Search for a term fitting finite examples using a discrete bandit policy.

    The result is an untrusted candidate. A zero example error is not a proof;
    the caller must compare the candidate with a target using ``Prover`` or the
    exact kernel before accepting it.
    """
    b = budget or OptimizationBudget()
    b.validate()
    variables_tuple = tuple(dict.fromkeys(variables_set))
    data = _validate_examples(examples, variables_tuple)
    actions = _actions(variables_tuple)
    values = {action: 0.0 for action in actions}
    rng = random.Random(b.seed)
    best_term: Term | None = None
    best_error: Fraction | None = None
    best_score = float("-inf")
    evaluations = 0
    zero_error = False

    for episode in range(1, b.episodes + 1):
        term = V(variables_tuple[0])
        chosen: list[str] = []
        for _ in range(b.steps_per_episode):
            action = _choose_action(rng, actions, values, b.epsilon)
            candidate = _apply(term, action)
            if candidate.size > b.max_size:
                continue
            term = candidate
            chosen.append(action)
            error = _evaluate(term, data)
            evaluations += 1
            score = -float(error) - b.complexity_penalty * term.size
            if best_error is None or error < best_error or (
                error == best_error and score > best_score
            ):
                best_term, best_error, best_score = term, error, score
            if error == 0:
                zero_error = True
                break
        reward = best_score if best_term is not None else -1.0
        for action in chosen:
            values[action] += b.learning_rate * (reward - values[action])
        if zero_error:
            break

    return OptimizationResult(
        candidate=best_term,
        score=best_score,
        exact_error=best_error,
        episodes=episode,
        evaluations=evaluations,
        policy_values=tuple((action, values[action]) for action in actions),
        exhausted=not zero_error,
    )


def candidate_variables(candidate: Term | None) -> frozenset[str]:
    """Return variables for diagnostics without granting any authority."""
    return variables(candidate) if candidate is not None else frozenset()
