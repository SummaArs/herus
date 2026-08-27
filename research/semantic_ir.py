"""Semantic IR v0.1: finite, typed and fail-closed bridge for HERUS.

This is a host-only research artifact. It deliberately accepts a small vocabulary
and emits only a proposal. It never performs an action, writes memory or sends a
frame. The firmware remains the authority for admission and confirmation.
"""
from __future__ import annotations

from dataclasses import dataclass
from enum import Enum
from typing import Any, Mapping


class EventKind(str, Enum):
    ARRIVE = "ARRIVE"
    HELP = "HELP"
    CANCEL = "CANCEL"


class Source(str, Enum):
    TEXT = "TEXT"
    VOICE = "VOICE"
    BUTTON = "BUTTON"
    SENSOR = "SENSOR"
    VISION = "VISION"
    CODE = "CODE"
    ENVIRONMENT = "ENVIRONMENT"


class Polarity(str, Enum):
    POSITIVE = "POSITIVE"
    NEGATIVE = "NEGATIVE"


class HypothesisStatus(str, Enum):
    TRUE = "TRUE"
    FALSE = "FALSE"
    BOTH = "BOTH"
    NEITHER = "NEITHER"


ALLOWED_EVIDENCE_KINDS = {"OBSERVATION", "RULE", "MODEL", "USER"}
ALLOWED_KEYS = {
    "schemaVersion",
    "eventKind",
    "source",
    "confidencePct",
    "runnerUpPct",
    "slots",
    "evidence",
    "hypothesisStatus",
    "authority",
}


@dataclass(frozen=True)
class IRIssue:
    path: str
    code: str
    message: str


@dataclass(frozen=True)
class SemanticProposal:
    event_kind: EventKind
    source: Source
    confidence_pct: int
    runner_up_pct: int
    minutes: int | None
    hypothesis_status: HypothesisStatus
    evidence: tuple[Mapping[str, Any], ...]

    @property
    def proposal_only(self) -> bool:
        return True


def _issue(path: str, code: str, message: str) -> IRIssue:
    return IRIssue(path, code, message)


def validate_ir(value: Any) -> tuple[IRIssue, ...]:
    """Validate the closed IR contract without coercion or defaulting."""
    issues: list[IRIssue] = []
    if not isinstance(value, dict):
        return (_issue("$", "TYPE", "Semantic IR must be an object"),)
    unknown = sorted(set(value) - ALLOWED_KEYS)
    for key in unknown:
        issues.append(_issue(f"$.{key}", "UNKNOWN_KEY", "key is outside the Semantic IR contract"))
    required = sorted(ALLOWED_KEYS - set(value))
    for key in required:
        issues.append(_issue(f"$.{key}", "REQUIRED", "required field is missing"))
    if issues:
        return tuple(issues)

    if value["schemaVersion"] != 1:
        issues.append(_issue("$.schemaVersion", "VERSION", "only schemaVersion 1 is accepted"))
    if value["eventKind"] not in {item.value for item in EventKind}:
        issues.append(_issue("$.eventKind", "ENUM", "eventKind is outside the finite vocabulary"))
    if value["source"] not in {item.value for item in Source}:
        issues.append(_issue("$.source", "ENUM", "source is outside the finite vocabulary"))
    for key in ("confidencePct", "runnerUpPct"):
        number = value[key]
        if isinstance(number, bool) or not isinstance(number, int) or not 0 <= number <= 100:
            issues.append(_issue(f"$.{key}", "RANGE", "must be an integer in 0..100"))
    if value["authority"] != "PROPOSAL_ONLY":
        issues.append(_issue("$.authority", "AUTHORITY", "only PROPOSAL_ONLY is accepted"))
    if value["hypothesisStatus"] not in {item.value for item in HypothesisStatus}:
        issues.append(_issue("$.hypothesisStatus", "ENUM", "invalid four-valued hypothesis status"))

    slots = value["slots"]
    if not isinstance(slots, dict):
        issues.append(_issue("$.slots", "TYPE", "slots must be an object"))
    else:
        if set(slots) != {"minutes"}:
            for key in sorted(set(slots) - {"minutes"}):
                issues.append(_issue(f"$.slots.{key}", "UNKNOWN_KEY", "slot is outside the finite vocabulary"))
            if "minutes" not in slots:
                issues.append(_issue("$.slots.minutes", "REQUIRED", "minutes slot is required, including as null"))
        minutes = slots.get("minutes")
        if minutes is not None and (isinstance(minutes, bool) or not isinstance(minutes, int) or not 1 <= minutes <= 60):
            issues.append(_issue("$.slots.minutes", "RANGE", "minutes must be null or an integer in 1..60"))
        if value.get("eventKind") != EventKind.ARRIVE.value and minutes is not None:
            issues.append(_issue("$.slots.minutes", "SEMANTIC", "minutes is only valid for ARRIVE"))

    evidence = value["evidence"]
    if not isinstance(evidence, list) or not 1 <= len(evidence) <= 8:
        issues.append(_issue("$.evidence", "CARDINALITY", "evidence must contain 1..8 items"))
    elif any(not isinstance(item, dict) for item in evidence):
        issues.append(_issue("$.evidence", "TYPE", "each evidence item must be an object"))
    else:
        for index, item in enumerate(evidence):
            prefix = f"$.evidence[{index}]"
            if set(item) != {"kind", "ref", "polarity", "weight"}:
                for key in sorted(set(item) - {"kind", "ref", "polarity", "weight"}):
                    issues.append(_issue(f"{prefix}.{key}", "UNKNOWN_KEY", "evidence key is not canonical"))
                for key in sorted({"kind", "ref", "polarity", "weight"} - set(item)):
                    issues.append(_issue(f"{prefix}.{key}", "REQUIRED", "evidence field is required"))
                continue
            if item["kind"] not in ALLOWED_EVIDENCE_KINDS:
                issues.append(_issue(f"{prefix}.kind", "ENUM", "evidence kind is outside the finite vocabulary"))
            ref = item["ref"]
            if not isinstance(ref, str) or not 1 <= len(ref) <= 48 or not all(char.isalnum() or char in "_.:-" for char in ref):
                issues.append(_issue(f"{prefix}.ref", "FORMAT", "ref must be a short canonical identifier"))
            if item["polarity"] not in {item.value for item in Polarity}:
                issues.append(_issue(f"{prefix}.polarity", "ENUM", "polarity must be POSITIVE or NEGATIVE"))
            weight = item["weight"]
            if isinstance(weight, bool) or not isinstance(weight, int) or not 0 <= weight <= 100:
                issues.append(_issue(f"{prefix}.weight", "RANGE", "weight must be an integer in 0..100"))
    return tuple(issues)


def compile_ir(value: Mapping[str, Any]) -> tuple[SemanticProposal | None, tuple[IRIssue, ...]]:
    """Compile accepted IR to a proposal; never coerce invalid data."""
    issues = validate_ir(value)
    if issues:
        return None, issues
    evidence = tuple(dict(item) for item in value["evidence"])
    proposal = SemanticProposal(
        event_kind=EventKind(value["eventKind"]),
        source=Source(value["source"]),
        confidence_pct=value["confidencePct"],
        runner_up_pct=value["runnerUpPct"],
        minutes=value["slots"]["minutes"],
        hypothesis_status=HypothesisStatus(value["hypothesisStatus"]),
        evidence=evidence,
    )
    return proposal, ()


def to_firmware_command(proposal: SemanticProposal) -> tuple[str, int] | None:
    """Map only unambiguous proposals to the existing finite command vocabulary."""
    if proposal.hypothesis_status != HypothesisStatus.TRUE:
        return None
    if proposal.confidence_pct < 80 or proposal.confidence_pct - proposal.runner_up_pct < 15:
        return None
    if proposal.event_kind is EventKind.ARRIVE:
        return "VOICE_COMMAND_ARRIVE", proposal.minutes or 0
    if proposal.event_kind is EventKind.HELP:
        return "VOICE_COMMAND_HELP", 0
    if proposal.event_kind is EventKind.CANCEL:
        return "VOICE_COMMAND_CANCEL", 0
    return None


def meaning_key(proposal: SemanticProposal) -> tuple[object, ...]:
    """Canonical semantic identity used for cross-modal convergence tests."""
    return (proposal.event_kind.value, proposal.minutes)
