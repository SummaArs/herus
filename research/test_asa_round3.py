from __future__ import annotations

import unittest

from research.asa_round3 import run


class AsaRound3Tests(unittest.TestCase):
    def test_clean_replicates_recover_all_valid_events(self) -> None:
        result = run()["scenarios"]["clean_replicates"]
        self.assertEqual(result["mean_valid_recall"], 1.0)
        self.assertEqual(result["mean_selective_accuracy"], 1.0)
        self.assertEqual(result["unsafe_non_abstention"], 0)

    def test_symmetric_noise_is_recovered_by_median(self) -> None:
        result = run()["scenarios"]["symmetric_bounded_noise"]
        self.assertEqual(result["mean_valid_recall"], 1.0)
        self.assertEqual(result["mean_selective_accuracy"], 1.0)
        self.assertEqual(result["unsafe_non_abstention"], 0)

    def test_one_bad_replica_does_not_change_consensus(self) -> None:
        result = run()["scenarios"]["one_bad_replica"]
        self.assertEqual(result["mean_valid_recall"], 1.0)
        self.assertEqual(result["unsafe_non_abstention"], 0)

    def test_persistent_bias_fails_closed(self) -> None:
        for name in ("persistent_positive_bias", "persistent_negative_bias"):
            result = run()["scenarios"][name]
            self.assertEqual(result["mean_valid_recall"], 0.0)
            self.assertEqual(result["mean_selective_accuracy"], 1.0)
            self.assertEqual(result["unsafe_non_abstention"], 0)

    def test_authority_boundary_is_proposal_only(self) -> None:
        self.assertEqual(run()["authority_boundary"], "proposal-only; no external execution")


if __name__ == "__main__":
    unittest.main()
