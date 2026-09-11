from __future__ import annotations

import unittest

from host_profile import discover_profile
from host_negotiation import negotiate


class HostNegotiationTests(unittest.TestCase):
    def profile(self):
        return discover_profile({
            "host_id": "pulse-001",
            "revision": "t3s3-v1",
            "resources": {"ram_bytes": 512000, "energy_uj": 1000},
            "interfaces": ["button", "haptic", "radio"],
            "constraints": {"max_skill_bytes": 64, "max_steps": 3},
            "representation_set": ["herus-wire-v1", "proposal-only"],
            "skill_budget": {"bytes": 64, "steps": 3, "depth": 2},
            "evidence": {"source": "declared-host-profile", "revision_digest": "abc"},
        })

    def test_compatible_plan_is_proposal_only(self) -> None:
        plan = negotiate(self.profile(), "herus-wire-v1", 10, 2, 1)
        self.assertEqual(plan.status, "PROPOSAL_ONLY")
        self.assertEqual(plan.reason, "compatible")
        self.assertEqual(plan.effects, frozenset())

    def test_missing_representation_blocks(self) -> None:
        plan = negotiate(self.profile(), "unknown-wire", 10, 2, 1)
        self.assertEqual((plan.status, plan.reason), ("BLOCKED", "representation_unavailable"))

    def test_budget_blocks(self) -> None:
        plan = negotiate(self.profile(), "herus-wire-v1", 65, 2, 1)
        self.assertEqual((plan.status, plan.reason), ("BLOCKED", "skill_bytes_exceeded"))

    def test_effects_never_come_from_discovery(self) -> None:
        plan = negotiate(self.profile(), "herus-wire-v1", 10, 2, 1, {"EXECUTE_ACTUATOR"})
        self.assertEqual((plan.status, plan.reason), ("BLOCKED", "effects_require_explicit_binding"))


if __name__ == "__main__":
    unittest.main()
