"""Bounded bottom-up generation of well-typed symbolic terms."""
from __future__ import annotations

from dataclasses import dataclass
from itertools import product

from .core import Budget, Signature, Status, Term


@dataclass(frozen=True)
class GenerationResult:
    status: Status
    terms: frozenset[Term]
    by_depth: tuple[int, ...]
    reason: str = ""


def generate(signature: Signature, budget: Budget | None = None) -> GenerationResult:
    budget = budget or Budget()
    if not budget.valid():
        return GenerationResult(Status.BUDGET_EXCEEDED, frozenset(), (), "invalid budget")
    terms: set[Term] = set()
    depth_counts: list[int] = []
    symbols = tuple(sorted(signature.symbols().values(), key=lambda symbol: (symbol.arity, symbol.name)))
    for depth in range(budget.max_depth + 1):
        before = len(terms)
        available = tuple(sorted(terms, key=str))
        for symbol in symbols:
            if symbol.arity == 0:
                if depth != 0:
                    continue
                built = signature.atom(symbol.name, budget)
                if built.ok:
                    terms.add(built.term)
                continue
            candidates = [tuple(term for term in available if term.output == expected) for expected in symbol.inputs]
            if any(not group for group in candidates):
                continue
            for args in product(*candidates):
                if max(arg.depth for arg in args) + 1 != depth:
                    continue
                built = signature.build(symbol.name, args, budget)
                if not built.ok:
                    if built.status is Status.BUDGET_EXCEEDED:
                        return GenerationResult(Status.BUDGET_EXCEEDED, frozenset(terms), tuple(depth_counts), built.reason)
                    continue
                terms.add(built.term)
                if len(terms) > budget.max_terms:
                    return GenerationResult(Status.BUDGET_EXCEEDED, frozenset(terms), tuple(depth_counts), "term budget")
        depth_counts.append(len(terms) - before)
    return GenerationResult(Status.OK, frozenset(terms), tuple(depth_counts))
