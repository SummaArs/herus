from __future__ import annotations

import unittest

from critical_systems_health import (
    HealthState,
    Observation,
    Policy,
    classify,
    synthesize_policy,
    verify_policy,
)


class CriticalSystemsHealthTests(unittest.TestCase):
    def test_synthesis_returns_a_verified_policy(self) -> None:
        policy = synthesize_policy()
        self.assertIsNotNone(policy)
        self.assertEqual(verify_policy(policy), ())

    def test_no_actuation_without_confirmation(self) -> None:
        decision = classify(Policy(), Observation(20, 80, 10, True, False))
        self.assertEqual(decision.state, HealthState.NOMINAL)
        self.assertFalse(decision.can_actuate)
        self.assertEqual(decision.reason, "confirmation_required")

    def test_communication_loss_is_safe_hold(self) -> None:
        decision = classify(Policy(), Observation(20, 80, 10, False, True))
        self.assertEqual(decision, decision.__class__(HealthState.SAFE_HOLD, False, "communication_lost"))

    def test_invalid_sensor_is_unknown_and_inert(self) -> None:
        decision = classify(Policy(), Observation(101, 80, 10, True, True))
        self.assertEqual(decision.state, HealthState.UNKNOWN)
        self.assertFalse(decision.can_actuate)

    def test_critical_limit_is_safe_hold(self) -> None:
        decision = classify(Policy(), Observation(90, 80, 10, True, True))
        self.assertEqual(decision.state, HealthState.SAFE_HOLD)
        self.assertFalse(decision.can_actuate)

    def test_invalid_policy_is_rejected_without_action(self) -> None:
        invalid = Policy(90, 70, 30, 15, 60, 85)
        self.assertFalse(invalid.valid())
        self.assertEqual(classify(invalid, Observation(20, 80, 10, True, True)).state, HealthState.UNKNOWN)
        self.assertEqual(verify_policy(invalid), ("invalid_policy",))


if __name__ == "__main__":
    unittest.main()
