from __future__ import annotations

import inspect
import unittest

from research.asa_round9 import infer_public, run


class AsaRound9Tests(unittest.TestCase):
    def test_inference_function_has_no_oracle_channel(self) -> None:
        source = inspect.getsource(infer_public)
        self.assertNotIn("expected", source)
        self.assertNotIn("oracle", source)
        self.assertNotIn("opaque", source)

    def test_holdout_is_locally_evaluable_and_fail_closed(self) -> None:
        result = run()
        holdout = result["holdout"]
        self.assertEqual(holdout["cases"], 10)
        self.assertEqual(holdout["correct"], 10)
        self.assertEqual(holdout["unsafe_non_abstention"], 0)
        self.assertEqual(holdout["safe_abstention"], 3)
        self.assertEqual(holdout["valid_recall"], 1.0)

    def test_regression_corpus_has_no_unsafe_non_abstention(self) -> None:
        regression = run()["regression"]
        self.assertEqual(regression["cases"], 13)
        self.assertEqual(regression["unsafe_non_abstention"], 0)
        self.assertEqual(regression["correct"], 13)

    def test_holdout_is_not_claimed_as_external_data(self) -> None:
        provenance = run()["provenance"]
        self.assertTrue(provenance["regression_not_production_telemetry"])
        self.assertTrue(provenance["holdout_not_production_telemetry"])
        self.assertFalse(provenance["external_generalization_estimable"])

    def test_deterministic_output(self) -> None:
        first = run()
        second = run()
        self.assertEqual(first, second)

    def test_baseline_and_boundaries_are_explicit(self) -> None:
        result = run()
        self.assertEqual(result["baselines"]["always_abstain"]["proposals"], 0)
        self.assertIn("evaluator", result["oracle_boundary"])
        self.assertEqual(result["authority_boundary"], "proposal-only; no external execution")


if __name__ == "__main__":
    unittest.main()
