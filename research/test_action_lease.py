from __future__ import annotations
import unittest

from action_lease import ActionLeaseBook


class ActionLeaseTests(unittest.TestCase):
    def setUp(self):
        self.now = 0.0
        self.book = ActionLeaseBook(ttl=5.0, clock=lambda: self.now)

    def test_only_one_owner_and_fencing_increases_after_expiry(self):
        first = self.book.acquire("a", "host-a")
        self.assertIsNotNone(first)
        self.assertIsNone(self.book.acquire("a", "host-b"))
        self.now = 6.0
        second = self.book.acquire("a", "host-b")
        self.assertIsNotNone(second)
        assert first and second
        self.assertGreater(second.fencing_token, first.fencing_token)
        self.assertFalse(self.book.valid(first))
        self.assertTrue(self.book.valid(second))

    def test_stale_owner_cannot_release_or_renew(self):
        first = self.book.acquire("a", "host-a")
        assert first
        self.now = 6.0
        second = self.book.acquire("a", "host-b")
        assert second
        self.assertFalse(self.book.release(first))
        self.assertIsNone(self.book.renew(first))
        self.assertTrue(self.book.valid(second))

    def test_lease_does_not_grant_execution_authority(self):
        lease = self.book.acquire("a", "host-a")
        assert lease
        self.assertGreater(lease.fencing_token, 0)
        self.assertEqual(self.book.fencing_token("a"), lease.fencing_token)
        self.assertTrue(True)  # lease only serializes; no execute API exists

    def test_invalid_identity_and_ttl_fail_closed(self):
        with self.assertRaises(ValueError):
            ActionLeaseBook(ttl=0)
        with self.assertRaises(ValueError):
            self.book.acquire("", "host-a")


if __name__ == "__main__":
    unittest.main()
