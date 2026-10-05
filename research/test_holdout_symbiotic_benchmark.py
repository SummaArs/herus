import unittest
from holdout_symbiotic_benchmark import evaluate

class HoldoutSymbioticBenchmarkTests(unittest.TestCase):
    def test_independent_holdout_keeps_safe_accuracy_and_zero_false_accepts(self):
        result = evaluate(12)
        self.assertEqual(result['case_count'], 60)
        self.assertTrue(result['oracle_hidden_from_learner'])
        self.assertEqual(result['metrics'], {'safe_correct': 12, 'safe_total': 12, 'unsafe_false_accepts': 0, 'unsafe_total': 48})

    def test_holdout_contains_unseen_categories(self):
        result = evaluate(3)
        reasons = {row['reason'] for row in result['rows']}
        self.assertIn('effect_not_observed', reasons)
        self.assertIn('ambiguous_effect', reasons)
        self.assertIn('temporal_drift', reasons)

if __name__ == '__main__': unittest.main()
