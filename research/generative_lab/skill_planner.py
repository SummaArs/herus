"""Retrieve-first planner for verified host-only Skills."""
from __future__ import annotations

from itertools import permutations

from .skill_library import SkillLibrary, compose_sequential
from .skill_verifier import ArithmeticCase, verify_arithmetic_skill
from .skills import Skill, SkillState


def retrieve_or_compose(
    library: SkillLibrary,
    *,
    input_type: str,
    output_type: str,
    visible: tuple[ArithmeticCase, ...],
    hidden: tuple[ArithmeticCase, ...],
    composed_id: str,
) -> tuple[Skill | None, dict[str, int]]:
    """Prefer direct verified Skills, then bounded pair composition."""
    direct = library.retrieve(input_type, output_type, minimum=SkillState.VERIFIED)
    if direct:
        return direct[0], {"direct_hits": 1, "composition_attempts": 0, "verified_compositions": 0}
    verified = library.retrieve(input_type, input_type, minimum=SkillState.VERIFIED)
    attempts = 0
    for first, second in permutations(verified, 2):
        attempts += 1
        try:
            candidate = compose_sequential(first, second, skill_id=composed_id)
        except (TypeError, ValueError):
            continue
        result = verify_arithmetic_skill(candidate, visible, hidden)
        if result.passed:
            return candidate, {"direct_hits": 0, "composition_attempts": attempts, "verified_compositions": 1}
    return None, {"direct_hits": 0, "composition_attempts": attempts, "verified_compositions": 0}
