"""Bounded local adaptation for SIM v1.

This is not unrestricted training: it updates finite integer prototypes only after
an explicit label, keeps one rollback snapshot, and never changes authority.
"""
from __future__ import annotations

from dataclasses import dataclass
from typing import Iterable

from symbiotic_intelligence import LABELS, Pattern


@dataclass(frozen=True)
class PrototypeSnapshot:
    version: int
    prototypes: tuple[tuple[int, ...], ...]
    samples: tuple[int, ...]


class LocalPrototypeBank:
    def __init__(self, *, dimensions: int = 4, max_abs: int = 127, max_samples: int = 255):
        if dimensions != 4 or max_abs < 1 or max_samples < 1:
            raise ValueError("prototype_policy_invalid")
        self.dimensions = dimensions
        self.max_abs = max_abs
        self.max_samples = max_samples
        self._version = 1
        self._prototypes = [[0] * dimensions for _ in LABELS]
        self._samples = [0] * len(LABELS)
        self._previous: PrototypeSnapshot | None = None

    @property
    def version(self) -> int:
        return self._version

    def snapshot(self) -> PrototypeSnapshot:
        return PrototypeSnapshot(self._version, tuple(tuple(row) for row in self._prototypes), tuple(self._samples))

    def update(self, pattern: Pattern, label: str) -> PrototypeSnapshot:
        if label not in LABELS or label == "UNKNOWN" or len(pattern.features) != self.dimensions:
            raise ValueError("prototype_label_or_shape_invalid")
        if not pattern.source or not pattern.digest:
            raise ValueError("prototype_provenance_missing")
        index = LABELS.index(label)
        if self._samples[index] >= self.max_samples:
            raise ValueError("prototype_sample_budget_exhausted")
        self._previous = self.snapshot()
        count = self._samples[index]
        for i, value in enumerate(pattern.features):
            bounded = max(-self.max_abs, min(self.max_abs, int(value)))
            self._prototypes[index][i] = (self._prototypes[index][i] * count + bounded) // (count + 1)
        self._samples[index] += 1
        self._version += 1
        return self.snapshot()

    def rollback(self) -> PrototypeSnapshot:
        if self._previous is None:
            raise ValueError("prototype_no_rollback")
        old = self._previous
        self._version = old.version
        self._prototypes = [list(row) for row in old.prototypes]
        self._samples = list(old.samples)
        self._previous = None
        return self.snapshot()

    def score(self, pattern: Pattern) -> tuple[str, int]:
        if len(pattern.features) != self.dimensions:
            raise ValueError("prototype_shape_invalid")
        ranked = []
        for index, prototype in enumerate(self._prototypes):
            if self._samples[index] == 0:
                continue
            distance = sum(abs(int(value) - prototype[i]) for i, value in enumerate(pattern.features))
            ranked.append((distance, LABELS[index]))
        if not ranked:
            return "UNKNOWN", 0
        ranked.sort()
        best = ranked[0]
        margin = (ranked[1][0] - best[0]) if len(ranked) > 1 else 0
        return best[1], max(0, min(1000, 500 + margin * 25))


def train_bounded(bank: LocalPrototypeBank, examples: Iterable[tuple[Pattern, str]], *, max_updates: int = 8) -> PrototypeSnapshot:
    if not 1 <= max_updates <= bank.max_samples:
        raise ValueError("training_update_budget_invalid")
    count = 0
    for pattern, label in examples:
        if count >= max_updates:
            break
        bank.update(pattern, label)
        count += 1
    return bank.snapshot()
