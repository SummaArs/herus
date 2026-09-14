"""Map defensive guardian outcomes to finite proposal-only haptic states."""
from __future__ import annotations

from defensive_guardian import DeviceStatus, GuardianDecision
from haptic_patterns import Authority, HapticEvent, HapticState, propose_haptic


def haptic_for_guardian(decision: GuardianDecision, evidence_digest: str):
    state_by_status = {
        DeviceStatus.VERIFIED: HapticState.CONFIRMATION,
        DeviceStatus.UNKNOWN: HapticState.ATTENTION,
        DeviceStatus.DRIFTED: HapticState.WAITING,
        DeviceStatus.QUARANTINED: HapticState.BLOCKED,
        DeviceStatus.REVOKED: HapticState.REFUSED,
    }
    state = state_by_status.get(decision.status)
    if state is None:
        return None
    return propose_haptic(
        HapticEvent(
            state=state,
            authority=Authority.PROPOSAL_ONLY,
            evidence_digest=evidence_digest,
            context=f"guardian:{decision.device_id}:{decision.reason}",
        )
    )
