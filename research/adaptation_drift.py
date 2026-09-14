"""Finite drift detection for HERUS adaptive host beliefs."""
from __future__ import annotations

from dataclasses import dataclass


@dataclass(frozen=True)
class Belief:
    key: str
    value: float
    tolerance: float
    observations: int
    host_digest: str
    valid: bool = True


@dataclass(frozen=True)
class DriftResult:
    key: str
    host_digest: str
    observed: float
    baseline: float
    delta: float
    drifted: bool
    action: str


def assess_drift(belief: Belief, observed: float, host_digest: str) -> DriftResult:
    delta = observed - belief.value
    host_changed = host_digest != belief.host_digest
    drifted = host_changed or abs(delta) > belief.tolerance
    action = "INVALIDATE_AND_RENEGOTIATE" if drifted else "KEEP_BELIEF"
    return DriftResult(belief.key, host_digest, observed, belief.value, delta, drifted, action)
