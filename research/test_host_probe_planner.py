import unittest

from host_discovery import HostHypothesis
from host_probe_planner import choose_next_probe


class HostProbePlannerTests(unittest.TestCase):
    def test_selects_unresolved_high_information_probe(self):
        hypothesis = HostHypothesis(
            session_id="s",
            proven_formats=frozenset(),
            proven_interfaces=frozenset(),
            max_payload_bytes=None,
            latencies_ms={},
            unknown_formats=frozenset({"HIR8", "HIR16"}),
            unknown_interfaces=frozenset({"radio"}),
        )
        plan = choose_next_probe(
            hypothesis,
            candidate_formats=("HIR8", "HIR16"),
            candidate_interfaces=("radio",),
            latency_targets=("HIR8",),
            next_sequence=0,
        )
        self.assertIsNotNone(plan)
        self.assertEqual(plan.request.name, "max_payload_bytes")

    def test_no_unresolved_capability_means_no_plan(self):
        hypothesis = HostHypothesis(
            session_id="s",
            proven_formats=frozenset({"HIR8"}),
            proven_interfaces=frozenset({"radio"}),
            max_payload_bytes=32,
            latencies_ms={"HIR8": 42},
            unknown_formats=frozenset(),
            unknown_interfaces=frozenset(),
        )
        plan = choose_next_probe(
            hypothesis,
            candidate_formats=("HIR8",),
            candidate_interfaces=("radio",),
            latency_targets=("HIR8",),
            next_sequence=4,
        )
        self.assertIsNone(plan)

    def test_authority_is_not_a_probe_candidate(self):
        hypothesis = HostHypothesis(
            session_id="s",
            proven_formats=frozenset(),
            proven_interfaces=frozenset(),
            max_payload_bytes=32,
            latencies_ms={},
            unknown_formats=frozenset({"HIR8"}),
            unknown_interfaces=frozenset(),
        )
        plan = choose_next_probe(
            hypothesis,
            candidate_formats=("HIR8",),
            candidate_interfaces=(),
            latency_targets=(),
            next_sequence=0,
        )
        self.assertNotIn("authority", plan.request.name)


if __name__ == "__main__":
    unittest.main()
