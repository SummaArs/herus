import unittest

from host_discovery_benchmark import run


class HostDiscoveryBenchmarkTests(unittest.TestCase):
    def test_full_budget_reaches_all_hidden_capabilities_without_false_positive(self):
        result = run()
        full = [row for row in result["rows"] if row["max_probes"] == 32]
        self.assertEqual(len(full), 3)
        self.assertTrue(all(row["coverage"] == 1.0 for row in full))
        self.assertTrue(all(row["false_positive_count"] == 0 for row in full))
        self.assertTrue(all(row["authority"] == "NONE" for row in full))
        self.assertTrue(all(row["allowed_effects"] == [] for row in full))

    def test_small_budget_abstains(self):
        result = run()
        small = [row for row in result["rows"] if row["max_probes"] == 2]
        self.assertEqual(len(small), 3)
        self.assertTrue(all(row["abstained"] for row in small))
        self.assertTrue(all(row["false_positive_count"] == 0 for row in small))


if __name__ == "__main__":
    unittest.main()
