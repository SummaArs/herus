from __future__ import annotations

import unittest

from adaptation_reconcile import Observation, reconcile


class AdaptationReconcileTests(unittest.TestCase):
    def rows(self):
        return [
            Observation("sensor-a", 1, "host", "latency", 20.0, "evidence"),
            Observation("sensor-b", 1, "host", "latency", 20.0, "evidence"),
        ]

    def test_consistent_quorum_is_accepted(self) -> None:
        result = reconcile(self.rows())
        self.assertEqual((result.status, result.value), ("ACCEPTED", 20.0))

    def test_missing_quorum_is_blocked(self) -> None:
        result = reconcile(self.rows()[:1])
        self.assertEqual((result.status, result.reason), ("BLOCKED", "quorum_missing"))

    def test_value_conflict_is_blocked(self) -> None:
        rows = self.rows()
        rows[1] = Observation("sensor-b", 1, "host", "latency", 90.0, "evidence")
        result = reconcile(rows)
        self.assertEqual((result.status, result.reason), ("BLOCKED", "value_conflict"))

    def test_replay_is_blocked(self) -> None:
        rows = self.rows()
        rows = rows + [rows[0]]
        result = reconcile(rows)
        self.assertEqual((result.status, result.reason), ("BLOCKED", "replay_or_nonmonotonic_source"))

    def test_scope_conflict_is_blocked(self) -> None:
        rows = self.rows()
        rows[1] = Observation("sensor-b", 1, "other-host", "latency", 20.0, "evidence")
        result = reconcile(rows)
        self.assertEqual((result.status, result.reason), ("BLOCKED", "scope_conflict"))


if __name__ == "__main__":
    unittest.main()
