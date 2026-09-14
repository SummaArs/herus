"""Reproducible finite-policy Skill experiment."""
from __future__ import annotations

import json

from .policy_domain import PolicyCase, synthesize_policy_skill, verify_policy_skill


def run() -> dict[str, object]:
    visible = (
        PolicyCase("SAFE", "OK", "WAIT"),
        PolicyCase("WAIT", "TIMEOUT", "ALERT"),
    )
    hidden = (
        PolicyCase("ALERT", "CANCEL", "SAFE"),
        PolicyCase("SAFE", "TIMEOUT", "ALERT"),
    )
    skill, metrics = synthesize_policy_skill(skill_id="safe-policy", visible=visible, hidden=hidden)
    passed, failures, total = verify_policy_skill(skill, visible, hidden) if skill else (False, hidden, 0)
    return {
        "schema": "herus.generative_lab.policy_experiment",
        "version": 1,
        "authority": "none",
        "metrics": metrics,
        "skill": skill.canonical_dict() if skill else None,
        "verification": {
            "passed": passed,
            "total": total,
            "failures": [case.__dict__ for case in failures],
        },
        "unknown_context": {
            "accepted": False,
            "reason": "unknown_context",
        },
        "claims_not_supported": [
            "physical actuation",
            "open-world policy learning",
            "human authorization",
            "safety certification",
        ],
    }


if __name__ == "__main__":
    print(json.dumps(run(), indent=2, sort_keys=True))
