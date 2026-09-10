"""Verified skill contracts for the host-only HERUS generative laboratory.

A Skill is a typed, serializable transformation proposal. This module deliberately
contains no code execution, no dynamic imports and no authority bridge.
"""
from __future__ import annotations

from dataclasses import dataclass, replace
from enum import Enum
import hashlib
import json
from typing import Any, Mapping


class SkillState(str, Enum):
    CANDIDATE = "CANDIDATE"
    QUARANTINED = "QUARANTINED"
    TESTED = "TESTED"
    VERIFIED = "VERIFIED"
    AUTHORIZED = "AUTHORIZED"
    ACTIVE = "ACTIVE"
    DEPRECATED = "DEPRECATED"
    ARCHIVED = "ARCHIVED"


TRUST_ORDER = {
    SkillState.CANDIDATE: 0,
    SkillState.QUARANTINED: 1,
    SkillState.TESTED: 2,
    SkillState.VERIFIED: 3,
    SkillState.AUTHORIZED: 4,
    SkillState.ACTIVE: 5,
    SkillState.DEPRECATED: 1,
    SkillState.ARCHIVED: 0,
}


@dataclass(frozen=True)
class Skill:
    """A closed-world procedure over semantic states.

    ``program`` is data interpreted by an independent domain verifier. It is not
    Python source and must never be evaluated as code. ``allowed_effects`` is
    intentionally separate from correctness and authority.
    """

    skill_id: str
    version: int
    input_type: str
    output_type: str
    preconditions: tuple[str, ...]
    postconditions: tuple[str, ...]
    program: tuple[str, ...]
    dependencies: tuple[str, ...]
    allowed_effects: tuple[str, ...]
    forbidden_effects: tuple[str, ...]
    resource_budget: Mapping[str, int]
    determinism: str
    provenance: Mapping[str, Any]
    evidence_hash: str
    state: SkillState = SkillState.CANDIDATE
    usage_count: int = 0
    reliability: float = 0.0
    generalization_score: float = 0.0

    def __post_init__(self) -> None:
        if not self.skill_id or self.version < 1:
            raise ValueError("skill_id and positive version are required")
        if not self.input_type or not self.output_type:
            raise ValueError("input_type and output_type are required")
        if any(value < 0 for value in self.resource_budget.values()):
            raise ValueError("resource budgets cannot be negative")
        if not 0.0 <= self.reliability <= 1.0:
            raise ValueError("reliability must be between 0 and 1")
        if not 0.0 <= self.generalization_score <= 1.0:
            raise ValueError("generalization_score must be between 0 and 1")
        if self.state in {SkillState.AUTHORIZED, SkillState.ACTIVE}:
            raise ValueError("authority lifecycle transitions require an external policy gate")

    def canonical_dict(self, *, include_evidence: bool = True) -> dict[str, Any]:
        data: dict[str, Any] = {
            "skill_id": self.skill_id,
            "version": self.version,
            "input_type": self.input_type,
            "output_type": self.output_type,
            "preconditions": list(self.preconditions),
            "postconditions": list(self.postconditions),
            "program": list(self.program),
            "dependencies": list(self.dependencies),
            "allowed_effects": list(self.allowed_effects),
            "forbidden_effects": list(self.forbidden_effects),
            "resource_budget": dict(sorted(self.resource_budget.items())),
            "determinism": self.determinism,
            "provenance": self.provenance,
            "state": self.state.value,
            "usage_count": self.usage_count,
            "reliability": self.reliability,
            "generalization_score": self.generalization_score,
        }
        if include_evidence:
            data["evidence_hash"] = self.evidence_hash
        return data

    def content_hash(self) -> str:
        data = self.canonical_dict(include_evidence=False)
        for administrative in ("state", "usage_count", "reliability", "generalization_score"):
            data.pop(administrative, None)
        payload = json.dumps(data, sort_keys=True, separators=(",", ":")).encode()
        return hashlib.sha256(payload).hexdigest()

    def with_state(self, state: SkillState) -> "Skill":
        if state in {SkillState.AUTHORIZED, SkillState.ACTIVE}:
            raise ValueError("Generation and verification cannot grant authority")
        return replace(self, state=state)

    def mark_used(self) -> "Skill":
        return replace(self, usage_count=self.usage_count + 1)


def candidate_skill(
    *,
    skill_id: str,
    input_type: str,
    output_type: str,
    program: tuple[str, ...],
    provenance: Mapping[str, Any],
    resource_budget: Mapping[str, int] | None = None,
    dependencies: tuple[str, ...] = (),
    preconditions: tuple[str, ...] = (),
    postconditions: tuple[str, ...] = (),
) -> Skill:
    """Construct a candidate with no effects and no authority by default."""
    return Skill(
        skill_id=skill_id,
        version=1,
        input_type=input_type,
        output_type=output_type,
        preconditions=preconditions,
        postconditions=postconditions,
        program=program,
        dependencies=dependencies,
        allowed_effects=(),
        forbidden_effects=("EXECUTE_ACTUATOR", "GRANT_AUTHORITY", "ALTER_VERIFIER"),
        resource_budget=resource_budget or {"steps": 64, "memory": 64},
        determinism="deterministic",
        provenance=provenance,
        evidence_hash="",
    )
