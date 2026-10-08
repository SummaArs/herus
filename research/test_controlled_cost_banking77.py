import json
import unittest
from pathlib import Path


class ControlledCostBankingTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.data=json.loads((Path(__file__).parent/'evidence/controlled_cost_banking77_v1.json').read_text())

    def test_same_predictions_and_real_holdout(self):
        self.assertTrue(self.data['relative']['predictions_equal'])
        self.assertEqual(self.data['uncached']['holdout'],3080)
        self.assertEqual(self.data['feature_cache_ablation']['holdout'],3080)

    def test_feature_cache_is_rejected_when_slower(self):
        self.assertEqual(self.data['relative']['decision'],'reject_feature_cache_as_default')
        self.assertGreater(self.data['relative']['cached_over_uncached_wall_ratio'],1.0)
        self.assertGreater(self.data['relative']['cached_over_uncached_cpu_ratio'],1.0)


if __name__=='__main__': unittest.main()
