from __future__ import annotations

import unittest

from research.symbiont_v2.frontier import (
    ActiveProbePlanner,
    CapabilityGraph,
    CapabilityNode,
    InformationGainModel,
    MemoryItem,
    ModularMemory,
    ProbeCandidate,
    ScalarBelief,
)


class FrontierTests(unittest.TestCase):
    def test_belief_updates_without_external_dependencies(self) -> None:
        belief = ScalarBelief()
        for value in (1, 2, 3, 2):
            belief.update(value)
        self.assertEqual(belief.count, 4)
        self.assertAlmostEqual(belief.mean, 2.0)
        self.assertTrue(belief.interval95.contains(2.0))

    def test_active_probe_prefers_information_per_cost_and_risk(self) -> None:
        planner = ActiveProbePlanner()
        chosen = planner.select((
            ProbeCandidate("cheap", 1.0, 1.0),
            ProbeCandidate("expensive", 4.0, 10.0),
            ProbeCandidate("risky", 5.0, 1.0, risk=1.0),
        ))
        self.assertIsNotNone(chosen)
        self.assertEqual(chosen.identifier, "cheap")  # type: ignore[union-attr]

    def test_capability_graph_is_compositional_and_rejects_cycles(self) -> None:
        graph = CapabilityGraph()
        graph.add(CapabilityNode("move"))
        graph.add(CapabilityNode("navigate", ("move",)))
        graph.add(CapabilityNode("deliver", ("navigate",)))
        self.assertEqual(graph.plan("deliver"), ("move", "navigate", "deliver"))
        graph.add(CapabilityNode("cycle", ("cycle",)))
        self.assertIsNone(graph.plan("cycle"))

    def test_verified_memory_is_protected(self) -> None:
        memory = ModularMemory(capacity=2)
        memory.put(MemoryItem("verified", object(), 0.0, True))
        memory.put(MemoryItem("low", object(), 0.1))
        memory.put(MemoryItem("high", object(), 1.0))
        memory.put(MemoryItem("new", object(), 0.2))
        self.assertIn("verified", memory.items)
        self.assertIn("high", memory.items)
        self.assertEqual(len(memory.items), 2)

    def test_entropy_normalization(self) -> None:
        model = InformationGainModel((2.0, 1.0)).normalized()
        self.assertAlmostEqual(sum(model.probabilities), 1.0)
        self.assertGreater(model.entropy(), 0.0)


if __name__ == "__main__":
    unittest.main()
