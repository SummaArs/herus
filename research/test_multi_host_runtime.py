from __future__ import annotations
import unittest

from multi_host_runtime import Experience, ExperienceBus, run_concurrent_symbiosis


class MultiHostRuntimeTests(unittest.TestCase):
    def test_all_hosts_run_with_same_identity_and_exchange_observations(self):
        ids = ("computer", "filesystem", "sandbox", "internet", "datasets", "robot-sim", "finance-sandbox", "server-shadow")
        runs = run_concurrent_symbiosis(ids)
        self.assertEqual({run.identity for run in runs}, {"herus-general"})
        self.assertEqual({run.host_id for run in runs}, set(ids))
        self.assertTrue(any(run.experiences_consumed for run in runs))
        self.assertEqual({run.execution for run in runs}, {"ABSTAIN"})
        self.assertEqual({run.proposal for run in runs}, {"PROPOSE"})

    def test_bus_rejects_authority_and_private_context(self):
        bus = ExperienceBus()
        for kind in ("authority", "private_context", "message_content", "execution"):
            item = Experience("host", kind, 1, "x", 900, "digest", 0)
            self.assertFalse(bus.publish(item))
        self.assertEqual(bus.snapshot(), ())

    def test_duplicate_experience_is_idempotent_and_replay_safe(self):
        bus = ExperienceBus()
        item = Experience("host", "latency", 10, "ms", 900, "digest", 0)
        self.assertTrue(bus.publish(item))
        self.assertTrue(bus.publish(item))
        self.assertEqual(len(bus.snapshot()), 1)

    def test_invalid_confidence_is_rejected(self):
        bus = ExperienceBus()
        item = Experience("host", "utility", 1, "milli", 1001, "digest", 0)
        self.assertFalse(bus.publish(item))


if __name__ == "__main__":
    unittest.main()
