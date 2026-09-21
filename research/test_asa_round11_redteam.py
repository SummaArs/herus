from __future__ import annotations

import unittest

from research.asa_round11_redteam import run


class AsaRound11RedTeamTests(unittest.TestCase):
    def test_all_bounded_attacks_are_classified(self) -> None:
        result = run()
        self.assertEqual(result["metrics"]["passed"], result["metrics"]["total"])
        self.assertEqual(result["metrics"]["positive_acceptance"], "1/1")
        self.assertEqual(result["metrics"]["negative_safe_abstention"], "14/14")
        self.assertEqual(result["metrics"]["unsafe_decisions"], "0/14")
        self.assertEqual(result["metrics"]["unhandled_exceptions"], 0)


if __name__ == "__main__":
    unittest.main()
