"""Bounded discrete search baselines for the symbolic laboratory."""
from __future__ import annotations

from dataclasses import dataclass
from fractions import Fraction
import math
import random
from typing import Sequence

from .free_reasoner import Term, V, normalize
from .optimizer import Example, _actions, _apply


@dataclass(frozen=True)
class SearchResult:
    candidate: Term | None
    train_error: Fraction | None
    evaluations: int
    exhausted: bool


def _error(term: Term, examples: Sequence[Example]) -> Fraction:
    poly = normalize(term)
    total = Fraction(0)
    for example in examples:
        env, expected = example.normalized()
        total += abs(poly.evaluate(env) - expected)
    return total


def beam_search(
    examples: Sequence[Example],
    variables_set: Sequence[str] = ("x",),
    max_depth: int = 6,
    beam_width: int = 16,
    max_evaluations: int = 768,
) -> SearchResult:
    """Keep the best bounded frontier by exact train error and size."""
    if max_depth <= 0 or beam_width <= 0 or max_evaluations <= 0:
        raise ValueError("search limits must be positive")
    actions = _actions(tuple(dict.fromkeys(variables_set)))
    frontier = (V(tuple(dict.fromkeys(variables_set))[0]),)
    best: Term | None = None
    best_error: Fraction | None = None
    evaluations = 0
    for _ in range(max_depth):
        next_terms: set[Term] = set()
        for term in frontier:
            for action in actions:
                candidate = _apply(term, action)
                if candidate.size > max_depth * 2 + 1 or candidate in next_terms:
                    continue
                next_terms.add(candidate)
        ranked: list[tuple[Fraction, int, str, Term]] = []
        for term in sorted(next_terms, key=str):
            if evaluations >= max_evaluations:
                return SearchResult(best, best_error, evaluations, True)
            current = _error(term, examples)
            evaluations += 1
            ranked.append((current, term.size, str(term), term))
            if best_error is None or (current, term.size, str(term)) < (best_error, best.size if best else 10**9, str(best) if best else ""):
                best, best_error = term, current
            if current == 0:
                return SearchResult(term, current, evaluations, False)
        frontier = tuple(item[3] for item in sorted(ranked)[:beam_width])
        if not frontier:
            break
    return SearchResult(best, best_error, evaluations, evaluations >= max_evaluations)


def mcts_search(
    examples: Sequence[Example],
    variables_set: Sequence[str] = ("x",),
    simulations: int = 768,
    max_depth: int = 6,
    seed: int = 0,
) -> SearchResult:
    """A small UCT-like tree policy with exact rollout scores.

    This is an intentionally modest research baseline, not a general planner.
    It returns an untrusted candidate and never calls a side effect.
    """
    if simulations <= 0 or max_depth <= 0:
        raise ValueError("simulation limits must be positive")
    actions = _actions(tuple(dict.fromkeys(variables_set)))
    rng = random.Random(seed)
    values: dict[tuple[Term, str], tuple[float, int]] = {}
    root = V(tuple(dict.fromkeys(variables_set))[0])
    best: Term | None = None
    best_error: Fraction | None = None
    evaluations = 0

    for _ in range(simulations):
        term = root
        path: list[tuple[Term, str]] = []
        for _depth in range(max_depth):
            unvisited = [action for action in actions if (term, action) not in values]
            if unvisited:
                action = unvisited[rng.randrange(len(unvisited))]
            else:
                total = sum(count for _, count in values.values()) + 1
                def uct(action: str) -> float:
                    value, count = values[(term, action)]
                    return value / count + math.sqrt(2.0 * math.log(total) / count)
                action = max(actions, key=uct)
            path.append((term, action))
            term = _apply(term, action)
            current = _error(term, examples)
            evaluations += 1
            if best_error is None or current < best_error or (current == best_error and term.size < best.size):
                best, best_error = term, current
            if current == 0:
                reward = 1.0 / term.size
                break
            reward = -float(current) - 0.001 * term.size
        for state_action in path:
            old_value, count = values.get(state_action, (0.0, 0))
            values[state_action] = (old_value + reward, count + 1)
    return SearchResult(best, best_error, evaluations, best_error != 0)
