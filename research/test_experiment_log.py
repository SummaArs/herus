import unittest

from experiment_log import make_record


class ExperimentLogTests(unittest.TestCase):
    def test_record_has_deterministic_digest(self):
        kwargs = dict(
            session_id="s",
            sequence=0,
            hypothesis="unknown interface",
            probe="has_interface",
            argument="haptic",
            expected_information=2,
            estimated_bytes=96,
            observed_value="false",
            outcome="OBSERVED",
        )
        first = make_record(**kwargs)
        second = make_record(**kwargs)
        self.assertEqual(first.digest, second.digest)

    def test_abstention_requires_reason(self):
        with self.assertRaisesRegex(ValueError, "abstention_reason_required"):
            make_record(
                session_id="s",
                sequence=0,
                hypothesis="unknown",
                probe="has_interface",
                argument="haptic",
                expected_information=2,
                estimated_bytes=96,
                observed_value=None,
                outcome="ABSTAIN",
            )

    def test_invalid_budget_is_rejected(self):
        with self.assertRaisesRegex(ValueError, "experiment_budget_invalid"):
            make_record(
                session_id="s",
                sequence=0,
                hypothesis="unknown",
                probe="has_interface",
                argument="haptic",
                expected_information=2,
                estimated_bytes=0,
                observed_value=None,
                outcome="ABSTAIN",
                abstention_reason="budget",
            )


if __name__ == "__main__":
    unittest.main()
