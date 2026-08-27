import unittest

from semantic_ir_fuzz import run_campaign


class SemanticIRFuzzTests(unittest.TestCase):
    def test_deterministic_adversarial_campaign(self):
        report = run_campaign(cases_per_mutator=25)
        self.assertEqual(report["valid_failures"], [])
        self.assertEqual(report["mutation_failures"], [])
        self.assertEqual(report["mutator_count"], 27)
        self.assertEqual(report["mutated_cases"], 25 * 27)
        self.assertTrue(report["pass"])


if __name__ == "__main__":
    unittest.main()
