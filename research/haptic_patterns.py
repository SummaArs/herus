"""Proposal-only finite haptic patterns for the HERUS host.

The module deliberately emits proposals, never hardware writes. A later firmware
adapter may accept a proposal only after host policy and driver health checks.
"""
from __future__ import annotations

from dataclasses import dataclass
from enum import Enum
from typing import Final


class HapticState(str, Enum):
    CONFIRMATION = "CONFIRMATION"
    ATTENTION = "ATTENTION"
    WAITING = "WAITING"
    REFUSED = "REFUSED"
    BLOCKED = "BLOCKED"
    EMERGENCY_PROPOSAL = "EMERGENCY_PROPOSAL"


class Authority(str, Enum):
    NONE = "NONE"
    PROPOSAL_ONLY = "PROPOSAL_ONLY"
    HUMAN_BOUND = "HUMAN_BOUND"


@dataclass(frozen=True)
class HapticEvent:
    state: HapticState
    authority: Authority
    evidence_digest: str
    context: str


@dataclass(frozen=True)
class HapticProposal:
    state: HapticState
    pulses: tuple[tuple[int, int], ...]
    priority: int
    authority: Authority
    evidence_digest: str
    context: str


_PATTERNS: Final[dict[HapticState, tuple[tuple[int, int], ...]]] = {
    HapticState.CONFIRMATION: ((90, 90),),
    HapticState.ATTENTION: ((90, 100), (90, 0)),
    HapticState.WAITING: ((60, 140), (60, 140), (60, 0)),
    HapticState.REFUSED: ((280, 0),),
    HapticState.BLOCKED: ((260, 120), (260, 0)),
    HapticState.EMERGENCY_PROPOSAL: ((120, 120), (120, 120), (120, 0)),
}

_PRIORITY: Final[dict[HapticState, int]] = {
    HapticState.CONFIRMATION: 1,
    HapticState.WAITING: 2,
    HapticState.ATTENTION: 3,
    HapticState.REFUSED: 4,
    HapticState.BLOCKED: 4,
    HapticState.EMERGENCY_PROPOSAL: 5,
}


def propose_haptic(event: HapticEvent) -> HapticProposal | None:
    """Translate a verified semantic state into a non-authoritative proposal.

    Empty evidence, unknown context, or non-proposal authority produces no output.
    Even HUMAN_BOUND authority is not converted automatically: actuation remains
    the responsibility of a separate host policy and driver-health gate.
    """
    if not event.evidence_digest or not event.context.strip():
        return None
    if event.authority is not Authority.PROPOSAL_ONLY:
        return None
    pattern = _PATTERNS.get(event.state)
    if pattern is None:
        return None
    return HapticProposal(
        state=event.state,
        pulses=pattern,
        priority=_PRIORITY[event.state],
        authority=Authority.PROPOSAL_ONLY,
        evidence_digest=event.evidence_digest,
        context=event.context,
    )
