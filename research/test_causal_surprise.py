from __future__ import annotations

import unittest

from causal_surprise import (
    ModelStatus,
    Transition,
    model_from_transitions,
)


class CausalSurpriseTests(unittest.TestCase):
    def setUp(self) -> None:
        self.model = model_from_transitions(
            (
                Transition("nominal", "hold", "nominal"),
                Transition("nominal", "degrade", "degrading"),
                Transition("degrading", "hold", "safe_hold"),
            ),
            ("nominal", "degrading", "safe_hold"),
            ("actuate",),
        )

    def test_predicted_transition_has_zero_surprise(self) -> None:
        report = self.model.observe("nominal", "degrade", "degrading")
        self.assertEqual(report.status, ModelStatus.PREDICTED)
        self.assertEqual(report.score, 0)

    def test_drift_is_reported_as_surprise(self) -> None:
        report = self.model.observe("nominal", "degrade", "safe_hold")
        self.assertEqual(report.status, ModelStatus.SURPRISE)
        self.assertEqual(report.expected, "degrading")
        self.assertEqual(report.score, 1)

    def test_unmodeled_transition_is_not_assumed_safe(self) -> None:
        report = self.model.observe("nominal", "unknown", "nominal")
        self.assertEqual(report.status, ModelStatus.UNKNOWN)
        self.assertEqual(report.score, 2)
        self.assertIsNone(self.model.choose_least_surprising_safe("nominal", ("unknown",)))

    def test_unsafe_prediction_is_blocked_even_if_modeled(self) -> None:
        model = model_from_transitions(
            (Transition("nominal", "bad", "unsafe"),),
            ("nominal",),
        )
        proposal = model.propose("nominal", "bad")
        self.assertFalse(proposal.admissible)
        self.assertEqual(proposal.reason, "unsafe_predicted_state")

    def test_contradictory_model_is_rejected(self) -> None:
        with self.assertRaises(ValueError):
            model_from_transitions(
                (
                    Transition("nominal", "a", "one"),
                    Transition("nominal", "a", "two"),
                ),
                ("nominal",),
            )

    def test_action_selection_abstains_when_no_safe_transition_exists(self) -> None:
        self.assertIsNone(self.model.choose_least_surprising_safe("degrading", ("actuate",)))


if __name__ == "__main__":
    unittest.main()
