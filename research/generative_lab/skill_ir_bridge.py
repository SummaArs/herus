"""Fail-closed proposal-only bridge from verified Skills to Semantic IR."""
from __future__ import annotations

from typing import Any

from semantic_ir import SemanticProposal, compile_ir

from .skills import Skill, SkillState


def skill_to_proposal(
    skill: Skill,
    *,
    event_kind: str,
    source: str = "CODE",
    confidence_pct: int = 90,
    runner_up_pct: int = 0,
    minutes: int | None = None,
    evidence_ref: str = "skill:verified",
) -> tuple[SemanticProposal | None, tuple[object, ...]]:
    """Convert only a verified proposal-producing Skill into Semantic IR.

    This function emits a proposal object only. It never calls the firmware bridge
    and never interprets a Skill as authorization.
    """
    if skill.state != SkillState.VERIFIED:
        return None, ("skill_not_verified",)
    if skill.output_type != "IntentProposal":
        return None, ("output_type_not_proposal",)
    if skill.allowed_effects:
        return None, ("skill_has_effects",)
    if not skill.evidence_hash:
        return None, ("missing_skill_evidence",)
    value: dict[str, Any] = {
        "schemaVersion": 1,
        "eventKind": event_kind,
        "source": source,
        "confidencePct": confidence_pct,
        "runnerUpPct": runner_up_pct,
        "slots": {"minutes": minutes},
        "evidence": [{"kind": "RULE", "ref": evidence_ref, "polarity": "POSITIVE", "weight": 90}],
        "hypothesisStatus": "TRUE",
        "authority": "PROPOSAL_ONLY",
    }
    proposal, issues = compile_ir(value)
    return proposal, issues
