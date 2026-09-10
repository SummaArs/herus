from __future__ import annotations

import unittest

from generative_lab.policy_composition import compose_policy_skills, verify_composed_policy
from generative_lab.policy_domain import PolicyCase
from generative_lab.skills import Skill, SkillState


class PolicyCompositionTests(unittest.TestCase):
    def setUp(self) -> None:
        self.first = Skill(
            skill_id="signal-policy",
            version=1,
            input_type="Context",
            output_type="Action",
            preconditions=(),
            postconditions=(),
            program=("SIGNAL:OK=>WAIT", "SIGNAL:TIMEOUT=>ALERT", "SIGNAL:CANCEL=>SAFE"),
            dependencies=(),
            resource_budget={"steps": 3, "memory": 3},
            allowed_effects=(),
            forbidden_effects=("EXECUTE_ACTUATOR",),
            determinism="deterministic",
            provenance={"kind": "fixture"},
            state=SkillState.VERIFIED,
            evidence_hash="first-evidence",
            generalization_score=1.0,
            reliability=1.0,
            usage_count=0,
        )
        self.second = Skill(
            skill_id="action-policy",
            version=1,
            input_type="Action",
            output_type="Decision",
            preconditions=(),
            postconditions=(),
            program=("ACTION:WAIT=>WAIT", "ACTION:ALERT=>ALERT", "ACTION:SAFE=>SAFE"),
            dependencies=(),
            resource_budget={"steps": 3, "memory": 3},
            allowed_effects=(),
            forbidden_effects=("EXECUTE_ACTUATOR",),
            determinism="deterministic",
            provenance={"kind": "fixture"},
            state=SkillState.VERIFIED,
            evidence_hash="second-evidence",
            generalization_score=1.0,
            reliability=1.0,
            usage_count=0,
        )

    def test_composed_policy_is_candidate_and_verifiable(self) -> None:
        composed = compose_policy_skills(self.first, self.second, skill_id="composed-policy")
        self.assertEqual(composed.state, SkillState.CANDIDATE)
        self.assertEqual(composed.dependencies, ("signal-policy@1", "action-policy@1"))
        cases = (
            PolicyCase("SAFE", "OK", "WAIT"),
            PolicyCase("WAIT", "TIMEOUT", "ALERT"),
            PolicyCase("ALERT", "CANCEL", "SAFE"),
        )
        passed, failures = verify_composed_policy(composed, cases)
        self.assertTrue(passed)
        self.assertEqual(failures, ())

    def test_unverified_dependency_is_refused(self) -> None:
        unverified = Skill(**{**self.first.__dict__, "state": SkillState.CANDIDATE})
        with self.assertRaises(ValueError):
            compose_policy_skills(unverified, self.second, skill_id="refused")


if __name__ == "__main__":
    unittest.main()
