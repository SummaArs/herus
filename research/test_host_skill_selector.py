from __future__ import annotations

import unittest

from host_profile import discover_profile
from host_skill_selector import select_for_host
from generative_lab.skills import SkillState, candidate_skill


class HostSkillSelectorTests(unittest.TestCase):
    def profile(self):
        return discover_profile({
            "host_id": "pulse-001",
            "revision": "t3s3-v1",
            "resources": {"ram_bytes": 512000},
            "interfaces": ["button", "haptic", "radio"],
            "constraints": {"max_skill_bytes": 64, "max_steps": 3},
            "representation_set": ["herus-wire-v1"],
            "skill_budget": {"bytes": 64, "steps": 3, "depth": 2},
            "evidence": {"source": "test", "revision_digest": "abc"},
        })

    def skill(self):
        return candidate_skill(
            skill_id="policy.confirm",
            input_type="Context",
            output_type="Decision",
            program=("IF_SAFE", "THEN_CONFIRM"),
            provenance={"source": "test"},
            resource_budget={"bytes": 32, "steps": 2, "depth": 1},
        )

    def test_verified_compatible_skill_is_proposal_only(self) -> None:
        skill = self.skill().with_state(SkillState.QUARANTINED).with_state(SkillState.TESTED).with_state(SkillState.VERIFIED)
        selected = select_for_host(self.profile(), skill, "herus-wire-v1")
        self.assertEqual((selected.status, selected.reason), ("PROPOSAL_ONLY", "verified_and_compatible"))

    def test_unverified_skill_is_blocked(self) -> None:
        selected = select_for_host(self.profile(), self.skill(), "herus-wire-v1")
        self.assertEqual((selected.status, selected.reason), ("BLOCKED", "skill_not_verified"))

    def test_incompatible_representation_is_blocked(self) -> None:
        skill = self.skill().with_state(SkillState.VERIFIED)
        selected = select_for_host(self.profile(), skill, "missing-wire")
        self.assertEqual((selected.status, selected.reason), ("BLOCKED", "representation_unavailable"))

    def test_authorized_effect_is_blocked_even_if_verified(self) -> None:
        skill = self.skill().with_state(SkillState.VERIFIED)
        object.__setattr__(skill, "allowed_effects", ("EXECUTE_ACTUATOR",))
        selected = select_for_host(self.profile(), skill, "herus-wire-v1")
        self.assertEqual((selected.status, selected.reason), ("BLOCKED", "skill_has_effects"))


if __name__ == "__main__":
    unittest.main()
