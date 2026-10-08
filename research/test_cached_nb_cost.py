import json
import unittest
from pathlib import Path


class CachedNBCostTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.data=json.loads((Path(__file__).parent/'evidence/cached_nb_cost_banking77_v1.json').read_text())

    def test_predictions_are_equivalent_on_real_sample(self):
        self.assertTrue(self.data['prediction_equivalence']['identical'])
        self.assertEqual(self.data['prediction_equivalence']['mismatches'],0)

    def test_cpu_gain_is_material_but_memory_is_reported(self):
        self.assertGreater(self.data['observed_speedup'],30)
        self.assertGreater(self.data['cached_peak_rss_kb'],500000)

    def test_claim_boundary_is_bounded(self):
        self.assertIn('no general energy or SOTA claim',self.data['protocol']['claim_boundary'])


if __name__=='__main__': unittest.main()
