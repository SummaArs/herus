"""Release gate for the first physical ASA firmware.

This gate does not flash a board and does not authorize effects. It only decides
whether a bench-only firmware package is eligible to be tested after the
host-only proof and B1/B2 identity checks are present.
"""
from __future__ import annotations

from dataclasses import dataclass
from typing import Mapping

from hardware_gate_preflight import evaluate as evaluate_preflight


ALLOWED_BENCH_MODES = ("OBSERVE", "PROPOSE", "HAPTIC_FEEDBACK")
FORBIDDEN_BENCH_MODES = ("ACTUATE", "EXECUTE_FINANCE", "AUTO_PAIR", "GRANT_AUTHORITY")


@dataclass(frozen=True)
class ReleaseDecision:
    status: str
    reason: str
    allowed_modes: tuple[str, ...] = ()
    forbidden_modes: tuple[str, ...] = FORBIDDEN_BENCH_MODES


def evaluate_release(*, host_proof_passed: bool, identity_record: Mapping[str, object]) -> ReleaseDecision:
    """Return READY_FOR_BENCH only when every precondition is explicit."""
    if not host_proof_passed:
        return ReleaseDecision("BLOCKED", "host_only_proof_failed")
    preflight = evaluate_preflight(identity_record)
    if preflight.status != "READY_FOR_BENCH":
        return ReleaseDecision("BLOCKED", f"preflight:{preflight.reason}")
    return ReleaseDecision(
        "READY_FOR_BENCH",
        "host_proof_and_b1_b2_passed",
        allowed_modes=ALLOWED_BENCH_MODES,
    )
