"""Fail-closed comparison primitives for real-data semantic convergence.

This module does not infer alignment. A multimodal rate is computable only when
both observations carry the same source, verified sample identity, and an
explicit PAIRED alignment class. INTRAMODAL observations may only be compared
within the same modality; UNPAIRED observations are never comparable.
"""
from __future__ import annotations

from dataclasses import dataclass
from enum import Enum


class AlignmentClass(str, Enum):
    PAIRED = "PAIRED"
    INTRAMODAL = "INTRAMODAL"
    UNPAIRED = "UNPAIRED"


class IdentityStatus(str, Enum):
    VERIFIED = "VERIFIED"
    AMBIGUOUS = "AMBIGUOUS"
    CONFLICT = "CONFLICT"


class PairingBasis(str, Enum):
    SOURCE_SAMPLE_ID = "SOURCE_SAMPLE_ID"
    LABEL = "LABEL"
    ORDER = "ORDER"
    PATH_BASENAME = "PATH_BASENAME"


@dataclass(frozen=True)
class Observation:
    source: str
    sample_id: str
    modality: str
    alignment: AlignmentClass
    semantic_key: str
    identity_status: IdentityStatus = IdentityStatus.VERIFIED
    pairing_basis: PairingBasis = PairingBasis.SOURCE_SAMPLE_ID
    timestamp_ms: int | None = None


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

    for observation in (left, right):
        if observation.identity_status is IdentityStatus.AMBIGUOUS:
            return Comparison(False, False, None, "identity_ambiguous")
        if observation.identity_status is IdentityStatus.CONFLICT:
            return Comparison(False, False, None, "identity_conflict")
        if observation.pairing_basis is PairingBasis.LABEL:
            return Comparison(False, False, None, "label_only_pairing")
        if observation.pairing_basis is PairingBasis.ORDER:
            return Comparison(False, False, None, "order_only_pairing")
        if observation.pairing_basis is PairingBasis.PATH_BASENAME:
            return Comparison(False, False, None, "basename_only_pairing")

    if (left.timestamp_ms is None) != (right.timestamp_ms is None):
        return Comparison(False, False, None, "timestamp_mismatch")
    if left.timestamp_ms is not None and left.timestamp_ms != right.timestamp_ms:
        return Comparison(False, False, None, "timestamp_mismatch")

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
    sample_keys = [(left.source, left.sample_id) for left, _ in pairs]
    if len(set(sample_keys)) != len(sample_keys):
        raise ValueError("invalid_multimodal_pairs:duplicate_sample_id")
    return sum(bool(item.equal) for item in comparisons) / len(comparisons)
