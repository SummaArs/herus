import json
import unittest
from pathlib import Path
from cross_domain_gate import audit


class CrossDomainGateTests(unittest.TestCase):
    def test_current_minds14_evidence_is_not_promoted(self):
        path = Path(__file__).parent / "evidence" / "minds14_real_benchmark_v1.json"
        result = audit(json.loads(path.read_text(encoding="utf-8")))
        self.assertFalse(result["eligible_for_cross_domain_claim"])
        self.assertEqual(result["decision"], "BLOCK")
        self.assertIn("speaker_independent_split_missing", result["blockers"])
        self.assertIn("label_contract_missing", result["blockers"])

    def test_complete_contract_can_reach_review(self):
        evidence = {
            "dataset": {"id": "independent", "holdout": 10},
            "methods": {"consensus_selective": {}},
            "limits": [],
        }
        result = audit(evidence)
        self.assertTrue(result["eligible_for_cross_domain_claim"])
        self.assertEqual(result["decision"], "ELIGIBLE_FOR_REVIEW")


if __name__ == "__main__":
    unittest.main()
