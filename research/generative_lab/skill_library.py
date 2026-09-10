"""Append-only host-only Skill library and typed composition."""
from __future__ import annotations

from dataclasses import replace
from typing import Iterable

from .skills import Skill, SkillState, TRUST_ORDER, candidate_skill


class SkillLibrary:
    def __init__(self) -> None:
        self._skills: dict[tuple[str, int], Skill] = {}

    def add(self, skill: Skill) -> None:
        key = (skill.skill_id, skill.version)
        if key in self._skills:
            raise ValueError(f"skill already exists: {skill.skill_id}@{skill.version}")
        if skill.state in {SkillState.AUTHORIZED, SkillState.ACTIVE}:
            raise ValueError("library cannot authorize or activate skills")
        self._skills[key] = skill

    def get(self, skill_id: str, version: int = 1) -> Skill:
        return self._skills[(skill_id, version)]

    def all(self) -> tuple[Skill, ...]:
        return tuple(self._skills.values())

    def retrieve(self, input_type: str, output_type: str, minimum: SkillState = SkillState.VERIFIED) -> tuple[Skill, ...]:
        minimum_rank = TRUST_ORDER[minimum]
        return tuple(
            skill for skill in self._skills.values()
            if skill.input_type == input_type
            and skill.output_type == output_type
            and TRUST_ORDER[skill.state] >= minimum_rank
        )

    def attest(self, skill_id: str, version: int, *, evidence_hash: str, generalization_score: float, reliability: float) -> Skill:
        current = self.get(skill_id, version)
        if current.state != SkillState.TESTED:
            raise ValueError("only TESTED skills may be attested")
        if not evidence_hash:
            raise ValueError("evidence_hash is required")
        if not evidence_hash.startswith(f"{current.content_hash()}:"):
            raise ValueError("evidence_hash is not bound to the current skill content")
        updated = replace(
            current,
            state=SkillState.VERIFIED,
            evidence_hash=evidence_hash,
            generalization_score=generalization_score,
            reliability=reliability,
        )
        self._skills[(skill_id, version)] = updated
        return updated

    def transition(self, skill_id: str, version: int, state: SkillState) -> Skill:
        current = self.get(skill_id, version)
        allowed = {
            SkillState.CANDIDATE: {SkillState.QUARANTINED},
            SkillState.QUARANTINED: {SkillState.TESTED, SkillState.ARCHIVED},
            SkillState.TESTED: {SkillState.VERIFIED, SkillState.ARCHIVED},
            SkillState.VERIFIED: {SkillState.DEPRECATED, SkillState.ARCHIVED},
            SkillState.DEPRECATED: {SkillState.ARCHIVED},
        }
        if state not in allowed.get(current.state, set()):
            raise ValueError(f"invalid transition {current.state.value}->{state.value}")
        updated = replace(current, state=state)
        self._skills[(skill_id, version)] = updated
        return updated


def compose_sequential(first: Skill, second: Skill, *, skill_id: str) -> Skill:
    """Compose two verified, effect-free procedures without executing them."""
    if first.state not in {SkillState.VERIFIED, SkillState.TESTED} or second.state not in {SkillState.VERIFIED, SkillState.TESTED}:
        raise ValueError("only tested or verified skills may be composed")
    if first.output_type != second.input_type:
        raise TypeError(f"type mismatch: {first.output_type} != {second.input_type}")
    if first.allowed_effects or second.allowed_effects:
        raise ValueError("effectful skills cannot enter this host-only composer")
    return candidate_skill(
        skill_id=skill_id,
        input_type=first.input_type,
        output_type=second.output_type,
        program=first.program + ("COMPOSE",) + second.program,
        dependencies=(f"{first.skill_id}@{first.version}", f"{second.skill_id}@{second.version}"),
        preconditions=first.preconditions + second.preconditions,
        postconditions=first.postconditions + second.postconditions,
        provenance={"kind": "composition", "parents": [first.content_hash(), second.content_hash()]},
        resource_budget={
            "steps": first.resource_budget.get("steps", 0) + second.resource_budget.get("steps", 0),
            "memory": max(first.resource_budget.get("memory", 0), second.resource_budget.get("memory", 0)),
        },
    )
