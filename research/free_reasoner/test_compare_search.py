from __future__ import annotations

import unittest

from .compare_search import error, enum_search, prove_candidate, tasks


class ComparisonSafetyTests(unittest.TestCase):
    def test_underdetermined_fit_is_not_promoted(self) -> None:
        task = next(item for item in tasks() if item.name == "underdetermined_square")
        candidate, _ = enum_search(task)
        self.assertIsNotNone(candidate)
        self.assertEqual(error(candidate, task.train), 0)
        self.assertNotEqual(error(candidate, task.holdout), 0)
        self.assertFalse(prove_candidate(candidate, task.target))


if __name__ == "__main__":
    unittest.main()
