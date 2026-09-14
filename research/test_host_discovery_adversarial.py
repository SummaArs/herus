import unittest

from host_discovery import DiscoveryBudget, discover_host
from host_discovery_adversarial import AdversarialOracle
from host_discovery_lab import HostOracle, build_hidden_hosts


class HostDiscoveryAdversarialTests(unittest.TestCase):
    def _run(self, mode):
        return discover_host(
            AdversarialOracle(HostOracle(build_hidden_hosts()[0]), mode),
            candidate_formats=("HIR8", "HIR16"),
            candidate_interfaces=("radio", "haptic"),
            latency_targets=("HIR8",),
            budget=DiscoveryBudget(max_probes=8, max_bytes=10000),
        )

    def test_tampered_digest_blocks_success(self):
        result = self._run("tamper_digest")
        self.assertEqual(result.hypothesis.proven_formats, frozenset())
        self.assertIn("evidence_digest_invalid", result.blocked_reasons)

    def test_replay_blocks_success(self):
        result = self._run("replay")
        self.assertNotIn("HIR16", result.hypothesis.proven_formats)
        self.assertIn("evidence_digest_invalid", result.blocked_reasons)

    def test_wrong_session_blocks_success(self):
        result = self._run("wrong_session")
        self.assertEqual(result.hypothesis.proven_interfaces, frozenset())
        self.assertIn("evidence_digest_invalid", result.blocked_reasons)

    def test_conflicting_value_with_valid_digest_is_not_authority(self):
        result = self._run("conflict")
        self.assertEqual(result.hypothesis.authority, "NONE")
        self.assertEqual(result.hypothesis.allowed_effects, frozenset())
        self.assertNotIn("HIR8", result.hypothesis.proven_formats)


if __name__ == "__main__":
    unittest.main()
