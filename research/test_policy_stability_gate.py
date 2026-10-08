import unittest
from policy_stability_gate import evaluate


class PolicyStabilityGateTests(unittest.TestCase):
    def test_current_three_hosts_do_not_promote_calibration(self):
        rows = [
            {"dataset": "MIntRec", "validation_accuracy": {"score_calibrated": .443396, "universal_default": .336478}},
            {"dataset": "MInDS-14", "validation_accuracy": {"score_calibrated": .844444, "universal_default": .844444}},
            {"dataset": "Banking77", "validation_accuracy": {"score_calibrated": .7255, "universal_default": .7215}},
        ]
        decision = evaluate(rows, margin=.01, min_hosts=3)
        self.assertEqual(decision.status, "ABSTAIN")
        self.assertEqual(decision.policy, "universal_default")
        self.assertEqual(decision.passing_hosts, 1)

    def test_clear_consistent_gain_promotes(self):
        rows = [{"dataset": name, "validation_accuracy": {"score_calibrated": .80, "universal_default": .70}} for name in ("a", "b", "c")]
        decision = evaluate(rows, margin=.05)
        self.assertEqual(decision.status, "PROMOTE")
        self.assertEqual(decision.policy, "score_calibrated")

    def test_missing_host_evidence_blocks(self):
        decision = evaluate([{"dataset": "a"}], min_hosts=2)
        self.assertEqual(decision.status, "BLOCKED")

    def test_negative_margin_is_rejected(self):
        with self.assertRaises(ValueError):
            evaluate([], margin=-.1)


if __name__ == "__main__":
    unittest.main()
