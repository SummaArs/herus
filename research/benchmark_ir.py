"""Finite benchmark IR for external spoken-language-understanding labels.

This is deliberately separate from HERUS Semantic IR. It can represent an
external dataset's finite slots for measurement, but it has no conversion path
to a HERUS event or firmware command.
"""
from __future__ import annotations

from dataclasses import dataclass
from enum import Enum
from typing import Iterable


MAX_VALUES_PER_SLOT = 64
MAX_VALUE_BYTES = 64


class BenchmarkStatus(str, Enum):
    ACCEPTED = "ACCEPTED"
    UNKNOWN = "UNKNOWN"
    AMBIGUOUS = "AMBIGUOUS"
    CONFLICT = "CONFLICT"


@dataclass(frozen=True)
class BenchmarkVocabulary:
    actions: frozenset[str]
    objects: frozenset[str]
    locations: frozenset[str]

    @staticmethod
    def _finite(values: Iterable[str], slot: str) -> frozenset[str]:
        materialised = frozenset(values)
        if not materialised or len(materialised) > MAX_VALUES_PER_SLOT:
            raise ValueError(f"invalid_finite_{slot}_vocabulary")
        for value in materialised:
            encoded = value.encode("ascii")
            if not value or len(encoded) > MAX_VALUE_BYTES or value != value.strip():
                raise ValueError(f"invalid_{slot}_value")
        return materialised

    @classmethod
    def create(
        cls,
        actions: Iterable[str],
        objects: Iterable[str],
        locations: Iterable[str],
    ) -> "BenchmarkVocabulary":
        return cls(
            cls._finite(actions, "action"),
            cls._finite(objects, "object"),
            cls._finite(locations, "location"),
        )


@dataclass(frozen=True)
class BenchmarkIR:
    source: str
    sample_id: str
    status: BenchmarkStatus
    action: str | None
    object: str | None
    location: str | None
    meaning_key: tuple[str, str, str] | None
    operational_authority: bool = False


def compile_slots(
    vocabulary: BenchmarkVocabulary,
    *,
    source: str,
    sample_id: str,
    action: str,
    object: str,
    location: str,
) -> BenchmarkIR:
    if not source or not sample_id:
        raise ValueError("missing_provenance")
    values = (action, object, location)
    if any(not isinstance(value, str) or not value for value in values):
        return BenchmarkIR(source, sample_id, BenchmarkStatus.UNKNOWN, None, None, None, None)
    if action not in vocabulary.actions or object not in vocabulary.objects or location not in vocabulary.locations:
        return BenchmarkIR(source, sample_id, BenchmarkStatus.UNKNOWN, None, None, None, None)
    key = (action, object, location)
    return BenchmarkIR(source, sample_id, BenchmarkStatus.ACCEPTED, action, object, location, key)


def reject_to_herus(_: BenchmarkIR) -> None:
    """Make the absence of an external-to-HERUS authority path executable."""
    raise ValueError("benchmark_ir_not_herus")
