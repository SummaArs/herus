from __future__ import annotations
import unittest

from h0_host_runtime import H0ComputerResidence
from host_profile import HostProfile
from symbiotic_models import WorldObservation


class H0HostRuntimeTests(unittest.TestCase):
    def profile(self, host_id: str) -> HostProfile:
        return HostProfile(
            host_id=host_id, revision="computer", resources={"ram_bytes": 1_000_000},
            interfaces=frozenset({"button", "haptic", "radio"}), constraints={"max_steps": 100},
            representation_set=frozenset({"SIM-INT8", "SIM-HDC8", "SIM-RULES"}),
            skill_budget={"bytes": 1_000_000, "steps": 100}, evidence={"host": "computer"},
        )

    def test_computer_is_residence_and_exit_preserves_identity_only(self):
        runtime = H0ComputerResidence("herus-001")
        runtime.enter(self.profile("computer"), skills={"observe", "remember"})
        runtime.observe(WorldObservation("file", "exists", "true", "filesystem"))
        bundle = runtime.prepare_exit()
        self.assertFalse(runtime.resident)
        self.assertEqual(bundle.herus_id, "herus-001")
        self.assertEqual(bundle.authority, "NONE")
        self.assertTrue(bundle.transferable_skills)
        self.assertFalse(bundle.world_context_transferred)
        self.assertFalse(bundle.secrets_transferred)
        self.assertFalse(bundle.execution_authority_transferred)
        self.assertEqual(runtime.validate_exit(bundle), ())

    def test_exit_cannot_be_forged_into_authority_or_context_transfer(self):
        runtime = H0ComputerResidence()
        runtime.enter(self.profile("computer"))
        bundle = runtime.prepare_exit()
        forged = bundle.__class__(**{**bundle.canonical(), "authority": "EXECUTE", "world_context_transferred": True})
        self.assertIn("exit_authority_escalation", runtime.validate_exit(forged))
        self.assertIn("exit_forbidden_state_transferred", runtime.validate_exit(forged))

    def test_cannot_enter_twice_without_exit(self):
        runtime = H0ComputerResidence()
        runtime.enter(self.profile("computer"))
        with self.assertRaises(RuntimeError):
            runtime.enter(self.profile("computer-2"))


if __name__ == "__main__":
    unittest.main()
