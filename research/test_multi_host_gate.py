import unittest
from research.multi_host_gate import decision, validate


class MultiHostGateTests(unittest.TestCase):
    def test_blocks_fixture_data_and_missing_hosts(self):
        ok, errors = validate({"schema": "herus-multi-host-gate-v1", "data_origin": "fixture", "hosts": []})
        self.assertFalse(ok)
        self.assertIn("real_world_data_required", errors)
        self.assertIn("at_least_three_hosts_required", errors)

    def test_blocks_leakage_and_insufficient_seeds(self):
        host = {
            "host_id": "a", "fit_count": 1, "holdout_count": 1,
            "target_feedback_count": 1, "retained_evidence": 1,
            "quarantined_evidence": 0, "leakage_detected": True,
            "seeds": [1], "herus_score": 1.0, "baseline_score": 0.5,
            "score_higher_is_better": True,
        }
        ok, errors = validate({"schema": "herus-multi-host-gate-v1", "data_origin": "real_world", "dataset_id": "x", "hosts": [host, {**host, "host_id": "b"}, {**host, "host_id": "c"}]})
        self.assertFalse(ok)
        self.assertTrue(any("leakage_detected" in error for error in errors))
        self.assertTrue(any("at_least_three_seeds_required" in error for error in errors))

    def test_valid_protocol_is_not_sota_claim(self):
        host = {
            "host_id": "a", "fit_count": 1, "holdout_count": 1,
            "target_feedback_count": 1, "retained_evidence": 1,
            "quarantined_evidence": 1, "leakage_detected": False,
            "seeds": [1, 2, 3], "herus_score": 1.0, "baseline_score": 0.5,
            "score_higher_is_better": True,
        }
        payload = {"schema": "herus-multi-host-gate-v1", "data_origin": "real_world", "dataset_id": "real-dataset", "hosts": [host, {**host, "host_id": "b"}, {**host, "host_id": "c"}]}
        result = decision(payload)
        self.assertEqual(result["status"], "READY_FOR_ANALYSIS")
        self.assertFalse(result["claim_allowed"])


if __name__ == "__main__":
    unittest.main()
