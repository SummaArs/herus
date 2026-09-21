from __future__ import annotations

import unittest

from research.asa_round2 import run


class AsaRound2Tests(unittest.TestCase):
    def test_clean_full_observability_has_full_coverage(self) -> None:
        result = run()["scenarios"]["full_clean"]
        self.assertEqual(result["mean_coverage"], 5 / 13)
        self.assertEqual(result["mean_valid_recall"], 1.0)
        self.assertEqual(result["unsafe_non_abstention"], 0)

    def test_partial_observability_abstains_without_unsafe_guesses(self) -> None:
        result = run()["scenarios"]["partial_one_key"]
        self.assertLess(result["mean_coverage"], 1.0)
        self.assertEqual(result["unsafe_non_abstention"], 0)
        self.assertGreater(result["safe_abstention"], 0)

    def test_noise_trades_coverage_for_safety(self) -> None:
        result = run()["scenarios"]["noisy_all_keys"]
        self.assertEqual(result["unsafe_non_abstention"], 0)
        self.assertLess(result["mean_coverage"], 1.0)
        self.assertEqual(result["mean_selective_accuracy"], 1.0)

    def test_authority_boundary_is_proposal_only(self) -> None:
        self.assertEqual(run()["authority_boundary"], "proposal-only; no external execution")


if __name__ == "__main__":
    unittest.main()
