import unittest
from real_data_baseline_benchmark import metrics


class RealBaselineMetricTests(unittest.TestCase):
    def test_metrics_distinguish_coverage_from_selective_accuracy(self):
        test = [{'label': 'A'}, {'label': 'B'}, {'label': 'A'}, {'label': 'B'}]
        predictions = ['A', None, 'A', None]
        result = metrics(test, predictions)
        self.assertEqual(result['coverage'], 0.5)
        self.assertEqual(result['selective_accuracy'], 1.0)
        self.assertEqual(result['accuracy'], 0.5)

    def test_wrong_full_coverage_is_not_hidden_by_selective_metric(self):
        test = [{'label': 'A'}, {'label': 'B'}]
        result = metrics(test, ['A', 'A'])
        self.assertEqual(result['coverage'], 1.0)
        self.assertEqual(result['selective_accuracy'], 0.5)


if __name__ == '__main__':
    unittest.main()
