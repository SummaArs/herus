from __future__ import annotations
import unittest

from real_symbiosis import run_real_symbiosis


class RealSymbiosisTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.result = run_real_symbiosis()

    def test_same_herus_identity_reaches_three_distinct_evidence_hosts(self) -> None:
        self.assertEqual(self.result["herus_id"], "herus-real-symbiosis-001")
        self.assertTrue(self.result["invariants"]["same_identity"])
        self.assertEqual(len(self.result["hosts"]), 3)
        self.assertEqual(len({row["host_id"] for row in self.result["hosts"]}), 3)

    def test_real_artifacts_are_distinct_and_provenance_bearing(self) -> None:
        hosts = self.result["hosts"]
        self.assertTrue(self.result["invariants"]["distinct_artifacts"])
        self.assertTrue(all(len(row["artifact_digest"]) == 64 for row in hosts))
        self.assertTrue(all(row["artifact"] for row in hosts))

    def test_all_domains_propose_but_never_execute(self) -> None:
        self.assertTrue(self.result["invariants"]["all_proposals"])
        self.assertTrue(self.result["invariants"]["all_execution_abstained"])
        self.assertEqual(self.result["invariants"]["authority_not_discovered"], True)
        self.assertTrue(self.result["invariants"]["rebind_clears_old_world"])

    def test_real_finance_artifact_keeps_forbidden_effects(self) -> None:
        finance = next(row for row in self.result["hosts"] if row["host_id"] == "ofr-observer")
        self.assertEqual(finance["facts"]["authority"], "NONE")
        self.assertIn("trade", finance["facts"]["forbidden_outputs"])
        self.assertIn("active_financial_effect", finance["facts"]["forbidden_outputs"])

    def test_real_corpus_numbers_are_not_synthetic(self) -> None:
        semantic = next(row for row in self.result["hosts"] if row["host_id"] == "semantic-gateway")
        mintsrec = next(row for row in self.result["hosts"] if row["host_id"] == "mintsrec-auditor")
        self.assertGreater(semantic["facts"]["cases"], 10)
        self.assertEqual(mintsrec["facts"]["rows"], 1779)
        self.assertEqual(mintsrec["facts"]["automatic_mapping_count"], 0)


if __name__ == "__main__":
    unittest.main()
