import unittest

from host_discovery import DiscoveryBudget, discover_host
from host_discovery_lab import HostOracle, build_hidden_hosts


class ActiveDiscoveryTests(unittest.TestCase):
    def _discover(self, index=0, budget=None):
        return discover_host(
            HostOracle(build_hidden_hosts()[index]),
            candidate_formats=("HIR8", "HIR16", "HIR32", "JSON"),
            candidate_interfaces=("radio", "serial", "haptic", "display", "sensor"),
            latency_targets=("HIR8", "HIR16", "JSON"),
            budget=budget or DiscoveryBudget(max_probes=32, max_bytes=10000),
        )

    def test_discovers_present_and_absent_capabilities(self):
        result = self._discover(0)
        self.assertIn("HIR8", result.hypothesis.proven_formats)
        self.assertIn("radio", result.hypothesis.proven_interfaces)
        self.assertNotIn("JSON", result.hypothesis.proven_formats)
        self.assertNotIn("JSON", result.hypothesis.unknown_formats)
        self.assertNotIn("display", result.hypothesis.proven_interfaces)
        self.assertNotIn("display", result.hypothesis.unknown_interfaces)
        self.assertEqual(result.hypothesis.authority, "NONE")
        self.assertEqual(result.hypothesis.allowed_effects, frozenset())

    def test_restricted_host_does_not_inherit_capabilities(self):
        result = self._discover(2)
        self.assertEqual(result.hypothesis.proven_formats, frozenset({"HIR8"}))
        self.assertEqual(result.hypothesis.proven_interfaces, frozenset({"serial"}))
        self.assertNotIn("haptic", result.hypothesis.proven_interfaces)
        self.assertNotIn("JSON", result.hypothesis.proven_formats)

    def test_probe_budget_forces_incomplete_hypothesis(self):
        result = self._discover(1, DiscoveryBudget(max_probes=2, max_bytes=10000))
        self.assertEqual(result.probes_used, 2)
        self.assertIn("probe_budget_exhausted", result.blocked_reasons)
        self.assertTrue(result.hypothesis.unknown_formats)

    def test_byte_budget_blocks_without_fabricating_success(self):
        result = self._discover(1, DiscoveryBudget(max_probes=32, max_bytes=1))
        self.assertEqual(result.probes_used, 0)
        self.assertIn("byte_budget_exhausted", result.blocked_reasons)
        self.assertEqual(result.hypothesis.proven_formats, frozenset())


if __name__ == "__main__":
    unittest.main()
