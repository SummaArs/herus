import unittest

from host_discovery_lab import HostOracle, ProbeRequest, build_hidden_hosts


class HiddenHostLabTests(unittest.TestCase):
    def test_hosts_are_structurally_distinct(self):
        hosts = build_hidden_hosts()
        self.assertEqual(len(hosts), 3)
        self.assertEqual({h.truth.host_id for h in hosts}, {"hidden-pulse", "hidden-server", "hidden-restricted"})
        self.assertNotEqual(hosts[0].truth.interfaces, hosts[1].truth.interfaces)
        self.assertNotEqual(hosts[1].truth.constraints, hosts[2].truth.constraints)

    def test_probe_exposes_observation_not_full_profile(self):
        oracle = HostOracle(build_hidden_hosts()[0])
        observation = oracle.probe(ProbeRequest("has_interface", "haptic", 0))
        self.assertEqual(observation.value, "true")
        self.assertEqual(observation.unit, "bool")
        self.assertEqual(observation.host_session, "session-pulse")
        self.assertNotIn("resources", observation.to_dict())
        self.assertTrue(observation.evidence_digest)

    def test_forbidden_probe_is_rejected(self):
        oracle = HostOracle(build_hidden_hosts()[0])
        with self.assertRaises(ValueError):
            oracle.probe(ProbeRequest("read_identity", "owner", 0))

    def test_authority_is_not_probeable(self):
        for hidden in build_hidden_hosts():
            self.assertEqual(HostOracle(hidden).truth_for_evaluation().authority, "NONE")
            with self.assertRaises(ValueError):
                HostOracle(hidden).probe(ProbeRequest("read_authority", "authority", 0))

    def test_negative_sequence_is_rejected(self):
        oracle = HostOracle(build_hidden_hosts()[0])
        with self.assertRaises(ValueError):
            oracle.probe(ProbeRequest("supports_format", "HIR8", -1))

    def test_unknown_capability_is_explicitly_false_or_unknown(self):
        oracle = HostOracle(build_hidden_hosts()[2])
        interface = oracle.probe(ProbeRequest("has_interface", "haptic", 0))
        fmt = oracle.probe(ProbeRequest("supports_format", "JSON", 1))
        self.assertEqual(interface.value, "false")
        self.assertEqual(fmt.value, "false")


if __name__ == "__main__":
    unittest.main()
