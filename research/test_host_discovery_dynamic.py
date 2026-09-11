import unittest

from host_discovery import DiscoveryBudget, discover_host
from host_discovery_dynamic import DynamicChange, DynamicHostOracle
from host_discovery_lab import build_hidden_hosts


class DynamicHostDiscoveryTests(unittest.TestCase):
    def test_change_is_observable_and_requires_new_session_for_fresh_truth(self):
        hidden = build_hidden_hosts()[0]
        oracle = DynamicHostOracle(
            hidden,
            DynamicChange(after_probe_count=4, capability="display", enabled_after=True),
        )
        first = discover_host(
            oracle,
            candidate_formats=("HIR8",),
            candidate_interfaces=("radio", "display", "display"),
            latency_targets=("HIR8",),
            budget=DiscoveryBudget(max_probes=8, max_bytes=2048),
        )
        self.assertEqual(first.hypothesis.authority, "NONE")
        self.assertNotIn("display", first.hypothesis.proven_interfaces)
        self.assertIn("display", first.hypothesis.unknown_interfaces)
        self.assertIn("observation_conflict", first.blocked_reasons)

    def test_dynamic_host_never_grants_effects(self):
        hidden = build_hidden_hosts()[1]
        oracle = DynamicHostOracle(
            hidden,
            DynamicChange(after_probe_count=2, capability="haptic", enabled_after=True),
        )
        result = discover_host(
            oracle,
            candidate_formats=("HIR16",),
            candidate_interfaces=("haptic",),
            latency_targets=("HIR16",),
            budget=DiscoveryBudget(max_probes=8, max_bytes=2048),
        )
        self.assertEqual(result.hypothesis.authority, "NONE")
        self.assertEqual(result.hypothesis.allowed_effects, frozenset())


if __name__ == "__main__":
    unittest.main()
