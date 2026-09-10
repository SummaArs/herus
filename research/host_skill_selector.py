"""Host-aware selection of already verified Skills.

Selection is not authorization and never executes a Skill. It only produces a
proposal describing whether a verified Skill fits the declared host profile.
"""
from __future__ import annotations

from dataclasses import dataclass

from host_profile import HostProfile
from host_negotiation import negotiate
from generative_lab.skills import Skill, SkillState


@dataclass(frozen=True)
class SkillSelection:
    skill_id: str
    host_digest: str
    status: str
    reason: str
    representation: str = ""


def select_for_host(profile: HostProfile, skill: Skill, representation: str) -> SkillSelection:
    if skill.state is not SkillState.VERIFIED:
        return SkillSelection(skill.skill_id, profile.digest(), "BLOCKED", "skill_not_verified")
    if skill.allowed_effects:
        return SkillSelection(skill.skill_id, profile.digest(), "BLOCKED", "skill_has_effects")
    plan = negotiate(
        profile,
        representation,
        required_bytes=len("|".join(skill.program).encode()),
        required_steps=skill.resource_budget.get("steps", 0),
        required_depth=skill.resource_budget.get("depth", 0),
        required_effects=skill.allowed_effects,
    )
    if plan.status == "BLOCKED":
        return SkillSelection(skill.skill_id, profile.digest(), "BLOCKED", plan.reason)
    return SkillSelection(skill.skill_id, profile.digest(), "PROPOSAL_ONLY", "verified_and_compatible", plan.representation)
