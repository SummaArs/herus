"""Reproducible first experiment for verified Skill growth."""
from __future__ import annotations

import json
from .skill_library import SkillLibrary
from .skills import SkillState
from .skill_synthesis import synthesize_arithmetic_skill
from .skill_verifier import ArithmeticCase, verify_arithmetic_skill


def run() -> dict[str, object]:
    visible = [ArithmeticCase(0, 3), ArithmeticCase(1, 5), ArithmeticCase(2, 7)]
    hidden = [ArithmeticCase(-3, -3), ArithmeticCase(4, 11), ArithmeticCase(8, 19)]
    skill, metrics = synthesize_arithmetic_skill(
        skill_id="affine-2x-plus-3",
        visible=visible,
        hidden=hidden,
        max_program_length=7,
    )
    library = SkillLibrary()
    if skill is not None:
        library.add(skill)
        library.transition(skill.skill_id, skill.version, SkillState.QUARANTINED)
        verification = verify_arithmetic_skill(skill, visible, hidden)
        library.transition(skill.skill_id, skill.version, SkillState.TESTED)
        if verification.passed:
            library.attest(
                skill.skill_id,
                skill.version,
                evidence_hash=verification.evidence_hash,
                generalization_score=verification.hidden_passed / len(hidden),
                reliability=1.0,
            )
        skill = library.get(skill.skill_id, skill.version)
    else:
        verification = None
    return {
        "schema": "herus.generative_lab.skill_experiment",
        "version": 1,
        "authority": "none",
        "domain": "closed_integer_rpn",
        "visible_cases": [case.__dict__ for case in visible],
        "hidden_cases": [case.__dict__ for case in hidden],
        "skill": skill.canonical_dict() if skill else None,
        "metrics": metrics,
        "verification": {
            "passed": verification.passed,
            "state": verification.state.value,
            "reason": verification.reason,
            "visible_passed": verification.visible_passed,
            "hidden_passed": verification.hidden_passed,
            "counterexamples": [case.__dict__ for case in verification.counterexamples],
            "evidence_hash": verification.evidence_hash,
        } if verification else None,
        "library_size": len(library.all()),
        "claims": [
            "A candidate can be generated from a finite DSL and verified against hidden cases.",
            "Generation does not grant authorization.",
            "This experiment does not demonstrate open-ended reasoning or language understanding.",
        ],
    }


def main() -> None:
    print(json.dumps(run(), indent=2, sort_keys=True))


if __name__ == "__main__":
    main()
