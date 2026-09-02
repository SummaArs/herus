import unittest

from critical_policy_refinement import (
    PolicyRefinementVerdict,
    check_policy_refinement,
)
from critical_state_verifier import PolicyRule, StateMachineSpec, Transition


class CriticalPolicyRefinementTests(unittest.TestCase):
    def setUp(self) -> None:
        self.abstract = StateMachineSpec(
            initial_state="safe",
            states=frozenset({"safe", "hold"}),
            inputs=frozenset({"ok", "loss"}),
            actions=frozenset({"hold", "safe_hold"}),
            transitions=(
                Transition("safe", "ok", "hold", "safe"),
                Transition("safe", "loss", "safe_hold", "hold"),
                Transition("hold", "ok", "safe_hold", "hold"),
                Transition("hold", "loss", "safe_hold", "hold"),
            ),
            forbidden_states=frozenset(),
            forbidden_actions=frozenset(),
            max_steps=2,
        )
        self.concrete = StateMachineSpec(
            initial_state="c_safe",
            states=frozenset({"c_safe", "c_hold"}),
            inputs=frozenset({"packet_ok", "packet_loss"}),
            actions=frozenset({"retain", "safe_retain"}),
            transitions=(
                Transition("c_safe", "packet_ok", "retain", "c_safe"),
                Transition("c_safe", "packet_loss", "safe_retain", "c_hold"),
                Transition("c_hold", "packet_ok", "safe_retain", "c_hold"),
                Transition("c_hold", "packet_loss", "safe_retain", "c_hold"),
            ),
            forbidden_states=frozenset(),
            forbidden_actions=frozenset(),
            max_steps=2,
        )
        self.maps = (
            {"c_safe": "safe", "c_hold": "hold"},
            {"packet_ok": "ok", "packet_loss": "loss"},
            {"retain": "hold", "safe_retain": "safe_hold"},
        )
        self.abstract_policy = (
            PolicyRule("safe", "ok", "hold"),
            PolicyRule("safe", "loss", "safe_hold"),
            PolicyRule("hold", "ok", "safe_hold"),
            PolicyRule("hold", "loss", "safe_hold"),
        )
        self.concrete_policy = (
            PolicyRule("c_safe", "packet_ok", "retain"),
            PolicyRule("c_safe", "packet_loss", "safe_retain"),
            PolicyRule("c_hold", "packet_ok", "safe_retain"),
            PolicyRule("c_hold", "packet_loss", "safe_retain"),
        )

    def test_concrete_policy_refines_abstract_policy(self) -> None:
        result = check_policy_refinement(
            self.abstract, self.concrete, self.abstract_policy, self.concrete_policy, *self.maps
        )
        self.assertEqual(result.verdict, PolicyRefinementVerdict.REFINED)

    def test_wrong_concrete_action_is_counterexample(self) -> None:
        policy = self.concrete_policy[:-1] + (PolicyRule("c_hold", "packet_loss", "retain"),)
        result = check_policy_refinement(
            self.abstract, self.concrete, self.abstract_policy, policy, *self.maps
        )
        self.assertEqual(result.verdict, PolicyRefinementVerdict.COUNTEREXAMPLE)
        self.assertEqual(result.reason, "policy_action_not_refined")
        self.assertEqual(result.concrete_rule.action, "retain")

    def test_incomplete_policy_is_invalid(self) -> None:
        result = check_policy_refinement(
            self.abstract, self.concrete, self.abstract_policy[:-1], self.concrete_policy, *self.maps
        )
        self.assertEqual(result.verdict, PolicyRefinementVerdict.INVALID)
        self.assertEqual(result.reason, "abstract_policy_not_total")

    def test_incomplete_map_is_invalid(self) -> None:
        result = check_policy_refinement(
            self.abstract, self.concrete, self.abstract_policy, self.concrete_policy,
            {"c_safe": "safe"}, self.maps[1], self.maps[2]
        )
        self.assertEqual(result.verdict, PolicyRefinementVerdict.INVALID)
        self.assertEqual(result.reason, "incomplete_state_map")


if __name__ == "__main__":
    unittest.main()
