from __future__ import annotations

import unittest

from research.asa_round4 import run


class AsaRound4Tests(unittest.TestCase):
    def test_temporal_contract_transfers_and_executes(self) -> None:
        result = run()
        self.assertTrue(result["contract_learned"])
        self.assertTrue(result["transfer"]["accepted"])
        self.assertEqual(result["transfer"]["reason"], "verified")
        self.assertTrue(result["execution"]["committed"])
        self.assertEqual(result["execution"]["final_state"]["committed"], 1)

    def test_precondition_rejects_commit_before_arm(self) -> None:
        adversarial = run()["adversarial"]
        self.assertFalse(adversarial["commit_before_arm_committed"])
        self.assertEqual(adversarial["commit_before_arm_state"], {"armed": 0, "committed": 0})

    def test_partial_failure_rolls_back_state(self) -> None:
        adversarial = run()["adversarial"]
        self.assertFalse(adversarial["partial_fault_committed"])
        self.assertEqual(adversarial["partial_fault_state"], {"armed": 0, "committed": 0})
        self.assertTrue(adversarial["partial_fault_reason"].startswith("rolled-back:"))

    def test_missing_precondition_is_not_authorized(self) -> None:
        adversarial = run()["adversarial"]
        self.assertFalse(adversarial["missing_precondition_accepted"])
        self.assertNotEqual(adversarial["missing_precondition_reason"], "verified")

    def test_proposal_boundary_is_explicit(self) -> None:
        self.assertEqual(run()["authority_boundary"], "proposal-only; transactional host execution only")


if __name__ == "__main__":
    unittest.main()
