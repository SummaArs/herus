"""Frontier building blocks for HERUS Symbiont v2.

These primitives move beyond exact transition matching without introducing a
large ML dependency. They are deliberately small enough to benchmark and later
replace with learned models.
"""
from __future__ import annotations

from dataclasses import dataclass, field
from math import log
from typing import Iterable


@dataclass(frozen=True)
class Interval:
    low: float
    high: float

    def __post_init__(self) -> None:
        if self.low > self.high:
            raise ValueError("interval low must not exceed high")

    @property
    def width(self) -> float:
        return self.high - self.low

    def contains(self, value: float) -> bool:
        return self.low <= value <= self.high


@dataclass
class ScalarBelief:
    """Online uncertainty estimate for a state-transition scalar."""

    count: int = 0
    mean: float = 0.0
    m2: float = 0.0

    def update(self, value: float) -> None:
        self.count += 1
        delta = value - self.mean
        self.mean += delta / self.count
        delta2 = value - self.mean
        self.m2 += delta * delta2

    @property
    def variance(self) -> float:
        return self.m2 / (self.count - 1) if self.count > 1 else float("inf")

    @property
    def interval95(self) -> Interval:
        if self.count == 0:
            return Interval(float("-inf"), float("inf"))
        if self.count == 1:
            return Interval(self.mean, self.mean)
        # Normal approximation is used only as a compact research baseline.
        radius = 1.96 * (self.variance ** 0.5) / (self.count ** 0.5)
        return Interval(self.mean - radius, self.mean + radius)


@dataclass(frozen=True)
class SemanticEffect:
    predicate: str
    arguments: tuple[str, ...]
    delta: tuple[tuple[str, Interval], ...]


@dataclass(frozen=True)
class ProbeCandidate:
    identifier: str
    expected_information: float
    cost: float
    risk: float = 0.0

    def score(self) -> float:
        if self.cost <= 0:
            return float("-inf")
        return (self.expected_information * (1.0 - max(0.0, min(1.0, self.risk)))) / self.cost


class ActiveProbePlanner:
    """Cost/risk-aware greedy experiment selector."""

    def select(self, candidates: Iterable[ProbeCandidate]) -> ProbeCandidate | None:
        items = tuple(candidates)
        return max(items, key=lambda c: (c.score(), c.expected_information, c.identifier), default=None)


@dataclass(frozen=True)
class CapabilityNode:
    name: str
    prerequisites: tuple[str, ...] = ()


@dataclass
class CapabilityGraph:
    """Dependency graph for compositional skills."""

    nodes: dict[str, CapabilityNode] = field(default_factory=dict)

    def add(self, node: CapabilityNode) -> None:
        if node.name in self.nodes:
            return
        self.nodes[node.name] = node

    def plan(self, target: str) -> tuple[str, ...] | None:
        if target not in self.nodes:
            return None
        visiting: set[str] = set()
        visited: set[str] = set()
        ordered: list[str] = []

        def visit(name: str) -> None:
            if name in visited:
                return
            if name in visiting:
                raise ValueError("capability dependency cycle")
            visiting.add(name)
            node = self.nodes[name]
            for dep in node.prerequisites:
                if dep not in self.nodes:
                    raise KeyError(dep)
                visit(dep)
            visiting.remove(name)
            visited.add(name)
            ordered.append(name)

        try:
            visit(target)
        except (KeyError, ValueError):
            return None
        return tuple(ordered)


@dataclass(frozen=True)
class MemoryItem:
    key: str
    payload: object
    salience: float
    verified: bool = False


@dataclass
class ModularMemory:
    """Bounded memory with protected verified items and salience-based eviction."""

    capacity: int = 128
    items: dict[str, MemoryItem] = field(default_factory=dict)

    def put(self, item: MemoryItem) -> None:
        self.items[item.key] = item
        self._trim()

    def get(self, key: str) -> MemoryItem | None:
        return self.items.get(key)

    def retrieve(self, *, minimum_salience: float = 0.0) -> tuple[MemoryItem, ...]:
        return tuple(sorted(
            (item for item in self.items.values() if item.salience >= minimum_salience),
            key=lambda item: (-item.verified, -item.salience, item.key),
        ))

    def _trim(self) -> None:
        while len(self.items) > max(0, self.capacity):
            candidates = [item for item in self.items.values() if not item.verified]
            if not candidates:
                break
            victim = min(candidates, key=lambda item: (item.salience, item.key))
            self.items.pop(victim.key, None)


@dataclass(frozen=True)
class InformationGainModel:
    """Tiny entropy helper for finite hypotheses."""

    probabilities: tuple[float, ...]

    def entropy(self) -> float:
        return -sum(p * log(p, 2) for p in self.probabilities if p > 0.0)

    def normalized(self) -> "InformationGainModel":
        total = sum(self.probabilities)
        if total <= 0:
            raise ValueError("probabilities must have positive mass")
        return InformationGainModel(tuple(p / total for p in self.probabilities))
