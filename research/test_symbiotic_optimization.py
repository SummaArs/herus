from __future__ import annotations
import unittest

from host_profile import HostProfile
from symbiotic_intelligence import OptimizationWeights, representation_objective, decide, Pattern


class SymbioticOptimizationTests(unittest.TestCase):
    def test_objective_is_explicit_and_prefers_lower_cost_when_equivalent(self):
        self.assertLess(representation_objective("SIM-RULES"), representation_objective("SIM-HDC8"))
        self.assertLess(representation_objective("SIM-HDC8"), representation_objective("SIM-INT8"))
        self.assertLess(representation_objective("SIM-INT8", weights=OptimizationWeights(bytes_cost=0, steps_cost=0, uncertainty_cost=0, utility_reward=10)), representation_objective("SIM-RULES", weights=OptimizationWeights(bytes_cost=0, steps_cost=0, uncertainty_cost=0, utility_reward=10)))

    def test_optimizer_never_selects_representation_outside_budget(self):
        profile = HostProfile(
            host_id="optimization-host", revision="1", resources={"memory_bytes": 4096},
            interfaces=frozenset({"serial"}), constraints={"max_payload_bytes": 32},
            representation_set=frozenset({"SIM-INT8", "SIM-HDC8", "SIM-RULES"}),
            evidence={"source": "test"}, authority="NONE", skill_budget={"bytes": 512, "steps": 4},
        )
        decision = decide(profile, Pattern((1, 0, 0, 0), "test", "digest"))
        self.assertEqual(decision.representation, "SIM-RULES")
        self.assertEqual(decision.execution, "ABSTAIN")


if __name__ == "__main__":
    unittest.main()
