"""Fail-closed comparison primitives for real-data semantic convergence.

This module does not infer alignment. A multimodal rate is computable only when
both observations carry the same source, sample ID, and an explicit PAIRED
alignment class. INTRAMODAL observations may only be compared within the same
modality; UNPAIRED observations are never comparable.
"""
from __future__ import annotations

from dataclasses import dataclass
from enum import Enum


class AlignmentClass(str, Enum):
    PAIRED = "PAIRED"
    INTRAMODAL = "INTRAMODAL"
    UNPAIRED = "UNPAIRED"


@dataclass(frozen=True)
class Observation:
    source: str
    sample_id: str
    modality: str
    alignment: AlignmentClass
    semantic_key: str


@dataclass(frozen=True)
class Comparison:
    allowed: bool
    multimodal: bool
    equal: bool | None
    reason: str


def compare(left: Observation, right: Observation) -> Comparison:
    """Compare two observations without manufacturing a pair.

    ``equal`` is deliberately ``None`` when comparison is refused. A caller
    must not turn a refused comparison into a zero or a one in a rate.
    """
    if left.source != right.source:
        return Comparison(False, False, None, "source_mismatch")
    if left.sample_id != right.sample_id:
        return Comparison(False, False, None, "sample_id_mismatch")

    if left.alignment is AlignmentClass.UNPAIRED or right.alignment is AlignmentClass.UNPAIRED:
        return Comparison(False, False, None, "unpaired_observation")

    if left.alignment is AlignmentClass.PAIRED and right.alignment is AlignmentClass.PAIRED:
        if left.modality == right.modality:
            return Comparison(False, False, None, "paired_requires_distinct_modalities")
        return Comparison(True, True, left.semantic_key == right.semantic_key, "paired_same_source_id")

    if left.alignment is AlignmentClass.INTRAMODAL and right.alignment is AlignmentClass.INTRAMODAL:
        if left.modality != right.modality:
            return Comparison(False, False, None, "intramodal_modality_mismatch")
        return Comparison(True, False, left.semantic_key == right.semantic_key, "intramodal_same_source_id")

    return Comparison(False, False, None, "alignment_class_mismatch")


def multimodal_rate(pairs: list[tuple[Observation, Observation]]) -> float:
    """Return a convergence rate, refusing any invalid or fabricated pair."""
    if not pairs:
        raise ValueError("no_pairs")
    comparisons = [compare(left, right) for left, right in pairs]
    refused = [item.reason for item in comparisons if not item.allowed or not item.multimodal]
    if refused:
        raise ValueError("invalid_multimodal_pairs:" + ",".join(refused))
    return sum(bool(item.equal) for item in comparisons) / len(comparisons)
