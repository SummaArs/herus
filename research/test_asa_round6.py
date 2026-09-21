from __future__ import annotations

import unittest

from research.asa_round6 import _actions, assess, run, source_contract, observe_host


class AsaRound6Tests(unittest.TestCase):
    def test_compatible_holdout_is_accepted(self) -> None:
        result = run()["scenarios"]["compatible"]
        self.assertEqual(result["runs"], 100)
        self.assertEqual(result["accepted"], 100)
        self.assertEqual(result["reasons"]["APPLICABLE"], 100)

    def test_each_drift_class_abstains_with_specific_reason(self) -> None:
        result = run()["scenarios"]
        self.assertEqual(result["drift_effect"]["accepted"], 0)
        self.assertEqual(result["drift_effect"]["reasons"]["DRIFT_EFFECT"], 100)
        self.assertEqual(result["drift_precondition"]["accepted"], 0)
        self.assertEqual(result["drift_precondition"]["reasons"]["DRIFT_PRECONDITION"], 100)
        self.assertEqual(result["drift_goal"]["accepted"], 0)
        self.assertEqual(result["drift_goal"]["reasons"]["DRIFT_GOAL"], 100)
        self.assertEqual(result["unknown_effect"]["accepted"], 0)
        self.assertEqual(result["unknown_effect"]["reasons"]["UNKNOWN_EFFECT"], 100)
        self.assertEqual(result["conflict"]["accepted"], 0)
        self.assertEqual(result["conflict"]["reasons"]["CONFLICTING_EVIDENCE"], 100)
        self.assertEqual(result["incomplete"]["accepted"], 0)
        self.assertEqual(result["incomplete"]["reasons"]["INCOMPLETE_OBSERVATION"], 100)

    def test_no_unsafe_non_abstention_in_any_scenario(self) -> None:
        for metrics in run()["scenarios"].values():
            self.assertEqual(metrics["unsafe_non_abstention"], 0)

    def test_baselines_are_reported_without_misleading_accuracy(self) -> None:
        baselines = run()["baselines"]
        self.assertEqual(baselines["blind_reuse_old_contract"]["incompatible_accepts"], 600)
        self.assertEqual(baselines["always_abstain"]["compatible_accepts"], 0)

    def test_action_renaming_and_reordering_are_metamerically_invariant(self) -> None:
        contract = source_contract()
        actions = _actions(4)
        renamed = tuple(reversed(actions))
        accepted, reason, _ = assess(contract, observe_host(renamed))
        self.assertTrue(accepted)
        self.assertEqual(reason, "APPLICABLE")


if __name__ == "__main__":
    unittest.main()
