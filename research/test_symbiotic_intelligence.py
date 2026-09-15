from __future__ import annotations
import unittest

from host_profile import HostProfile
from symbiotic_intelligence import MicroMLP, Pattern, decide, train_local_delta


class SIMTests(unittest.TestCase):
    def profile(self, *, representations=frozenset({"SIM-INT8"}), bytes=4096, steps=12, authority="NONE"):
        return HostProfile(
            host_id="sim-host", revision="r1", resources={"ram_bytes": 8192},
            interfaces=frozenset({"sensor"}), constraints={"max_steps": steps},
            representation_set=representations, skill_budget={"bytes": bytes, "steps": steps},
            evidence={"source": "fixture"}, authority=authority,
        )

    def pattern(self, features=(4, 0, 0, 0)):
        return Pattern(features, "local-sensor", "a" * 64)

    def test_neural_scores_are_deterministic_and_finite(self):
        first = MicroMLP().infer(self.pattern())
        second = MicroMLP().infer(self.pattern())
        self.assertEqual(first, second)
        self.assertIn(first.label, {"ARRIVE", "HELP", "CANCEL", "UNKNOWN"})
        self.assertTrue(0 <= first.confidence_milli <= 1000)

    def test_sim_proposes_but_never_executes(self):
        result = decide(self.profile(), self.pattern())
        self.assertEqual(result.proposal, "PROPOSE")
        self.assertEqual(result.execution, "ABSTAIN")
        self.assertEqual(result.representation, "SIM-INT8")

    def test_budget_selects_smaller_representation_or_abstains(self):
        result = decide(self.profile(representations=frozenset({"SIM-RULES"}), bytes=512, steps=4), self.pattern(), required_bytes=512, required_steps=4)
        self.assertEqual(result.representation, "SIM-RULES")
        blocked = decide(self.profile(representations=frozenset({"SIM-INT8"}), bytes=128, steps=1), self.pattern())
        self.assertEqual(blocked.reason, "no_representation_fits_budget")
        self.assertEqual(blocked.proposal, "ABSTAIN")

    def test_authority_and_provenance_fail_closed(self):
        self.assertEqual(decide(self.profile(authority="PROPOSAL_ONLY"), self.pattern()).reason, "host_authority_not_discoverable")
        self.assertEqual(decide(self.profile(), Pattern((4, 0, 0, 0), "", "a" * 64)).proposal, "ABSTAIN")

    def test_local_training_returns_stats_without_mutating_weights(self):
        model_before = MicroMLP().hidden_weights
        stats = train_local_delta(((self.pattern(), "ARRIVE"), (self.pattern((0, 4, 0, 0)), "HELP")))
        self.assertEqual(stats["ARRIVE"], 1)
        self.assertEqual(stats["HELP"], 1)
        self.assertEqual(MicroMLP().hidden_weights, model_before)

    def test_invalid_training_label_is_rejected(self):
        with self.assertRaises(ValueError):
            train_local_delta(((self.pattern(), "OPEN_BANK_ACCOUNT"),))


if __name__ == "__main__":
    unittest.main()
