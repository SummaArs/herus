import unittest

from adaptive_generalization_benchmark import run_benchmark


class AdaptiveGeneralizationBenchmarkTests(unittest.TestCase):
    def test_full_budget_covers_all_hidden_hosts_without_effects(self):
        result = run_benchmark()
        full = [row for row in result["rows"] if row["budget_bytes"] == 4096]
        self.assertEqual(len(full), 3)
        for row in full:
            self.assertGreater(row["observations"], 0)
            self.assertEqual(row["abstentions"], 0)
            self.assertEqual(row["authority"], "NONE")
            self.assertEqual(row["allowed_effects"], [])

    def test_small_budget_abstains_on_every_hidden_host(self):
        result = run_benchmark()
        small = [row for row in result["rows"] if row["budget_bytes"] == 192]
        self.assertEqual(len(small), 3)
        for row in small:
            self.assertEqual(row["abstentions"], 1)
            self.assertEqual(row["proven_formats"], [])
            self.assertEqual(row["proven_interfaces"], [])


if __name__ == "__main__":
    unittest.main()
