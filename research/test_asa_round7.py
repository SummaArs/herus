from __future__ import annotations

import unittest

from research.asa_round7 import run


class AsaRound7Tests(unittest.TestCase):
    def test_clean_stream_stays_active(self) -> None:
        result = run()["scenarios"]["clean"]
        self.assertEqual(result["final_state"], "ACTIVE")
        self.assertIsNone(result["abstain_started"])
        self.assertFalse(result["false_alarm"])
        self.assertEqual(result["unsafe_non_abstention"], 0)

    def test_single_transient_does_not_trigger_false_alarm(self) -> None:
        result = run()["scenarios"]["single_transient"]
        self.assertEqual(result["final_state"], "ACTIVE")
        self.assertIsNone(result["abstain_started"])
        self.assertEqual(result["proposals"], 19)

    def test_persistent_drift_triggers_after_two_bad_samples(self) -> None:
        result = run()["scenarios"]["persistent_drift"]
        self.assertEqual(result["abstain_started"], 7)
        self.assertEqual(result["unsafe_non_abstention"], 0)
        self.assertEqual(result["blind_proposals"], 12)
        self.assertLess(result["proposals"], result["blind_proposals"])

    def test_stale_reordered_and_missing_bursts_are_not_proposed(self) -> None:
        scenarios = run()["scenarios"]
        for name in ("stale_memory", "reordered_frames", "dropped_frame_burst"):
            self.assertEqual(scenarios[name]["unsafe_non_abstention"], 0)
            self.assertLess(scenarios[name]["proposals"], scenarios[name]["blind_proposals"])

    def test_recovery_requires_three_good_samples(self) -> None:
        result = run()["scenarios"]["drift_and_recovery"]
        self.assertEqual(result["abstain_started"], 6)
        self.assertEqual(result["recovered_at"], 10)
        self.assertEqual(result["unsafe_non_abstention"], 0)

    def test_authority_boundary_is_explicit(self) -> None:
        self.assertEqual(run()["authority_boundary"], "proposal-only; temporal drift disables proposals")


if __name__ == "__main__":
    unittest.main()
