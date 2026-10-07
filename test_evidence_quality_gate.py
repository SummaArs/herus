import json
import unittest
from pathlib import Path
from evidence_quality_gate import audit


class EvidenceQualityGateTests(unittest.TestCase):
    def test_current_mintrec_claims_are_blocked_for_missing_ledger_and_costs(self):
        result = audit()
        self.assertEqual(result["decision"], "BLOCK")
        self.assertIn("paired_prediction_ledger_not_archived", result["blockers"])
        self.assertIn("mintrec_costs_missing", result["blockers"])
        self.assertFalse(result["eligible_for_complete_paired_claim"])
        self.assertFalse(result["eligible_for_efficiency_claim"])

    def test_complete_fixture_can_pass_the_gate(self):
        root = Path(__file__).parent / "research" / "evidence"
        transformer = json.loads((root / "transformer_multilingual_mintrec_v1.json").read_text())
        transformer["prediction_ledger"] = [
            {"index": 0, "prediction": "A", "label": "A"},
            {"index": 1, "prediction": "B", "label": "A"},
        ]
        transformer["dataset"]["holdout_rows"] = 2
        paired = {
            "models": {
                "naive_bayes": {},
                "herus_context_memory": {},
                "distilbert_multilingual": {},
            },
            "paired_prediction_ledger": [{"index": 0}, {"index": 1}],
        }
        matrix = {"pareto": [
            {"dataset": "MIntRec S06", "infer_ms": 1.0, "train_ms": 2.0},
        ]}
        import tempfile
        with tempfile.TemporaryDirectory() as tmp:
            p = Path(tmp)
            a = p / "a.json"; b = p / "b.json"; c = p / "c.json"
            a.write_text(json.dumps(transformer)); b.write_text(json.dumps(paired)); c.write_text(json.dumps(matrix))
            result = audit(a, b, c)
        self.assertEqual(result["decision"], "READY_FOR_CLAIM_REVIEW")
        self.assertTrue(result["eligible_for_complete_paired_claim"])
        self.assertTrue(result["eligible_for_efficiency_claim"])


if __name__ == "__main__":
    unittest.main()
