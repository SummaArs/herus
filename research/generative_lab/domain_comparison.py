"""Cross-domain comparison for the bounded Skill experiments."""
from __future__ import annotations

import json

from .generalization_benchmark import run as run_arithmetic
from .policy_experiment import run as run_policy


def run() -> dict[str, object]:
    arithmetic = run_arithmetic()
    policy = run_policy()
    arithmetic_tasks = arithmetic["tasks"]
    return {
        "schema": "herus.generative_lab.domain_comparison",
        "version": 1,
        "authority": "none",
        "domains": {
            "arithmetic": {
                "task_count": len(arithmetic_tasks),
                "verified_tasks": sum(1 for task in arithmetic_tasks if task["skill_found"]),
                "hidden_cases": sum(task["hidden_passed"] for task in arithmetic_tasks),
                "counterexamples": sum(task["counterexamples"] for task in arithmetic_tasks),
            },
            "finite_policy": {
                "candidate_count": policy["metrics"]["candidates"],
                "verified": policy["metrics"]["verified"],
                "hidden_cases": policy["verification"]["total"] - 2,
                "unknown_context_accepted": policy["unknown_context"]["accepted"],
            },
        },
        "shared_invariants": [
            "finite vocabulary",
            "bounded enumeration",
            "independent verification",
            "hidden-case acceptance",
            "no authority",
        ],
        "domain_specificity": [
            "The arithmetic verifier interprets RPN integer programs.",
            "The policy verifier interprets total signal-to-action tables.",
            "Passing one DSL does not imply competence in another DSL.",
        ],
    }


if __name__ == "__main__":
    print(json.dumps(run(), indent=2, sort_keys=True))
