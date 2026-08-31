"""Bounded enumeration over reusable structural subterms."""
from __future__ import annotations

from dataclasses import dataclass
from fractions import Fraction
from itertools import combinations_with_replacement

from .free_reasoner import Term, normalize
from .optimizer import Example
from .structured_terms import Node, Program, StructuralBudget, append_node, seed_variable


@dataclass(frozen=True)
class StructuralSearchResult:
    candidate: Program | None
    train_error: Fraction | None
    evaluations: int
    exhausted: bool


def _error(program: Program, examples: tuple[Example, ...]) -> Fraction:
    term = normalize(program.materialize())
    total = Fraction(0)
    for example in examples:
        env, expected = example.normalized()
        total += abs(term.evaluate(env) - expected)
    return total


def _extensions(program: Program) -> tuple[Program, ...]:
    refs = range(len(program.nodes))
    candidates: list[Program] = []
    for ref in refs:
        candidates.append(append_node(program, Node.apply("neg", ref)))
    for left, right in combinations_with_replacement(refs, 2):
        candidates.append(append_node(program, Node.apply("add", left, right)))
        candidates.append(append_node(program, Node.apply("mul", left, right)))
    return tuple(candidates)


def structural_search(
    examples: tuple[Example, ...],
    budget: StructuralBudget = StructuralBudget(max_nodes=8, max_depth=8),
    max_evaluations: int = 5000,
) -> StructuralSearchResult:
    """Enumerate DAG programs without executing generated text."""
    if max_evaluations <= 0:
        raise ValueError("max_evaluations must be positive")
    frontier = [seed_variable("x", budget)]
    evaluations = 0
    best: Program | None = None
    best_error: Fraction | None = None
    seen: set[tuple[Node, ...]] = set()
    while frontier and evaluations < max_evaluations:
        program = frontier.pop(0)
        if program.nodes in seen:
            continue
        seen.add(program.nodes)
        current = _error(program, examples)
        evaluations += 1
        if best_error is None or current < best_error or (current == best_error and program.depth < best.depth):
            best, best_error = program, current
        if current == 0:
            return StructuralSearchResult(program, current, evaluations, False)
        if len(program.nodes) < budget.max_nodes:
            for extension in _extensions(program):
                try:
                    if extension.depth <= budget.max_depth:
                        frontier.append(extension)
                except Exception:
                    continue
    return StructuralSearchResult(best, best_error, evaluations, bool(frontier))
