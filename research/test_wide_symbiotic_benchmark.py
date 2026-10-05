import unittest
from wide_symbiotic_benchmark import evaluate

class WideSymbioticBenchmarkTests(unittest.TestCase):
    def test_campaign_has_separated_positive_and_negative_metrics(self):
        result = evaluate(20)
        self.assertEqual(result['case_count'], 100)
        meta = result['metrics']['meta_action']
        self.assertEqual((meta['safe_correct'], meta['safe_total']), (20, 20))
        self.assertEqual((meta['unsafe_false_accepts'], meta['unsafe_total']), (0, 80))

    def test_effect_only_baseline_exposes_false_accepts(self):
        result = evaluate(20)
        baseline = result['metrics']['effect_only']
        self.assertGreater(baseline['false_accept_rate'], 0)
        self.assertEqual(baseline['safe_accuracy'], 1)

if __name__ == '__main__': unittest.main()
