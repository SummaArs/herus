import json
import unittest

from critical_runtime_guardian import (
    Event,
    GuardianError,
    InvariantConfig,
    Observation,
    RuntimeGuardian,
    Severity,
    State,
    calculate_risk,
)


class CriticalRuntimeGuardianTests(unittest.TestCase):
    def _guardian(self) -> RuntimeGuardian:
        return RuntimeGuardian([
            (Event.AUTHORITY_VIOLATION, InvariantConfig(True, True, 1000)),
            (Event.REPLAY_DETECTED, InvariantConfig(True, True, 1000)),
            (Event.CAPACITY_EXHAUSTED, InvariantConfig(False, False, 0)),
        ])

    def test_critical_observation_blocks_and_alerts(self) -> None:
        guardian = self._guardian()
        alert = guardian.observe(Observation(Event.AUTHORITY_VIOLATION, Severity.CRITICAL, 100, 42))
        self.assertTrue(alert)
        self.assertTrue(guardian.is_blocked(42))
        self.assertEqual(guardian.state, State.ALERTING)

    def test_acknowledges_but_never_releases_block(self) -> None:
        guardian = self._guardian()
        guardian.observe(Observation(Event.REPLAY_DETECTED, Severity.CRITICAL, 100, 7))
        guardian.human_decision(7, 1)
        self.assertEqual(guardian.state, State.MONITORING)
        self.assertTrue(guardian.is_blocked(7))

    def test_cooldown_suppresses_repeated_alert_only(self) -> None:
        guardian = self._guardian()
        self.assertTrue(guardian.observe(Observation(Event.AUTHORITY_VIOLATION, Severity.CRITICAL, 100, 1)))
        self.assertFalse(guardian.observe(Observation(Event.AUTHORITY_VIOLATION, Severity.CRITICAL, 500, 2)))
        self.assertTrue(guardian.is_blocked(2))
        self.assertTrue(guardian.observe(Observation(Event.AUTHORITY_VIOLATION, Severity.CRITICAL, 1100, 3)))

    def test_invalid_input_is_rejected(self) -> None:
        guardian = self._guardian()
        with self.assertRaises(GuardianError):
            guardian.observe(Observation(Event.AUTHORITY_VIOLATION, Severity.CRITICAL, -1, 1))
        with self.assertRaises(GuardianError):
            calculate_risk(Observation(Event.AUTHORITY_VIOLATION, Severity.CRITICAL, 0, 1), {"safety": 11})

    def test_capacity_exhaustion_is_not_pass(self) -> None:
        guardian = RuntimeGuardian([(Event.AUTHORITY_VIOLATION, InvariantConfig(True, False))])
        for correlation in range(guardian.MAX_BLOCKED_ACTIONS):
            guardian.observe(Observation(Event.AUTHORITY_VIOLATION, Severity.CRITICAL, 1, correlation))
        with self.assertRaises(GuardianError):
            guardian.observe(Observation(Event.AUTHORITY_VIOLATION, Severity.CRITICAL, 2, 99))
        self.assertEqual(guardian.state, State.DEGRADED)

    def test_snapshot_is_deterministic_and_typed(self) -> None:
        guardian = self._guardian()
        guardian.observe(Observation(Event.AUTHORITY_VIOLATION, Severity.CRITICAL, 100, 42, "asset-a"))
        first = guardian.evidence_snapshot()
        second = guardian.evidence_snapshot()
        self.assertEqual(first, second)
        record = json.loads(first)
        self.assertEqual(record["schema"], "herus.runtime_guardian.evidence.v1")
        self.assertEqual(record["observations"][0]["event"], "AUTHORITY_VIOLATION")

    def test_risk_escalates_with_safety_baseline(self) -> None:
        obs = Observation(Event.INVARIANT_BREACH, Severity.ELEVATED, 0, 1)
        normal = calculate_risk(obs)
        critical = calculate_risk(obs, {"safety": 8})
        self.assertEqual(normal.severity, Severity.ELEVATED)
        self.assertEqual(critical.severity, Severity.CRITICAL)
        self.assertTrue(critical.auto_block)


if __name__ == "__main__":
    unittest.main()
