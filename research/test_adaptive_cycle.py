import unittest

from adaptive_cycle import run_cycle
from host_discovery import DiscoveryBudget
from host_discovery_lab import HostOracle, build_hidden_hosts


class AdaptiveCycleTests(unittest.TestCase):
    def test_cycle_updates_hypothesis_and_records_experiments(self):
        hypothesis, records = run_cycle(
            HostOracle(build_hidden_hosts()[0]),
            candidate_formats=("HIR8", "HIR16", "HIR32", "JSON"),
            candidate_interfaces=("radio", "serial", "haptic", "display", "sensor"),
            latency_targets=("HIR8", "HIR16", "JSON"),
            budget=DiscoveryBudget(max_probes=32, max_bytes=4096),
        )
        self.assertEqual(hypothesis.authority, "NONE")
        self.assertEqual(hypothesis.allowed_effects, frozenset())
        self.assertEqual(hypothesis.proven_formats, frozenset({"HIR8", "HIR16"}))
        self.assertEqual(hypothesis.proven_interfaces, frozenset({"radio", "serial", "haptic"}))
        self.assertEqual(len(records), 13)
        self.assertTrue(all(record.digest for record in records))

    def test_cycle_abstains_when_byte_budget_is_too_small(self):
        hypothesis, records = run_cycle(
            HostOracle(build_hidden_hosts()[0]),
            candidate_formats=("HIR8",),
            candidate_interfaces=("radio",),
            latency_targets=(),
            budget=DiscoveryBudget(max_probes=32, max_bytes=1),
        )
        self.assertEqual(hypothesis.authority, "NONE")
        self.assertTrue(records)
        self.assertEqual(records[-1].outcome, "ABSTAIN")
        self.assertEqual(records[-1].abstention_reason, "byte_budget_exhausted")


if __name__ == "__main__":
    unittest.main()
