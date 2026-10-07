import json
import unittest
from pathlib import Path
from paired_ledger import validate_rows


class PairedLedgerTests(unittest.TestCase):
    def setUp(self):
        data = json.loads(Path("research/evidence/mintrec_s06_identity_manifest_v1.json").read_text())
        self.records = data["records"][:2]
        self.ids = {row["example_id"]: row["label"] for row in data["records"]}

    def row(self, seed, ident, model, label=None):
        return {"seed": seed, "example_id": ident, "model": model,
                "y_true": self.ids[ident] if label is None else label,
                "prediction": self.ids[ident], "accepted": True, "score": 0.5}

    def test_empty_or_partial_ledger_blocks(self):
        result = validate_rows([] , seeds=(17,), models=("herus_full",), identity_path=Path("research/evidence/mintrec_s06_identity_manifest_v1.json"))
        self.assertEqual(result["decision"], "BLOCK")
        self.assertIn("ledger_not_complete_for_all_seeds_ids_models", result["errors"])

    def test_complete_fixture_passes_schema(self):
        data = json.loads(Path("research/evidence/mintrec_s06_identity_manifest_v1.json").read_text())
        rows = []
        for seed in (17,):
            for record in data["records"]:
                rows.append(self.row(seed, record["example_id"], "herus_full"))
        result = validate_rows(rows, seeds=(17,), models=("herus_full",))
        self.assertEqual(result["decision"], "PASS_GATE1_LEDGER")
        self.assertFalse(result["errors"])

    def test_duplicate_and_label_mismatch_fail_closed(self):
        record = self.records[0]
        rows = [self.row(17, record["example_id"], "herus_full"),
                self.row(17, record["example_id"], "herus_full", label="INVALID")]
        result = validate_rows(rows, seeds=(17,), models=("herus_full",), identity_path=Path("research/evidence/mintrec_s06_identity_manifest_v1.json"))
        self.assertEqual(result["decision"], "BLOCK")
        self.assertIn("duplicate_seed_id_model_rows", result["errors"])
        self.assertIn("label_mismatch", result["errors"])


if __name__ == "__main__":
    unittest.main()
