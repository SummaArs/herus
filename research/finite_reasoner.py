"""Finite symbolic reasoner prototype for HERUS research.

This module is intentionally host-only and outside firmware/. It does not claim
open-ended language understanding. Terms are typed, colors are finite, equality
saturation is budgeted, and contradictions are stored without explosion.
"""
from __future__ import annotations

from dataclasses import dataclass, field
from enum import Enum
from itertools import product
from typing import Callable, Iterable, Sequence


class Sort(str, Enum):
    PERSON = "person"
    PLACE = "place"
    TIME = "time"
    ACTION = "action"
    STATUS = "status"
    CARD = "card"
    BOOL = "bool"


@dataclass(frozen=True)
class Symbol:
    name: str
    sort: Sort
    color: str


@dataclass(frozen=True)
class Term:
    op: str
    sort: Sort
    color: str
    args: tuple["Term", ...] = ()
    atom: str | None = None

    @staticmethod
    def atom_term(symbol: Symbol) -> "Term":
        return Term(symbol.name, symbol.sort, symbol.color, atom=symbol.name)


@dataclass(frozen=True)
class Signature:
    """Finite colored-operad signature: each operation has typed input colors."""

    operations: dict[str, tuple[str, Sort, tuple[tuple[str, Sort], ...]]] = field(default_factory=dict)

    def add(self, name: str, color: str, output: Sort, inputs: tuple[tuple[str, Sort], ...]) -> None:
        if name in self.operations:
            raise ValueError(f"duplicate operation: {name}")
        self.operations[name] = (color, output, inputs)

    def compose(self, name: str, args: tuple[Term, ...]) -> Term:
        try:
            color, output, inputs = self.operations[name]
        except KeyError as exc:
            raise ValueError(f"unknown operation: {name}") from exc
        if len(args) != len(inputs):
            raise TypeError(f"{name}: expected {len(inputs)} args, got {len(args)}")
        for i, (term, (expected_color, expected_sort)) in enumerate(zip(args, inputs)):
            if term.color != expected_color or term.sort != expected_sort:
                raise TypeError(
                    f"{name}[{i}]: expected {expected_color}/{expected_sort}, "
                    f"got {term.color}/{term.sort}"
                )
        return Term(name, output, color, args=args)


def generate_terms(
    signature: Signature,
    symbols: Sequence[Symbol],
    max_depth: int,
    max_terms: int = 512,
) -> tuple[Term, ...]:
    """Enumerate novel terms from a finite typed signature, fail-closed on budget."""
    if max_depth < 0 or max_terms < 1:
        raise ValueError("depth and budget must be positive")
    all_terms: list[Term] = [Term.atom_term(symbol) for symbol in symbols]
    seen = set(all_terms)
    if len(all_terms) > max_terms:
        raise OverflowError("term generation budget exhausted")
    by_signature: dict[tuple[str, Sort], list[Term]] = {}
    for term in all_terms:
        by_signature.setdefault((term.color, term.sort), []).append(term)
    for _depth in range(max_depth):
        additions: list[Term] = []
        for name, (color, output, inputs) in signature.operations.items():
            pools = [by_signature.get(key, []) for key in inputs]
            if any(not pool for pool in pools):
                continue
            for args in product(*pools):
                candidate = signature.compose(name, tuple(args))
                if candidate not in seen:
                    seen.add(candidate)
                    additions.append(candidate)
                    if len(seen) > max_terms:
                        raise OverflowError("term generation budget exhausted")
        for term in additions:
            by_signature.setdefault((term.color, term.sort), []).append(term)
            all_terms.append(term)
        if not additions:
            break
    return tuple(all_terms)


class EGraph:
    """Small, deterministic equality graph with explicit saturation limits."""

    def __init__(self, max_nodes: int = 512):
        self.max_nodes = max_nodes
        self.parent: dict[Term, Term] = {}
        self.rules: list[Callable[[Term], Iterable[Term]]] = []

    def add(self, term: Term) -> Term:
        pending: list[Term] = []
        queued: set[Term] = set()
        stack = [term]
        while stack:
            current = stack.pop()
            if current in self.parent or current in queued:
                continue
            queued.add(current)
            pending.append(current)
            stack.extend(current.args)
        if len(self.parent) + len(pending) > self.max_nodes:
            raise OverflowError("e-graph node budget exhausted")
        for current in pending:
            self.parent[current] = current
        return term

    def find(self, term: Term) -> Term:
        self.add(term)
        parent = self.parent[term]
        if parent != term:
            self.parent[term] = self.find(parent)
        return self.parent[term]

    def union(self, left: Term, right: Term) -> None:
        if left.sort != right.sort or left.color != right.color:
            raise TypeError("cannot equate terms with different sort or color")
        self.add(left)
        self.add(right)
        lroot, rroot = self.find(left), self.find(right)
        if lroot != rroot:
            self.parent[rroot] = lroot

    def saturate(self, fuel: int = 64) -> int:
        """Apply rules to a fixed point or until fuel/node budget is exhausted."""
        steps = 0
        while steps < fuel:
            changed = False
            snapshot = tuple(self.parent)
            for term in snapshot:
                for candidate in tuple(rule(term) for rule in self.rules):
                    for result in candidate:
                        self.add(result)
                        before = self.find(term)
                        other = self.find(result)
                        if before != other:
                            self.union(term, result)
                            changed = True
                            steps += 1
                            if steps >= fuel:
                                return steps
            if not changed:
                return steps
        return steps

    def equivalent(self, left: Term, right: Term) -> bool:
        return self.find(left) == self.find(right)


@dataclass(frozen=True)
class Evidence:
    proposition: str
    source: str
    confidence: int


class ParaconsistentKB:
    """Four-valued local facts: true, false, both, or neither."""

    def __init__(self, allowed: set[str]):
        self.allowed = allowed
        self.positive: dict[str, list[Evidence]] = {}
        self.negative: dict[str, list[Evidence]] = {}

    def add(self, evidence: Evidence, value: bool) -> None:
        if evidence.proposition not in self.allowed:
            raise ValueError("proposition outside finite vocabulary")
        target = self.positive if value else self.negative
        target.setdefault(evidence.proposition, []).append(evidence)

    def status(self, proposition: str) -> str:
        if proposition not in self.allowed:
            return "UNKNOWN_SYMBOL"
        p = proposition in self.positive
        n = proposition in self.negative
        return {(True, False): "TRUE", (False, True): "FALSE", (True, True): "BOTH", (False, False): "NEITHER"}[p, n]

    def explain(self, proposition: str) -> tuple[str, tuple[Evidence, ...], tuple[Evidence, ...]]:
        return (
            self.status(proposition),
            tuple(self.positive.get(proposition, ())),
            tuple(self.negative.get(proposition, ())),
        )


def build_context_signature() -> Signature:
    sig = Signature()
    sig.add("arrive", "context", Sort.STATUS, (("context", Sort.PERSON), ("context", Sort.PLACE)))
    sig.add("arrive_alias", "context", Sort.STATUS, (("context", Sort.PERSON), ("context", Sort.PLACE)))
    sig.add("meet", "context", Sort.ACTION, (("context", Sort.PERSON), ("context", Sort.PLACE), ("context", Sort.TIME)))
    sig.add("bind_card", "card", Sort.CARD, (("card", Sort.STATUS), ("card", Sort.TIME)))
    return sig


def demo() -> dict[str, object]:
    sig = build_context_signature()
    alice = Term.atom_term(Symbol("alice", Sort.PERSON, "context"))
    trail = Term.atom_term(Symbol("trail", Sort.PLACE, "context"))
    friday = Term.atom_term(Symbol("friday", Sort.TIME, "context"))
    arrival = sig.compose("arrive", (alice, trail))
    arrival_alias = sig.compose("arrive_alias", (alice, trail))

    graph = EGraph(max_nodes=32)
    graph.add(arrival)
    graph.add(arrival_alias)
    graph.rules.append(lambda t: (arrival,) if t == arrival_alias else ())
    graph.saturate(fuel=8)

    kb = ParaconsistentKB({"arrival_confirmed", "arrival_cancelled"})
    kb.add(Evidence("arrival_confirmed", "button", 100), True)
    kb.add(Evidence("arrival_confirmed", "radio", 55), False)

    return {
        "typed_terms": (arrival, arrival_alias),
        "arrival_equivalent_to_alias": graph.equivalent(arrival, arrival_alias),
        "contradiction_status": kb.status("arrival_confirmed"),
        "contradiction_explanation": kb.explain("arrival_confirmed"),
        "node_count": len(graph.parent),
    }


if __name__ == "__main__":
    result = demo()
    print("finite reasoner demo")
    print(f"typed terms: {len(result['typed_terms'])}")
    print(f"equivalent after bounded saturation: {result['arrival_equivalent_to_alias']}")
    print(f"paraconsistent status: {result['contradiction_status']}")
    print(f"e-graph nodes: {result['node_count']}")
