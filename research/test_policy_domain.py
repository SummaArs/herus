from __future__ import annotations

import unittest

from generative_lab.policy_domain import PolicyCase, run_policy, synthesize_policy_skill, verify_policy_skill


class PolicyDomainTests(unittest.TestCase):
    def test_unknown_context_fails_closed(self) -> None:
        result, reason = run_policy(("SIGNAL:OK=>WAIT", "SIGNAL:TIMEOUT=>ALERT", "SIGNAL:CANCEL=>SAFE"), "UNKNOWN", "OK")
        self.assertIsNone(result)
        self.assertEqual(reason, "unknown_context")

    def test_non_total_and_duplicate_policies_are_rejected(self) -> None:
        incomplete, reason = run_policy(("SIGNAL:OK=>WAIT",), "SAFE", "OK")
        self.assertIsNone(incomplete)
        self.assertEqual(reason, "non_total_policy")
        duplicate, reason = run_policy(("SIGNAL:OK=>WAIT", "SIGNAL:OK=>SAFE", "SIGNAL:CANCEL=>SAFE"), "SAFE", "OK")
        self.assertIsNone(duplicate)
        self.assertEqual(reason, "duplicate_rule")

    def test_synthesis_generalizes_policy_to_hidden_states(self) -> None:
        visible = (
            PolicyCase("SAFE", "OK", "WAIT"),
            PolicyCase("WAIT", "TIMEOUT", "ALERT"),
        )
        hidden = (
            PolicyCase("ALERT", "CANCEL", "SAFE"),
            PolicyCase("SAFE", "TIMEOUT", "ALERT"),
        )
        skill, metrics = synthesize_policy_skill(skill_id="safe-policy", visible=visible, hidden=hidden)
        self.assertIsNotNone(skill)
        passed, failures, total = verify_policy_skill(skill, visible, hidden)
        self.assertTrue(passed)
        self.assertEqual(failures, ())
        self.assertEqual(total, 4)
        self.assertGreater(metrics["rejected"], 0)


if __name__ == "__main__":
    unittest.main()
