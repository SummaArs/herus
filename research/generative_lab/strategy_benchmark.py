"""Compare bounded synthesis with retrieve-first composition."""
from __future__ import annotations

import json
import time
from dataclasses import replace

from .skill_library import SkillLibrary
from .skill_planner import retrieve_or_compose
from .skill_synthesis import synthesize_arithmetic_skill
from .skill_verifier import ArithmeticCase, verify_arithmetic_skill
from .skills import SkillState, candidate_skill


def run() -> dict[str, object]:
    visible = (ArithmeticCase(0, 3), ArithmeticCase(1, 5), ArithmeticCase(2, 7))
    hidden = (ArithmeticCase(-3, -3), ArithmeticCase(4, 11), ArithmeticCase(8, 19))
    start = time.perf_counter()
    generated, generation_metrics = synthesize_arithmetic_skill(
        skill_id="strategy-generated",
        visible=visible,
        hidden=hidden,
        max_program_length=7,
    )
    generation_ms = round((time.perf_counter() - start) * 1000, 3)
    generated_verification = verify_arithmetic_skill(generated, visible, hidden) if generated else None

    library = SkillLibrary()
    double = candidate_skill(skill_id="strategy-double", input_type="Int", output_type="Int", program=("INPUT", "CONST:2", "MUL"), provenance={"kind": "seed"}).with_state(SkillState.VERIFIED)
    add_three = candidate_skill(skill_id="strategy-add-three", input_type="Int", output_type="Int", program=("INPUT", "CONST:3", "ADD"), provenance={"kind": "seed"}).with_state(SkillState.VERIFIED)
    library.add(double)
    library.add(add_three)
    start = time.perf_counter()
    composed, composition_metrics = retrieve_or_compose(
        library,
        input_type="Int",
        output_type="Token",
        visible=visible,
        hidden=hidden,
        composed_id="strategy-composed",
    )
    composition_ms = round((time.perf_counter() - start) * 1000, 3)
    composed_verification = verify_arithmetic_skill(composed, visible, hidden) if composed else None
    return {
        "schema": "herus.generative_lab.strategy_benchmark",
        "version": 1,
        "authority": "none",
        "strategies": {
            "bounded_synthesis": {
                "candidates": generation_metrics["candidates"],
                "rejected": generation_metrics["rejected"],
                "elapsed_ms": generation_ms,
                "hidden_passed": generated_verification.hidden_passed if generated_verification else 0,
            },
            "retrieve_first_composition": {
                **composition_metrics,
                "elapsed_ms": composition_ms,
                "hidden_passed": composed_verification.hidden_passed if composed_verification else 0,
            },
        },
        "interpretation": [
            "The comparison is a closed arithmetic benchmark, not a claim about general intelligence.",
            "Retrieve-first can reduce search when reusable verified primitives already exist.",
            "Elapsed times are host observations and are not hardware performance claims.",
        ],
    }


if __name__ == "__main__":
    print(json.dumps(run(), indent=2, sort_keys=True))
