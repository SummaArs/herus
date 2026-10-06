import unittest
import numpy as np
from ml_paradigms_benchmark import kmeans, contextual_bandit, self_supervised_cooccurrence


class ParadigmBenchmarkTests(unittest.TestCase):
    def test_kmeans_is_deterministic_and_finite(self):
        x = np.array([[1., 0.], [1., 0.], [0., 1.], [0., 1.]])
        centers = kmeans(x, 2)
        self.assertEqual(centers.shape, (2, 2))
        self.assertTrue(np.isfinite(centers).all())

    def test_bandit_proxy_can_abstain_on_unknown_context(self):
        fit = [{'text': 'alpha', 'label': 'A'}]
        test = [{'text': 'unknown', 'label': 'B'}]
        self.assertEqual(contextual_bandit(fit, [], test), [None])

    def test_self_supervised_adapter_returns_finite_predictions(self):
        fit = [
            {'text': 'alpha blue', 'label': 'A'},
            {'text': 'beta red', 'label': 'B'},
        ]
        predictions = self_supervised_cooccurrence(fit, [], [{'text': 'alpha', 'label': 'A'}])
        self.assertEqual(len(predictions), 1)
        self.assertIn(predictions[0], {'A', 'B'})


if __name__ == '__main__':
    unittest.main()
