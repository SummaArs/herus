from __future__ import annotations

import unittest

from critical_state_benchmark import run


class CriticalStateBenchmarkTests(unittest.TestCase):
    def test_expected_verdicts_are_preserved(self) -> None:
        rows = {row["case"]: row for row in run(max_candidates=256)}
        self.assertEqual(rows["valid_finite_policy"]["observed"], "VERIFIED")
        self.assertTrue(rows["valid_finite_policy"]["verified"])
        self.assertEqual(rows["incomplete_model"]["observed"], "UNKNOWN")
        self.assertFalse(rows["incomplete_model"]["verified"])

    def test_results_are_reproducible(self) -> None:
        self.assertEqual(run(max_candidates=32), run(max_candidates=32))


if __name__ == "__main__":
    unittest.main()
