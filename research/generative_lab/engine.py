"""Bounded composition, local hypotheses and equality saturation.

All operations are symbolic and inert: generated terms are never executed and
no result is an HERUS command or authority decision.
"""
from __future__ import annotations

from dataclasses import dataclass
from typing import Iterable

from .core import Budget, Signature, Status, Term


@dataclass(frozen=True)
class Var:
    name: str
    output: str


Pattern = Term | Var


@dataclass(frozen=True)
class Rule:
    name: str
    lhs: Pattern
    rhs: Pattern


@dataclass(frozen=True)
class Derivation:
    rule: str
    context: str
    source: Term
    result: Term


@dataclass(frozen=True)
class KnowledgeResult:
    status: Status
    context: str
    added: bool
    reason: str = ""


class KnowledgeBase:
    """Context-indexed facts with explicit local contradiction states."""

    def __init__(self, budget: Budget | None = None, signature: Signature | None = None) -> None:
        self.budget = budget or Budget()
        self.signature = signature
        self._facts: dict[str, set[Term]] = {}
        self._negated: dict[str, set[Term]] = {}
        self._derivations: list[Derivation] = []

    def add(self, context: str, term: Term, positive: bool = True) -> KnowledgeResult:
        if not context or not self.budget.valid():
            return KnowledgeResult(Status.BUDGET_EXCEEDED, context, False, "invalid context or budget")
        if self.signature is not None:
            validation = self.signature.validate(term, self.budget)
            if not validation.ok:
                return KnowledgeResult(validation.status, context, False, validation.reason)
        target = self._facts.setdefault(context, set()) if positive else self._negated.setdefault(context, set())
        other = self._negated.setdefault(context, set()) if positive else self._facts.setdefault(context, set())
        if term in target:
            return KnowledgeResult(Status.OK, context, False, "already present")
        target.add(term)
        if len(self.all_terms()) > self.budget.max_terms:
            target.remove(term)
            return KnowledgeResult(Status.BUDGET_EXCEEDED, context, False, "term budget")
        if term in other:
            return KnowledgeResult(Status.CONFLICT, context, True, "local positive/negative contradiction")
        return KnowledgeResult(Status.OK, context, True)

    def facts(self, context: str) -> frozenset[Term]:
        return frozenset(self._facts.get(context, set()))

    def negated(self, context: str) -> frozenset[Term]:
        return frozenset(self._negated.get(context, set()))

    def all_terms(self) -> frozenset[Term]:
        return frozenset(term for group in (*self._facts.values(), *self._negated.values()) for term in group)

    def contexts(self) -> tuple[str, ...]:
        return tuple(sorted(set(self._facts) | set(self._negated)))

    def derivations(self) -> tuple[Derivation, ...]:
        return tuple(self._derivations)

    def derive(self, rules: Iterable[Rule]) -> Status:
        steps = 0
        changed = True
        rules = tuple(rules)
        while changed:
            changed = False
            for context in self.contexts():
                for source in tuple(self.facts(context)):
                    for rule in rules:
                        substitution: dict[str, Term] = {}
                        if not _match(rule.lhs, source, substitution):
                            continue
                        built = _instantiate(rule.rhs, substitution)
                        if built is None:
                            continue
                        result = self.add(context, built)
                        if result.status is Status.BUDGET_EXCEEDED:
                            return result.status
                        if result.added:
                            self._derivations.append(Derivation(rule.name, context, source, built))
                            changed = True
                        if result.status is Status.CONFLICT:
                            return Status.CONFLICT
                        steps += 1
                        if steps >= self.budget.max_steps:
                            return Status.BUDGET_EXCEEDED
        return Status.OK


def _match(pattern: Pattern, term: Term, substitution: dict[str, Term]) -> bool:
    if isinstance(pattern, Var):
        if pattern.output != term.output:
            return False
        previous = substitution.get(pattern.name)
        if previous is not None:
            return previous == term
        substitution[pattern.name] = term
        return True
    return (
        pattern.symbol == term.symbol
        and pattern.output == term.output
        and len(pattern.args) == len(term.args)
        and all(_match(left, right, substitution) for left, right in zip(pattern.args, term.args))
    )


def _instantiate(pattern: Pattern, substitution: dict[str, Term]) -> Term | None:
    if isinstance(pattern, Var):
        return substitution.get(pattern.name)
    args: list[Term] = []
    for arg in pattern.args:
        built = _instantiate(arg, substitution)
        if built is None:
            return None
        args.append(built)
    return Term(pattern.symbol, pattern.output, tuple(args))


@dataclass(frozen=True)
class SaturationResult:
    status: Status
    terms: frozenset[Term]
    classes: tuple[frozenset[Term], ...]
    steps: int
    reason: str = ""

    def canonical(self) -> tuple[Term, ...]:
        return tuple(sorted(self.terms, key=lambda term: (term.nodes, term.depth, str(term))))


class EqualitySaturation:
    """Small deterministic e-graph-like closure under declared rewrites."""

    def __init__(self, signature: Signature, budget: Budget | None = None) -> None:
        self.signature = signature
        self.budget = budget or Budget()

    def run(self, seeds: Iterable[Term], rules: Iterable[Rule]) -> SaturationResult:
        if not self.budget.valid():
            return SaturationResult(Status.BUDGET_EXCEEDED, frozenset(), (), 0, "invalid budget")
        terms = set(seeds)
        if len(terms) > self.budget.max_terms:
            return SaturationResult(Status.BUDGET_EXCEEDED, frozenset(), (), 0, "seed term budget")
        for seed in terms:
            validation = self.signature.validate(seed, self.budget)
            if not validation.ok:
                return SaturationResult(validation.status, frozenset(), (), 0, validation.reason)
        edges: list[tuple[Term, Term]] = []
        steps = 0
        frontier = list(sorted(terms, key=str))
        rules = tuple(rules)
        while frontier:
            current = frontier.pop(0)
            for rule in rules:
                for rewritten in _rewrite_all(current, rule):
                    if rewritten.depth > self.budget.max_depth or rewritten.nodes > self.budget.max_nodes:
                        continue
                    validation = self.signature.validate(rewritten, self.budget)
                    if not validation.ok:
                        continue
                    steps += 1
                    if steps > self.budget.max_steps:
                        return self._result(Status.BUDGET_EXCEEDED, terms, edges, steps, "step budget")
                    edges.append((current, rewritten))
                    if rewritten not in terms:
                        terms.add(rewritten)
                        if len(terms) > self.budget.max_terms:
                            return self._result(Status.BUDGET_EXCEEDED, terms, edges, steps, "term budget")
                        frontier.append(rewritten)
        return self._result(Status.OK, terms, edges, steps)

    def _result(self, status: Status, terms: set[Term], edges: list[tuple[Term, Term]], steps: int, reason: str = "") -> SaturationResult:
        parent = {term: term for term in terms}

        def find(term: Term) -> Term:
            while parent[term] != term:
                parent[term] = parent[parent[term]]
                term = parent[term]
            return term

        def union(left: Term, right: Term) -> None:
            left_root, right_root = find(left), find(right)
            if left_root != right_root:
                parent[right_root] = left_root

        for left, right in edges:
            union(left, right)
        groups: dict[Term, set[Term]] = {}
        for term in terms:
            groups.setdefault(find(term), set()).add(term)
        classes = tuple(frozenset(group) for group in sorted(groups.values(), key=lambda group: min(map(str, group))))
        return SaturationResult(status, frozenset(terms), classes, steps, reason)


def _rewrite_all(term: Term, rule: Rule) -> tuple[Term, ...]:
    results: list[Term] = []
    substitution: dict[str, Term] = {}
    if _match(rule.lhs, term, substitution):
        replaced = _instantiate(rule.rhs, substitution)
        if replaced is not None:
            results.append(replaced)
    for index, child in enumerate(term.args):
        for rewritten_child in _rewrite_all(child, rule):
            args = list(term.args)
            args[index] = rewritten_child
            results.append(Term(term.symbol, term.output, tuple(args)))
    return tuple(dict.fromkeys(results))
