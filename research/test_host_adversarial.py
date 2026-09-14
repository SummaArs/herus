from __future__ import annotations

import unittest

from host_adversarial import evaluate_profiles, hostile_profiles
from host_profile import discover_profile
from host_skill_selector import select_for_host
from generative_lab.skills import SkillState, candidate_skill


class HostAdversarialTests(unittest.TestCase):
    def test_malformed_or_unusable_profiles_are_blocked(self) -> None:
        results = evaluate_profiles()
        self.assertEqual(results["missing_haptic"], "ACCEPTED")
        self.assertEqual(results["negative_resource"], "BLOCKED")
        self.assertEqual(results["unknown_interface"], "BLOCKED")
        self.assertEqual(results["no_representation"], "BLOCKED")

    def test_missing_interface_blocks_dependent_skill(self) -> None:
        profile = discover_profile(hostile_profiles()["missing_haptic"])
        skill = candidate_skill(
            skill_id="haptic.alert", input_type="Context", output_type="Decision",
            program=("ALERT",), provenance={"source": "test"},
            resource_budget={"bytes": 8, "steps": 1, "depth": 1},
        ).with_state(SkillState.VERIFIED)
        selected = select_for_host(profile, skill, "herus-wire-v1", frozenset({"haptic"}))
        self.assertEqual((selected.status, selected.reason), ("BLOCKED", "interface_unavailable"))


if __name__ == "__main__":
    unittest.main()
