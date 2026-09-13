import unittest

from symbiosis_modes import (
    PermissionDecision,
    PermissionRequest,
    SymbiosisMode,
    decide_permission,
)


class SymbiosisModeTests(unittest.TestCase):
    def request(self, **overrides):
        values = dict(
            mode=SymbiosisMode.HOST,
            host_id="host-1",
            channel="USB",
            effect="MOVE_MOTOR",
            evidence_digest="digest",
            human_confirmation=False,
        )
        values.update(overrides)
        return PermissionRequest(**values)

    def test_connection_without_confirmation_is_proposal_only(self):
        self.assertEqual(decide_permission(self.request()), PermissionDecision.PROPOSE)

    def test_unknown_effect_is_not_executed(self):
        self.assertEqual(
            decide_permission(self.request(effect="UNKNOWN", human_confirmation=True)),
            PermissionDecision.PROPOSE,
        )

    def test_missing_evidence_denies(self):
        self.assertEqual(
            decide_permission(self.request(evidence_digest="")),
            PermissionDecision.DENY,
        )

    def test_defensive_isolation_requires_confirmation(self):
        request = self.request(
            mode=SymbiosisMode.DEFENSIVE_MESH,
            effect="ISOLATE_DEVICE",
            human_confirmation=True,
        )
        self.assertEqual(decide_permission(request), PermissionDecision.ALLOW_ONCE)

    def test_personal_haptic_can_be_session_scoped(self):
        request = self.request(
            mode=SymbiosisMode.PERSONAL_GUARDIAN,
            effect="HAPTIC_FEEDBACK",
            human_confirmation=True,
        )
        self.assertEqual(decide_permission(request), PermissionDecision.ALLOW_SESSION)


if __name__ == "__main__":
    unittest.main()
