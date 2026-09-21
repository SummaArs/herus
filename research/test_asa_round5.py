from __future__ import annotations

import unittest

from research.asa_round5 import run


class AsaRound5Tests(unittest.TestCase):
    def test_structural_holdout_transfers_safe_plan(self) -> None:
        result = run()
        holdout = result["holdout"]
        self.assertEqual(holdout["accepted"], 10)
        self.assertEqual(holdout["committed"], 10)
        self.assertEqual(holdout["safe_final_states"], 10)

    def test_shortcut_reaches_goal_but_is_rejected_for_damage(self) -> None:
        counterfactual = run()["counterfactual"]
        self.assertTrue(counterfactual["shortcut_reaches_goal"])
        self.assertFalse(counterfactual["shortcut_safe"])

    def test_irreversible_action_is_not_implicitly_authorized(self) -> None:
        counterfactual = run()["counterfactual"]
        self.assertTrue(counterfactual["irreversible_available"])
        self.assertFalse(counterfactual["irreversible_in_selected_plan"])
        self.assertTrue(counterfactual["safe_plan_selected_alongside_irreversible"])
        self.assertEqual(counterfactual["irreversible_execution_state"], {
            "ready": 0,
            "committed": 0,
            "damage": 0,
            "external": 0,
        })

    def test_authority_boundary_is_explicit(self) -> None:
        self.assertEqual(
            run()["authority_boundary"],
            "proposal-only; irreversible actions require explicit contract authorization",
        )


if __name__ == "__main__":
    unittest.main()
