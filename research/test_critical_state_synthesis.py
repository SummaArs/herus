from __future__ import annotations

import unittest

from critical_state_synthesis import synthesize
from critical_state_verifier import StateMachineSpec, Transition, Verdict


class CriticalStateSynthesisTests(unittest.TestCase):
    def _spec(self) -> StateMachineSpec:
        return StateMachineSpec(
            initial_state="nominal",
            states=frozenset({"nominal", "safe_hold", "unsafe"}),
            inputs=frozenset({"ok", "loss"}),
            actions=frozenset({"hold", "safe_hold", "actuate"}),
            transitions=(
                Transition("nominal", "ok", "hold", "nominal"),
                Transition("nominal", "loss", "safe_hold", "safe_hold"),
                Transition("safe_hold", "ok", "safe_hold", "safe_hold"),
                Transition("safe_hold", "loss", "safe_hold", "safe_hold"),
            ),
            forbidden_states=frozenset({"unsafe"}),
            forbidden_actions=frozenset({"actuate"}),
            max_steps=2,
        )

    def test_synthesis_finds_independently_verified_policy(self) -> None:
        result = synthesize(self._spec(), max_candidates=1000)
        self.assertIsNotNone(result.policy)
        self.assertEqual(result.certificate.verdict, Verdict.VERIFIED)
        self.assertGreater(result.candidates, 0)
        self.assertFalse(result.exhausted)

    def test_budget_exhaustion_is_unknown(self) -> None:
        result = synthesize(self._spec(), max_candidates=1)
        self.assertIsNone(result.policy)
        self.assertEqual(result.certificate.verdict, Verdict.UNKNOWN)
        self.assertTrue(result.exhausted)

    def test_invalid_budget_is_unknown(self) -> None:
        result = synthesize(self._spec(), max_candidates=0)
        self.assertIsNone(result.policy)
        self.assertEqual(result.certificate.reason, "invalid_candidate_budget")


if __name__ == "__main__":
    unittest.main()
