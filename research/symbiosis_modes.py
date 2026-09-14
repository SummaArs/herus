"""Finite modes and permission boundary for HERUS symbiosis."""
from __future__ import annotations

from dataclasses import dataclass
from enum import Enum


class SymbiosisMode(str, Enum):
    HOST = "HOST"
    PERSONAL_GUARDIAN = "PERSONAL_GUARDIAN"
    DEFENSIVE_MESH = "DEFENSIVE_MESH"
    DOMAIN_CONNECTOR = "DOMAIN_CONNECTOR"


class PermissionDecision(str, Enum):
    DENY = "DENY"
    PROPOSE = "PROPOSE"
    ALLOW_ONCE = "ALLOW_ONCE"
    ALLOW_SESSION = "ALLOW_SESSION"
    QUARANTINE = "QUARANTINE"
    REVOKE = "REVOKE"


@dataclass(frozen=True)
class PermissionRequest:
    mode: SymbiosisMode
    host_id: str
    channel: str
    effect: str
    evidence_digest: str
    human_confirmation: bool = False


def decide_permission(request: PermissionRequest) -> PermissionDecision:
    """Fail closed: connectivity and discovery never grant authority."""
    if not request.host_id or not request.channel or not request.evidence_digest:
        return PermissionDecision.DENY
    if request.effect == "" or request.effect == "UNKNOWN":
        return PermissionDecision.PROPOSE
    if not request.human_confirmation:
        return PermissionDecision.PROPOSE
    if request.mode == SymbiosisMode.DEFENSIVE_MESH and request.effect == "ISOLATE_DEVICE":
        return PermissionDecision.ALLOW_ONCE
    if request.mode == SymbiosisMode.PERSONAL_GUARDIAN and request.effect in {"HAPTIC_FEEDBACK", "READ_TELEMETRY"}:
        return PermissionDecision.ALLOW_SESSION
    return PermissionDecision.PROPOSE
