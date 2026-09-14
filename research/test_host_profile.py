from __future__ import annotations

import unittest

from host_profile import HostProfile, discover_profile, validate_profile


class HostProfileTests(unittest.TestCase):
    def raw(self) -> dict[str, object]:
        return {
            "host_id": "pulse-001",
            "revision": "t3s3-v1",
            "resources": {"ram_bytes": 512000, "energy_uj": 1000},
            "interfaces": ["button", "haptic", "radio"],
            "constraints": {"max_skill_bytes": 64, "max_steps": 3},
            "representation_set": ["herus-wire-v1", "proposal-only"],
            "skill_budget": {"bytes": 64, "steps": 3, "depth": 2},
            "evidence": {"source": "declared-host-profile", "revision_digest": "abc"},
        }

    def test_discovery_forces_none_authority(self) -> None:
        raw = self.raw()
        raw["authority"] = "HUMAN_BOUND"
        profile = discover_profile(raw)
        self.assertIsNotNone(profile)
        assert profile is not None
        self.assertEqual(profile.authority, "NONE")
        self.assertEqual(validate_profile(profile), ())

    def test_unknown_interface_is_refused(self) -> None:
        raw = self.raw()
        raw["interfaces"] = ["button", "telepathy"]
        self.assertIsNone(discover_profile(raw))

    def test_negative_budget_is_refused(self) -> None:
        raw = self.raw()
        raw["skill_budget"] = {"bytes": -1, "steps": 3}
        self.assertIsNone(discover_profile(raw))

    def test_digest_is_deterministic(self) -> None:
        first = discover_profile(self.raw())
        second = discover_profile(self.raw())
        self.assertIsNotNone(first)
        self.assertEqual(first.digest(), second.digest())  # type: ignore[union-attr]

    def test_unknown_authority_is_invalid_for_manual_profile(self) -> None:
        profile = HostProfile(
            host_id="x", revision="r", resources={"ram": 1},
            interfaces=frozenset({"button"}), constraints={"steps": 1},
            representation_set=frozenset({"wire"}), skill_budget={"bytes": 1},
            evidence={"source": "test"}, authority="EXECUTE_ANYTHING",
        )
        self.assertIn("authority_unknown", validate_profile(profile))


if __name__ == "__main__":
    unittest.main()
