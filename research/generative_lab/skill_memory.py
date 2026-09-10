"""Utility scoring and conservative archival for the Skill library."""
from __future__ import annotations

from dataclasses import replace
import math

from .skill_library import SkillLibrary
from .skills import Skill, SkillState


def utility_score(skill: Skill) -> float:
    """Rank a skill without treating usage as proof of correctness."""
    cost = max(1, skill.resource_budget.get("steps", 1))
    reuse = math.log1p(skill.usage_count)
    return (0.55 * skill.reliability + 0.45 * skill.generalization_score) * (1.0 + reuse) / cost


class SkillMemory:
    def __init__(self, library: SkillLibrary) -> None:
        self.library = library

    def rank(self) -> tuple[tuple[Skill, float], ...]:
        return tuple(sorted(((skill, utility_score(skill)) for skill in self.library.all()), key=lambda pair: (-pair[1], pair[0].skill_id)))

    def record_use(self, skill_id: str, version: int = 1) -> Skill:
        current = self.library.get(skill_id, version)
        if current.state not in {SkillState.VERIFIED, SkillState.ACTIVE}:
            raise ValueError("only verified or active skills may be reused")
        updated = current.mark_used()
        self.library.replace_record(updated)
        return updated

    def archive_redundant(self) -> tuple[str, ...]:
        """Archive exact duplicate verified procedures, never delete evidence."""
        groups: dict[tuple[str, str, tuple[str, ...]], list[Skill]] = {}
        for skill in self.library.all():
            if skill.state != SkillState.VERIFIED:
                continue
            key = (skill.input_type, skill.output_type, skill.program)
            groups.setdefault(key, []).append(skill)
        dependencies = {dep for skill in self.library.all() for dep in skill.dependencies}
        archived: list[str] = []
        for skills in groups.values():
            if len(skills) < 2:
                continue
            keep = max(skills, key=lambda skill: (utility_score(skill), skill.skill_id))
            for candidate in skills:
                if candidate is keep or f"{candidate.skill_id}@{candidate.version}" in dependencies:
                    continue
                self.library.transition(candidate.skill_id, candidate.version, SkillState.ARCHIVED)
                archived.append(f"{candidate.skill_id}@{candidate.version}")
        return tuple(sorted(archived))
