"""Retrieve-first composition experiment."""
from __future__ import annotations

import json

from .skill_library import SkillLibrary
from .skill_planner import retrieve_or_compose
from .skill_verifier import ArithmeticCase, verify_arithmetic_skill
from .skills import SkillState, candidate_skill


def run() -> dict[str, object]:
    library = SkillLibrary()
    double = candidate_skill(
        skill_id="double",
        input_type="Int",
        output_type="Int",
        program=("INPUT", "CONST:2", "MUL"),
        provenance={"kind": "seed_skill", "source": "fixture"},
    ).with_state(SkillState.VERIFIED)
    add_three = candidate_skill(
        skill_id="add-three",
        input_type="Int",
        output_type="Int",
        program=("INPUT", "CONST:3", "ADD"),
        provenance={"kind": "seed_skill", "source": "fixture"},
    ).with_state(SkillState.VERIFIED)
    library.add(double)
    library.add(add_three)
    visible = (ArithmeticCase(0, 3), ArithmeticCase(2, 7))
    hidden = (ArithmeticCase(-4, -5), ArithmeticCase(9, 21))
    composed, metrics = retrieve_or_compose(
        library,
        input_type="Int",
        output_type="Token",
        visible=visible,
        hidden=hidden,
        composed_id="double-then-add-three",
    )
    verification = verify_arithmetic_skill(composed, visible, hidden) if composed else None
    return {
        "schema": "herus.generative_lab.composition_experiment",
        "version": 1,
        "authority": "none",
        "seed_skills": [double.skill_id, add_three.skill_id],
        "planner_metrics": metrics,
        "composed_skill": composed.canonical_dict() if composed else None,
        "verification": {
            "passed": verification.passed,
            "visible_passed": verification.visible_passed,
            "hidden_passed": verification.hidden_passed,
            "counterexamples": len(verification.counterexamples),
        } if verification else None,
        "claims": [
            "Verified skills can be reused before new synthesis.",
            "Typed composition can create a new procedure that passes hidden cases.",
            "Composition does not grant authority.",
        ],
    }


if __name__ == "__main__":
    print(json.dumps(run(), indent=2, sort_keys=True))
