from __future__ import annotations

import unittest

from critical_state_verifier import (
    Certificate,
    PolicyRule,
    StateMachineSpec,
    Transition,
    Verdict,
    verify,
)


class CriticalStateVerifierTests(unittest.TestCase):
    def setUp(self) -> None:
        self.spec = StateMachineSpec(
            initial_state="nominal",
            states=frozenset({"nominal", "safe_hold", "unsafe"}),
            inputs=frozenset({"ok", "loss"}),
            actions=frozenset({"hold", "safe_hold", "actuate"}),
            transitions=(
                Transition("nominal", "ok", "hold", "nominal"),
                Transition("nominal", "loss", "safe_hold", "safe_hold"),
                Transition("safe_hold", "ok", "safe_hold", "safe_hold"),
                Transition("safe_hold", "loss", "safe_hold", "safe_hold"),
                Transition("nominal", "ok", "actuate", "unsafe"),
            ),
            forbidden_states=frozenset({"unsafe"}),
            forbidden_actions=frozenset({"actuate"}),
            max_steps=2,
        )

    def test_valid_policy_is_verified(self) -> None:
        policy = (
            PolicyRule("nominal", "ok", "hold"),
            PolicyRule("nominal", "loss", "safe_hold"),
            PolicyRule("safe_hold", "ok", "safe_hold"),
            PolicyRule("safe_hold", "loss", "safe_hold"),
        )
        result = verify(self.spec, policy)
        self.assertEqual(result.verdict, Verdict.VERIFIED)
        self.assertGreater(result.explored, 0)
        self.assertEqual(result.reason, "all_bounded_traces_safe")

    def test_forbidden_action_returns_counterexample(self) -> None:
        policy = (
            PolicyRule("nominal", "ok", "actuate"),
            PolicyRule("nominal", "loss", "safe_hold"),
            PolicyRule("safe_hold", "ok", "safe_hold"),
            PolicyRule("safe_hold", "loss", "safe_hold"),
        )
        result = verify(self.spec, policy)
        self.assertEqual(result.verdict, Verdict.COUNTEREXAMPLE)
        self.assertEqual(result.reason, "forbidden_action")
        self.assertEqual(result.path, ())

    def test_unmodeled_transition_is_unknown_not_verified(self) -> None:
        policy = (
            PolicyRule("nominal", "ok", "hold"),
            PolicyRule("nominal", "loss", "safe_hold"),
            PolicyRule("safe_hold", "ok", "hold"),
            PolicyRule("safe_hold", "loss", "safe_hold"),
        )
        result = verify(self.spec, policy)
        self.assertEqual(result.verdict, Verdict.UNKNOWN)
        self.assertEqual(result.reason, "unmodeled_transition")

    def test_missing_policy_rule_is_unknown(self) -> None:
        result = verify(self.spec, (PolicyRule("nominal", "ok", "hold"),))
        self.assertEqual(result.verdict, Verdict.UNKNOWN)
        self.assertEqual(result.reason, "missing_policy_rule")

    def test_invalid_specification_is_rejected(self) -> None:
        invalid = StateMachineSpec(
            initial_state="missing",
            states=frozenset({"nominal"}),
            inputs=frozenset({"ok"}),
            actions=frozenset({"hold"}),
            transitions=(),
            forbidden_states=frozenset(),
            forbidden_actions=frozenset(),
            max_steps=1,
        )
        result = verify(invalid, ())
        self.assertEqual(result.verdict, Verdict.INVALID_SPEC)

    def test_duplicate_policy_rule_is_rejected(self) -> None:
        rule = PolicyRule("nominal", "ok", "hold")
        result = verify(self.spec, (rule, rule))
        self.assertEqual(result.verdict, Verdict.INVALID_SPEC)
        self.assertEqual(result.reason, "duplicate_policy_rule")

    def test_negative_bound_is_rejected(self) -> None:
        invalid = StateMachineSpec(
            initial_state="nominal",
            states=frozenset({"nominal"}),
            inputs=frozenset({"ok"}),
            actions=frozenset({"hold"}),
            transitions=(),
            forbidden_states=frozenset(),
            forbidden_actions=frozenset(),
            max_steps=-1,
        )
        result = verify(invalid, ())
        self.assertEqual(result.verdict, Verdict.INVALID_SPEC)
        self.assertEqual(result.reason, "invalid_step_bound")


if __name__ == "__main__":
    unittest.main()
