from __future__ import annotations

import unittest

from transfer_benchmark import build_abstraction, transfer


class TransferBenchmarkTests(unittest.TestCase):
    def test_transfer_to_new_domain_is_proposal_only(self) -> None:
        abstraction = build_abstraction()
        result = transfer(abstraction, "server", "host-server-a", {"stop", "rollback"})
        self.assertTrue(result.transferred)
        self.assertEqual(result.reason, "structural_transfer_proposal_only")

    def test_known_domain_is_not_called_transfer(self) -> None:
        abstraction = build_abstraction()
        result = transfer(abstraction, "robotics", "host-robot-b", {"stop"})
        self.assertFalse(result.transferred)
        self.assertEqual(result.reason, "source_domain_reused")

    def test_missing_target_vocabulary_blocks_transfer(self) -> None:
        abstraction = build_abstraction()
        result = transfer(abstraction, "finance_sandbox", "host-finance-a", {"observe", "propose"})
        self.assertFalse(result.transferred)
        self.assertEqual(result.reason, "consequence_not_in_target_vocabulary")


if __name__ == "__main__":
    unittest.main()
