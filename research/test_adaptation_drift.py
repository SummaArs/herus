from __future__ import annotations

import unittest

from adaptation_drift import Belief, assess_drift


class AdaptationDriftTests(unittest.TestCase):
    def test_stable_belief_is_kept(self) -> None:
        belief = Belief("latency_p95_ms", 20.0, 5.0, 10, "host-a")
        result = assess_drift(belief, 23.0, "host-a")
        self.assertFalse(result.drifted)
        self.assertEqual(result.action, "KEEP_BELIEF")

    def test_metric_drift_invalidates_belief(self) -> None:
        belief = Belief("latency_p95_ms", 20.0, 5.0, 10, "host-a")
        result = assess_drift(belief, 31.0, "host-a")
        self.assertTrue(result.drifted)
        self.assertEqual(result.action, "INVALIDATE_AND_RENEGOTIATE")

    def test_host_revision_change_invalidates_even_same_metric(self) -> None:
        belief = Belief("ram_free_bytes", 1000.0, 100.0, 10, "host-a")
        result = assess_drift(belief, 1000.0, "host-b")
        self.assertTrue(result.drifted)
        self.assertEqual(result.action, "INVALIDATE_AND_RENEGOTIATE")


if __name__ == "__main__":
    unittest.main()
