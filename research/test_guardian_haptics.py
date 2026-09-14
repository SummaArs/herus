import unittest

from defensive_guardian import DeviceStatus, GuardianDecision
from guardian_haptics import haptic_for_guardian
from haptic_patterns import Authority, HapticState


class GuardianHapticsTests(unittest.TestCase):
    def decision(self, status):
        return GuardianDecision("sensor-1", status, "test", status is not DeviceStatus.VERIFIED)

    def test_verified_device_produces_confirmation_proposal(self):
        proposal = haptic_for_guardian(self.decision(DeviceStatus.VERIFIED), "digest")
        self.assertEqual(proposal.state, HapticState.CONFIRMATION)
        self.assertEqual(proposal.authority, Authority.PROPOSAL_ONLY)

    def test_drift_waits_for_renegotiation(self):
        proposal = haptic_for_guardian(self.decision(DeviceStatus.DRIFTED), "digest")
        self.assertEqual(proposal.state, HapticState.WAITING)

    def test_quarantine_is_blocked(self):
        proposal = haptic_for_guardian(self.decision(DeviceStatus.QUARANTINED), "digest")
        self.assertEqual(proposal.state, HapticState.BLOCKED)

    def test_revocation_is_refused(self):
        proposal = haptic_for_guardian(self.decision(DeviceStatus.REVOKED), "digest")
        self.assertEqual(proposal.state, HapticState.REFUSED)

    def test_missing_evidence_produces_no_actuation_proposal(self):
        proposal = haptic_for_guardian(self.decision(DeviceStatus.QUARANTINED), "")
        self.assertIsNone(proposal)


if __name__ == "__main__":
    unittest.main()
