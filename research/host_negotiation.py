"""Representation and skill-budget negotiation for HERUS ASA."""
from __future__ import annotations

from dataclasses import dataclass
from typing import Iterable

from host_profile import HostProfile, validate_profile


@dataclass(frozen=True)
class NegotiatedPlan:
    host_digest: str
    representation: str
    max_skill_bytes: int
    max_steps: int
    max_depth: int
    effects: frozenset[str]
    status: str
    reason: str


def negotiate(profile: HostProfile, required_representation: str, required_bytes: int, required_steps: int, required_depth: int, required_effects: Iterable[str] = ()) -> NegotiatedPlan:
    effects = frozenset(required_effects)
    issues = validate_profile(profile)
    if issues:
        return NegotiatedPlan(profile.digest(), "", 0, 0, 0, effects, "BLOCKED", "invalid_host_profile")
    if required_representation not in profile.representation_set:
        return NegotiatedPlan(profile.digest(), "", 0, 0, 0, effects, "BLOCKED", "representation_unavailable")
    if required_bytes > profile.skill_budget.get("bytes", -1):
        return NegotiatedPlan(profile.digest(), "", 0, 0, 0, effects, "BLOCKED", "skill_bytes_exceeded")
    if required_steps > profile.skill_budget.get("steps", -1):
        return NegotiatedPlan(profile.digest(), "", 0, 0, 0, effects, "BLOCKED", "skill_steps_exceeded")
    if required_depth > profile.skill_budget.get("depth", -1):
        return NegotiatedPlan(profile.digest(), "", 0, 0, 0, effects, "BLOCKED", "skill_depth_exceeded")
    if effects:
        return NegotiatedPlan(profile.digest(), "", 0, 0, 0, effects, "BLOCKED", "effects_require_explicit_binding")
    return NegotiatedPlan(profile.digest(), required_representation, required_bytes, required_steps, required_depth, effects, "PROPOSAL_ONLY", "compatible")
