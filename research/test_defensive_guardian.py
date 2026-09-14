import unittest

from defensive_guardian import (
    DefensiveGuardian,
    DeviceObservation,
    DeviceStatus,
)


class DefensiveGuardianTests(unittest.TestCase):
    def observation(self, **overrides):
        values = dict(
            device_id="sensor-1",
            channel="BLE",
            identity_digest="id-a",
            capability_digest="caps-a",
            authorized=True,
        )
        values.update(overrides)
        return DeviceObservation(**values)

    def test_first_authorized_observation_is_verified(self):
        decision = DefensiveGuardian().observe(self.observation())
        self.assertEqual(decision.status, DeviceStatus.VERIFIED)
        self.assertFalse(decision.requires_human_confirmation)

    def test_unknown_device_is_quarantined(self):
        decision = DefensiveGuardian().observe(self.observation(authorized=False))
        self.assertEqual(decision.status, DeviceStatus.QUARANTINED)
        self.assertTrue(decision.requires_human_confirmation)

    def test_identity_change_is_more_severe_than_capability_drift(self):
        guardian = DefensiveGuardian()
        guardian.observe(self.observation())
        identity = guardian.observe(self.observation(identity_digest="id-b"))
        self.assertEqual(identity.status, DeviceStatus.QUARANTINED)
        self.assertEqual(identity.reason, "identity_changed")

    def test_capability_drift_requires_renegotiation(self):
        guardian = DefensiveGuardian()
        guardian.observe(self.observation())
        drift = guardian.observe(self.observation(capability_digest="caps-b"))
        self.assertEqual(drift.status, DeviceStatus.DRIFTED)
        self.assertTrue(drift.requires_human_confirmation)

    def test_revocation_survives_reobservation(self):
        guardian = DefensiveGuardian()
        guardian.observe(self.observation())
        guardian.revoke("sensor-1")
        decision = guardian.observe(self.observation())
        self.assertEqual(decision.status, DeviceStatus.REVOKED)
        self.assertEqual(decision.reason, "previously_restricted")


if __name__ == "__main__":
    unittest.main()
