"""Independent human annotation protocol for HERUS semantic research.

This schema is deliberately not an adapter from an external dataset. It records
an independently judged HERUS label and has no path to a firmware command,
transport frame, or operational authority.
"""
from __future__ import annotations

from dataclasses import dataclass
from enum import Enum
from typing import Any


PROTOCOL_VERSION = "herus-independent-annotation-v1"


class AnnotationLabel(str, Enum):
    ARRIVE = "ARRIVE"
    HELP = "HELP"
    CANCEL = "CANCEL"
    OTHER = "OTHER"
    AMBIGUOUS = "AMBIGUOUS"
    CONFLICT = "CONFLICT"


NON_OPERATIONAL_LABELS = frozenset(
    {
        AnnotationLabel.OTHER,
        AnnotationLabel.AMBIGUOUS,
        AnnotationLabel.CONFLICT,
    }
)


@dataclass(frozen=True)
class IndependentAnnotation:
    evidence_ref: str
    annotator_ref: str
    label: AnnotationLabel
    protocol_version: str = PROTOCOL_VERSION
    independent_of_external_labels: bool = True
    operational_authority: bool = False


def create_annotation(
    *, evidence_ref: str, annotator_ref: str, label: AnnotationLabel | str
) -> IndependentAnnotation:
    if not evidence_ref or not annotator_ref:
        raise ValueError("annotation_provenance_required")
    try:
        parsed_label = label if isinstance(label, AnnotationLabel) else AnnotationLabel(label)
    except ValueError as exc:
        raise ValueError("annotation_label_invalid") from exc
    return IndependentAnnotation(
        evidence_ref=evidence_ref,
        annotator_ref=annotator_ref,
        label=parsed_label,
    )


def classify_disagreement(labels: list[AnnotationLabel | str]) -> AnnotationLabel:
    """Resolve only the annotation state; disagreement never selects a command."""
    if not labels:
        return AnnotationLabel.AMBIGUOUS
    parsed = {label if isinstance(label, AnnotationLabel) else AnnotationLabel(label) for label in labels}
    if len(parsed) == 1:
        return next(iter(parsed))
    return AnnotationLabel.CONFLICT


def external_label_to_annotation(_: Any) -> IndependentAnnotation:
    """There is intentionally no automatic external-label conversion."""
    raise ValueError("external_label_mapping_forbidden")


def annotation_to_operational_command(_: IndependentAnnotation) -> None:
    """Annotations are evidence only; the firmware bridge remains unreachable."""
    raise ValueError("annotation_has_no_operational_authority")


def annotation_is_non_operational(annotation: IndependentAnnotation) -> bool:
    return (
        annotation.label in NON_OPERATIONAL_LABELS
        or not annotation.independent_of_external_labels
        or annotation.operational_authority
    )


def protocol_schema() -> dict[str, Any]:
    return {
        "protocol_version": PROTOCOL_VERSION,
        "labels": [label.value for label in AnnotationLabel],
        "non_operational_labels": sorted(label.value for label in NON_OPERATIONAL_LABELS),
        "requires_independent_human_judgment": True,
        "external_label_mapping": "forbidden",
        "bridge_path": "none",
        "rules": {
            "no_evidence": "AMBIGUOUS",
            "multiple_intents_or_incompatible_judgments": "CONFLICT",
            "no_HERUS_intent": "OTHER",
            "OTHER_AMBIGUOUS_CONFLICT_cross_bridge": "forbidden",
            "annotation_is_not_confirmation": True,
        },
    }
