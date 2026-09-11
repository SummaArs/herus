import unittest

from haptic_patterns import (
    Authority,
    HapticEvent,
    HapticState,
    propose_haptic,
)


class HapticPatternTests(unittest.TestCase):
    def test_known_state_is_proposal_only(self):
        proposal = propose_haptic(
            HapticEvent(
                HapticState.CONFIRMATION,
                Authority.PROPOSAL_ONLY,
                "sha256:abc",
                "message-received",
            )
        )
        self.assertIsNotNone(proposal)
        self.assertEqual(proposal.pulses, ((90, 90),))
        self.assertEqual(proposal.authority, Authority.PROPOSAL_ONLY)

    def test_unknown_evidence_is_silent(self):
        self.assertIsNone(
            propose_haptic(
                HapticEvent(
                    HapticState.ATTENTION,
                    Authority.PROPOSAL_ONLY,
                    "",
                    "attention",
                )
            )
        )

    def test_authority_does_not_trigger_actuation(self):
        for authority in (Authority.NONE, Authority.HUMAN_BOUND):
            self.assertIsNone(
                propose_haptic(
                    HapticEvent(
                        HapticState.BLOCKED,
                        authority,
                        "sha256:abc",
                        "blocked",
                    )
                )
            )

    def test_context_is_required(self):
        self.assertIsNone(
            propose_haptic(
                HapticEvent(
                    HapticState.REFUSED,
                    Authority.PROPOSAL_ONLY,
                    "sha256:abc",
                    "   ",
                )
            )
        )

    def test_all_states_are_finite(self):
        for state in HapticState:
            proposal = propose_haptic(
                HapticEvent(state, Authority.PROPOSAL_ONLY, "sha256:abc", state.value)
            )
            self.assertIsNotNone(proposal)
            self.assertGreater(proposal.priority, 0)


if __name__ == "__main__":
    unittest.main()
