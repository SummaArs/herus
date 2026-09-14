import unittest

from defensive_guardian import DeviceStatus
from domain_contract import Domain
from symbiosis_sandbox import (
    evaluate_bluetooth_change,
    evaluate_finance_observation,
    evaluate_robot_legacy,
)


class SymbiosisSandboxTests(unittest.TestCase):
    def test_legacy_robot_stays_in_simulation(self):
        result = evaluate_robot_legacy()
        self.assertEqual(result.domain, Domain.ROBOTICS)
        self.assertEqual(result.guardian.status, DeviceStatus.VERIFIED)
        self.assertTrue(result.effect_permitted)
        self.assertEqual(result.haptic_state, "CONFIRMATION")

    def test_bluetooth_capability_drift_blocks_active_effects(self):
        result = evaluate_bluetooth_change()
        self.assertEqual(result.guardian.status, DeviceStatus.DRIFTED)
        self.assertFalse(result.effect_permitted)
        self.assertEqual(result.haptic_state, "WAITING")

    def test_finance_is_observation_in_a_sandbox(self):
        result = evaluate_finance_observation()
        self.assertEqual(result.domain, Domain.FINANCE_SANDBOX)
        self.assertEqual(result.guardian.status, DeviceStatus.VERIFIED)
        self.assertTrue(result.effect_permitted)
        self.assertEqual(result.haptic_state, "CONFIRMATION")


if __name__ == "__main__":
    unittest.main()
