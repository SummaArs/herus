from __future__ import annotations
import unittest

from sim_local_learning import LocalPrototypeBank, train_bounded
from symbiotic_intelligence import Pattern


class SIMLocalLearningTests(unittest.TestCase):
    def pattern(self, values, source="sensor", digest="d"):
        return Pattern(tuple(values), source, digest)

    def test_bounded_training_and_scoring(self):
        bank = LocalPrototypeBank()
        train_bounded(bank, [(self.pattern((8, 0, 0, 0)), "ARRIVE"), (self.pattern((0, 8, 0, 0)), "HELP")], max_updates=2)
        label, confidence = bank.score(self.pattern((7, 0, 0, 0)))
        self.assertEqual(label, "ARRIVE")
        self.assertGreaterEqual(confidence, 0)
        self.assertEqual(bank.version, 3)

    def test_update_has_one_step_rollback(self):
        bank = LocalPrototypeBank()
        bank.update(self.pattern((8, 0, 0, 0)), "ARRIVE")
        before = bank.snapshot()
        bank.update(self.pattern((0, 8, 0, 0)), "HELP")
        restored = bank.rollback()
        self.assertEqual(restored, before)
        with self.assertRaises(ValueError):
            bank.rollback()

    def test_invalid_data_and_budget_fail_closed(self):
        bank = LocalPrototypeBank(max_samples=1)
        with self.assertRaises(ValueError):
            bank.update(self.pattern((1, 2), digest="d"), "ARRIVE")
        with self.assertRaises(ValueError):
            bank.update(self.pattern((1, 2, 3, 4), source=""), "ARRIVE")
        bank.update(self.pattern((1, 2, 3, 4)), "ARRIVE")
        with self.assertRaises(ValueError):
            bank.update(self.pattern((1, 2, 3, 4)), "ARRIVE")

    def test_unknown_never_creates_learned_class(self):
        bank = LocalPrototypeBank()
        with self.assertRaises(ValueError):
            bank.update(self.pattern((1, 2, 3, 4)), "UNKNOWN")
        self.assertEqual(bank.score(self.pattern((1, 2, 3, 4)))[0], "UNKNOWN")


if __name__ == "__main__":
    unittest.main()
