from __future__ import annotations

import unittest

from critical_state_refinement import RefinementVerdict, check_refinement, check_refinement_with_maps
from critical_state_verifier import StateMachineSpec, Transition


class CriticalStateRefinementTests(unittest.TestCase):
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

    def test_concrete_machine_refines(self) -> None:
        concrete = self.abstract
        result = check_refinement(self.abstract, concrete, {"safe": "safe", "hold": "hold"})
        self.assertEqual(result.verdict, RefinementVerdict.REFINED)

    def test_missing_map_is_invalid(self) -> None:
        result = check_refinement(self.abstract, self.abstract, {"safe": "safe"})
        self.assertEqual(result.verdict, RefinementVerdict.INVALID)
        self.assertEqual(result.reason, "incomplete_state_map")

    def test_bad_successor_is_counterexample(self) -> None:
        concrete = StateMachineSpec(
            initial_state="safe",
            states=frozenset({"safe", "hold"}),
            inputs=self.abstract.inputs,
            actions=self.abstract.actions,
            transitions=(Transition("safe", "loss", "safe_hold", "safe"),),
            forbidden_states=frozenset(),
            forbidden_actions=frozenset(),
            max_steps=1,
        )
        result = check_refinement(self.abstract, concrete, {"safe": "safe", "hold": "hold"})
        self.assertEqual(result.verdict, RefinementVerdict.COUNTEREXAMPLE)
        self.assertEqual(result.reason, "next_state_not_refined")

    def test_renamed_inputs_and_actions_refine_with_explicit_maps(self) -> None:
        concrete = StateMachineSpec(
            initial_state="c_safe",
            states=frozenset({"c_safe", "c_hold"}),
            inputs=frozenset({"packet_ok", "packet_loss"}),
            actions=frozenset({"retain", "safe_retain"}),
            transitions=(
                Transition("c_safe", "packet_ok", "retain", "c_safe"),
                Transition("c_safe", "packet_loss", "safe_retain", "c_hold"),
            ),
            forbidden_states=frozenset(),
            forbidden_actions=frozenset(),
            max_steps=1,
        )
        result = check_refinement_with_maps(
            self.abstract,
            concrete,
            {"c_safe": "safe", "c_hold": "hold"},
            {"packet_ok": "ok", "packet_loss": "loss"},
            {"retain": "hold", "safe_retain": "safe_hold"},
        )
        self.assertEqual(result.verdict, RefinementVerdict.REFINED)

    def test_missing_input_map_is_invalid(self) -> None:
        concrete = StateMachineSpec(
            initial_state="c_safe",
            states=frozenset({"c_safe", "c_hold"}),
            inputs=frozenset({"packet_ok", "packet_loss"}),
            actions=frozenset({"hold", "safe_hold"}),
            transitions=(),
            forbidden_states=frozenset(),
            forbidden_actions=frozenset(),
            max_steps=1,
        )
        result = check_refinement_with_maps(
            self.abstract, concrete, {"c_safe": "safe", "c_hold": "hold"}, {}
        )
        self.assertEqual(result.verdict, RefinementVerdict.INVALID)
        self.assertEqual(result.reason, "incomplete_input_map")

    def test_wrong_action_map_is_counterexample(self) -> None:
        concrete = StateMachineSpec(
            initial_state="c_safe",
            states=frozenset({"c_safe", "c_hold"}),
            inputs=frozenset({"packet_ok", "packet_loss"}),
            actions=frozenset({"retain", "safe_retain"}),
            transitions=(Transition("c_safe", "packet_loss", "safe_retain", "c_safe"),),
            forbidden_states=frozenset(),
            forbidden_actions=frozenset(),
            max_steps=1,
        )
        result = check_refinement_with_maps(
            self.abstract,
            concrete,
            {"c_safe": "safe", "c_hold": "hold"},
            {"packet_ok": "ok", "packet_loss": "loss"},
            {"retain": "hold", "safe_retain": "safe_hold"},
        )
        self.assertEqual(result.verdict, RefinementVerdict.COUNTEREXAMPLE)
        self.assertEqual(result.reason, "next_state_not_refined")

    def test_initial_state_mismatch_is_invalid(self) -> None:
        concrete = StateMachineSpec(
            initial_state="hold",
            states=frozenset({"safe", "hold"}),
            inputs=self.abstract.inputs,
            actions=self.abstract.actions,
            transitions=(),
            forbidden_states=frozenset(),
            forbidden_actions=frozenset(),
            max_steps=1,
        )
        result = check_refinement(self.abstract, concrete, {"safe": "safe", "hold": "hold"})
        self.assertEqual(result.verdict, RefinementVerdict.INVALID)
        self.assertEqual(result.reason, "initial_state_not_refined")


if __name__ == "__main__":
    unittest.main()
