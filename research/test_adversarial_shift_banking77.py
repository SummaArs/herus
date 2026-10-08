import json
import unittest
from pathlib import Path


class AdversarialShiftEvidenceTests(unittest.TestCase):
    def test_evidence_is_holdout_only_and_reports_tradeoff(self):
        path = Path(__file__).parent / "evidence" / "adversarial_shift_banking77_v1.json"
        data = json.loads(path.read_text())
        self.assertFalse(data["detector"]["holdout_labels_used"])
        self.assertEqual(data["detector"]["calibration_examples"], 1519)
        self.assertEqual(set(data["attacks"]), {"case_punctuation", "typo", "deletion", "irrelevant_prefix"})
        for row in data["attacks"].values():
            self.assertGreater(row["detector_abstention_rate"], 0)
            self.assertLess(row["coverage"], 1)
            self.assertIn("claim_boundary", data["protocol"])


if __name__ == "__main__":
    unittest.main()
