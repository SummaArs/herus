"""Multi-task hidden-case benchmark for bounded Skill synthesis."""
from __future__ import annotations

import json

from .skill_synthesis import synthesize_arithmetic_skill
from .skill_verifier import ArithmeticCase, verify_arithmetic_skill


TASKS = {
    "affine_2x_plus_3": {
        "visible": ((0, 3), (1, 5), (2, 7)),
        "hidden": ((-3, -3), (4, 11), (8, 19)),
    },
    "affine_neg_x_plus_1": {
        "visible": ((0, 1), (1, 0), (2, -1)),
        "hidden": ((-4, 5), (7, -6), (10, -9)),
    },
    "affine_3x_minus_2": {
        "visible": ((0, -2), (1, 1), (2, 4)),
        "hidden": ((-3, -11), (5, 13), (9, 25)),
    },
}


def run() -> dict[str, object]:
    rows: list[dict[str, object]] = []
    for task_id, definition in TASKS.items():
        visible = tuple(ArithmeticCase(x, y) for x, y in definition["visible"])
        hidden = tuple(ArithmeticCase(x, y) for x, y in definition["hidden"])
        skill, metrics = synthesize_arithmetic_skill(
            skill_id=task_id,
            visible=visible,
            hidden=hidden,
            max_program_length=7,
        )
        verification = verify_arithmetic_skill(skill, visible, hidden) if skill else None
        rows.append({
            "task": task_id,
            "skill_found": skill is not None,
            "program": list(skill.program) if skill else None,
            "candidates": metrics["candidates"],
            "rejected": metrics["rejected"],
            "visible_passed": verification.visible_passed if verification else 0,
            "hidden_passed": verification.hidden_passed if verification else 0,
            "counterexamples": len(verification.counterexamples) if verification else len(hidden),
        })
    return {
        "schema": "herus.generative_lab.generalization_benchmark",
        "version": 1,
        "authority": "none",
        "tasks": rows,
        "all_tasks_verified": all(row["skill_found"] and row["counterexamples"] == 0 for row in rows),
        "claims": [
            "The same bounded synthesis and independent verifier can be reused across multiple affine tasks.",
            "Hidden cases are part of acceptance, not an after-the-fact score.",
            "This remains a closed arithmetic benchmark, not open-ended reasoning.",
        ],
    }


if __name__ == "__main__":
    print(json.dumps(run(), indent=2, sort_keys=True))
