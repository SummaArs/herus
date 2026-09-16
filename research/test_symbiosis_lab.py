from __future__ import annotations
import unittest

from host_discovery_lab import build_hidden_hosts
from symbiotic_intelligence import LABELS
from symbiotic_models import PersistentIdentity
from symbiosis_lab import run_symbiosis


class SymbiosisLabTests(unittest.TestCase):
    def test_same_core_adapts_to_distinct_hosts_without_authority(self):
        identity = PersistentIdentity("herus-lab")
        runs = [run_symbiosis(host, identity) for host in build_hidden_hosts()]
        self.assertGreaterEqual(len(runs), 3)
        self.assertEqual({run.state.identity.herus_id for run in runs}, {"herus-lab"})
        self.assertGreaterEqual(len({run.plan.representations for run in runs}), 2)
        for run in runs:
            self.assertIn(run.plan.status, {"ADAPTED", "ADAPTED_WITH_LIMITS"})
            self.assertTrue(run.state.host.host_id.startswith("discovered:"))
            self.assertEqual(run.state.host.resources["memory_bytes"], 0)
            self.assertEqual(run.state.self_model.authority, "NONE")
            self.assertEqual(run.execution, "ABSTAIN")
            self.assertIn(run.proposal, {"PROPOSE", "ABSTAIN"})
            self.assertEqual(run.state.validate(), ())

    def test_restricted_budget_adapts_without_inventing_capabilities(self):
        host = build_hidden_hosts()[0]
        run = run_symbiosis(host, PersistentIdentity("herus-restricted"), max_probes=2)
        self.assertEqual(run.plan.status, "ADAPTED_WITH_LIMITS")
        self.assertLessEqual(set(run.plan.representations), set(host.truth.representation_set))
        self.assertEqual(run.execution, "ABSTAIN")

    def test_host_change_never_carries_world_or_authority(self):
        identity = PersistentIdentity("herus-isolated")
        first, second = build_hidden_hosts()[:2]
        a = run_symbiosis(first, identity)
        b = run_symbiosis(second, identity)
        self.assertNotEqual(a.state.host.host_id, b.state.host.host_id)
        self.assertEqual(a.state.world.observations, ())
        self.assertEqual(b.state.world.observations, ())
        self.assertEqual(a.state.self_model.authority, "NONE")
        self.assertEqual(b.state.self_model.authority, "NONE")


if __name__ == "__main__":
    unittest.main()
