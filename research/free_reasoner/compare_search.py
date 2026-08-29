"""Compare bounded enumeration with the stochastic discrete optimizer.

The benchmark measures search efficiency, not language understanding. Every
candidate is evaluated on train and holdout examples, and formal promotion is
attempted only through the exact Prover against a declared target.
"""
from __future__ import annotations

from dataclasses import dataclass
from fractions import Fraction
import json
import time
from typing import Callable, Sequence

from .free_reasoner import Term, Prover, enumerate_terms, normalize, variables
from .optimizer import Example, OptimizationBudget, optimize_examples
from .search_methods import beam_search, mcts_search


@dataclass(frozen=True)
class Task:
    name: str
    target: Term
    train: tuple[Example, ...]
    holdout: tuple[Example, ...]


@dataclass(frozen=True)
class Row:
    task: str
    method: str
    seed: int | None
    evaluations: int
    candidate: str | None
    train_error: str | None
    holdout_error: str | None
    proved: bool
    elapsed_ms: float


def error(term: Term | None, examples: Sequence[Example]) -> Fraction | None:
    if term is None:
        return None
    poly = normalize(term)
    total = Fraction(0)
    for example in examples:
        env, expected = example.normalized()
        total += abs(poly.evaluate(env) - expected)
    return total


def prove_candidate(candidate: Term | None, target: Term) -> bool:
    if candidate is None:
        return False
    try:
        return Prover().prove(candidate, target).verified
    except Exception:
        return False


def enum_search(task: Task, max_terms: int = 768) -> tuple[Term | None, int]:
    terms = enumerate_terms(("x",), max_depth=5, max_terms=max_terms)
    best: Term | None = None
    best_error: Fraction | None = None
    for term in terms:
        current = error(term, task.train)
        if best_error is None or current < best_error:
            best, best_error = term, current
        if current == 0:
            return term, terms.index(term) + 1
    return best, len(terms)


def tasks() -> tuple[Task, ...]:
    train_x = (-2, -1, 0, 1, 2)
    holdout_x = (-7, 3, 4, 9)
    targets = (
        ("square", "x*x"),
        ("square_plus_x", "x*x+x"),
        ("square_minus_x", "x*x-x"),
    )
    from .free_reasoner import parse_simple

    out: list[Task] = []
    for name, expression in targets:
        target = parse_simple(expression)
        out.append(Task(
            name,
            target,
            tuple(Example({"x": x}, normalize(target).evaluate({"x": Fraction(x)})) for x in train_x),
            tuple(Example({"x": x}, normalize(target).evaluate({"x": Fraction(x)})) for x in holdout_x),
        ))
    target = parse_simple("x*x")
    out.append(Task(
        "underdetermined_square",
        target,
        (Example({"x": 0}, 0),),
        tuple(Example({"x": x}, normalize(target).evaluate({"x": Fraction(x)})) for x in (-2, 3, 4)),
    ))
    return tuple(out)


def run() -> list[Row]:
    rows: list[Row] = []
    for task in tasks():
        start = time.perf_counter()
        candidate, evaluations = enum_search(task)
        rows.append(Row(
            task.name, "enumeration", None, evaluations, str(candidate) if candidate else None,
            str(error(candidate, task.train)) if candidate else None,
            str(error(candidate, task.holdout)) if candidate else None,
            prove_candidate(candidate, task.target), (time.perf_counter() - start) * 1000,
        ))
        for seed in (0, 1, 2, 3, 4):
            start = time.perf_counter()
            result = optimize_examples(
                task.train,
                budget=OptimizationBudget(episodes=128, steps_per_episode=6, seed=seed),
            )
            candidate = result.candidate
            rows.append(Row(
                task.name, "stochastic_bandit", seed, result.evaluations,
                str(candidate) if candidate else None,
                str(error(candidate, task.train)) if candidate else None,
                str(error(candidate, task.holdout)) if candidate else None,
                prove_candidate(candidate, task.target), (time.perf_counter() - start) * 1000,
            ))
        start = time.perf_counter()
        result = beam_search(task.train, beam_width=24, max_evaluations=768)
        candidate = result.candidate
        rows.append(Row(
            task.name, "beam", None, result.evaluations,
            str(candidate) if candidate else None,
            str(error(candidate, task.train)) if candidate else None,
            str(error(candidate, task.holdout)) if candidate else None,
            prove_candidate(candidate, task.target), (time.perf_counter() - start) * 1000,
        ))
        for seed in (0, 1, 2, 3, 4):
            start = time.perf_counter()
            result = mcts_search(task.train, simulations=128, seed=seed)
            candidate = result.candidate
            rows.append(Row(
                task.name, "mcts", seed, result.evaluations,
                str(candidate) if candidate else None,
                str(error(candidate, task.train)) if candidate else None,
                str(error(candidate, task.holdout)) if candidate else None,
                prove_candidate(candidate, task.target), (time.perf_counter() - start) * 1000,
            ))
    return rows


def main() -> None:
    print(json.dumps([row.__dict__ for row in run()], sort_keys=True))


if __name__ == "__main__":
    main()
