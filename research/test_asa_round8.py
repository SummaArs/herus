from __future__ import annotations

import unittest

from research.asa_round8 import run


class AsaRound8Tests(unittest.TestCase):
    def test_only_valid_observation_and_trusted_rule_authorize(self) -> None:
        result = run()["scenarios"]
        self.assertTrue(result["valid_authorized"]["accepted"])
        self.assertEqual(result["valid_authorized"]["reason"], "AUTHORIZED_BY_TRUSTED_RULE")

    def test_hypothesis_and_preference_cannot_authorize(self) -> None:
        result = run()["scenarios"]
        self.assertFalse(result["hypothesis_only"]["accepted"])
        self.assertEqual(result["hypothesis_only"]["reason"], "NO_POLICY_AUTHORIZATION")
        self.assertFalse(result["preference_only"]["accepted"])
        self.assertEqual(result["preference_only"]["reason"], "NO_POLICY_AUTHORIZATION")

    def test_conflicts_and_missing_observation_abstain(self) -> None:
        result = run()["scenarios"]
        self.assertEqual(result["conflicting_observations"]["reason"], "CONFLICTING_OBSERVATIONS")
        self.assertEqual(result["conflicting_equal_rules"]["reason"], "CONFLICTING_RULES")
        self.assertEqual(result["missing_observation"]["reason"], "MISSING_OBSERVATION")
        self.assertEqual(result["unknown_rule_value"]["reason"], "UNKNOWN_RULE_VALUE")

    def test_untrusted_sources_cannot_override_policy(self) -> None:
        result = run()["scenarios"]
        self.assertEqual(result["model_override_attempt"]["reason"], "UNTRUSTED_RULE_SOURCE")
        self.assertEqual(result["lower_authority_untrusted_rule"]["reason"], "UNTRUSTED_RULE_SOURCE")
        self.assertEqual(run()["metrics"]["untrusted_override_accepted"], 0)

    def test_safety_rule_overrides_preference_without_being_overridden(self) -> None:
        scenario = run()["scenarios"]["safety_deny_over_preference"]
        self.assertFalse(scenario["accepted"])
        self.assertEqual(scenario["reason"], "POLICY_DENIED")
        self.assertIn("PREFERENCE", scenario["audit"]["ignored_non_authoritative"])

    def test_authority_boundary_is_explicit(self) -> None:
        self.assertEqual(run()["authority_boundary"], "proposal-only; policy engine is the sole rule authority")
        self.assertEqual(run()["metrics"]["unsafe_non_abstention"], 0)


if __name__ == "__main__":
    unittest.main()
