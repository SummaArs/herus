"""Finite, fail-closed host capability profile for HERUS ASA."""
from __future__ import annotations

from dataclasses import dataclass, field
from typing import Any
import hashlib
import json

ALLOWED_AUTHORITIES = {"NONE", "PROPOSAL_ONLY", "HUMAN_BOUND"}
KNOWN_INTERFACES = {"button", "haptic", "radio", "serial", "display", "sensor", "actuator"}


@dataclass(frozen=True)
class HostProfile:
    host_id: str
    revision: str
    resources: dict[str, int]
    interfaces: frozenset[str]
    constraints: dict[str, int]
    representation_set: frozenset[str]
    skill_budget: dict[str, int]
    evidence: dict[str, str]
    authority: str = "NONE"

    def canonical(self) -> bytes:
        return json.dumps(self.to_dict(include_digest=False), sort_keys=True, separators=(",", ":")).encode()

    def digest(self) -> str:
        return hashlib.sha256(self.canonical()).hexdigest()

    def to_dict(self, *, include_digest: bool = True) -> dict[str, Any]:
        value: dict[str, Any] = {
            "host_id": self.host_id,
            "revision": self.revision,
            "resources": dict(sorted(self.resources.items())),
            "interfaces": sorted(self.interfaces),
            "constraints": dict(sorted(self.constraints.items())),
            "representation_set": sorted(self.representation_set),
            "skill_budget": dict(sorted(self.skill_budget.items())),
            "evidence": dict(sorted(self.evidence.items())),
            "authority": self.authority,
        }
        if include_digest:
            value["profile_digest"] = self.digest()
        return value


def validate_profile(profile: HostProfile) -> tuple[str, ...]:
    issues: list[str] = []
    if not profile.host_id or not profile.revision:
        issues.append("identity_missing")
    if profile.authority not in ALLOWED_AUTHORITIES:
        issues.append("authority_unknown")
    unknown = profile.interfaces - KNOWN_INTERFACES
    issues.extend(f"interface_unknown:{item}" for item in sorted(unknown))
    if any(value < 0 for value in profile.resources.values()):
        issues.append("resource_negative")
    if any(value < 0 for value in profile.constraints.values()):
        issues.append("constraint_negative")
    if any(value < 0 for value in profile.skill_budget.values()):
        issues.append("skill_budget_negative")
    if not profile.evidence:
        issues.append("evidence_missing")
    if not profile.representation_set:
        issues.append("representation_missing")
    if profile.authority != "NONE":
        issues.append("authority_not_discoverable")
    return tuple(issues)


def discover_profile(raw: dict[str, Any]) -> HostProfile | None:
    """Parse only declared finite metadata; reject malformed/unknown authority."""
    try:
        profile = HostProfile(
            host_id=str(raw["host_id"]),
            revision=str(raw["revision"]),
            resources={str(k): int(v) for k, v in raw["resources"].items()},
            interfaces=frozenset(str(v) for v in raw["interfaces"]),
            constraints={str(k): int(v) for k, v in raw["constraints"].items()},
            representation_set=frozenset(str(v) for v in raw["representation_set"]),
            skill_budget={str(k): int(v) for k, v in raw["skill_budget"].items()},
            evidence={str(k): str(v) for k, v in raw["evidence"].items()},
            authority="NONE",
        )
    except (KeyError, TypeError, ValueError, AttributeError):
        return None
    return profile if not validate_profile(profile) else None
