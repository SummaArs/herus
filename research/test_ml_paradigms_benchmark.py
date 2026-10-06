import unittest
import numpy as np
from ml_paradigms_benchmark import kmeans, contextual_bandit


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


if __name__ == '__main__':
    unittest.main()
