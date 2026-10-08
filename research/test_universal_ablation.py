import json
import unittest
from pathlib import Path


class UniversalAblationTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.data=json.loads((Path(__file__).parent/'evidence/universal_ablation_v1.json').read_text())

    def test_two_real_datasets_are_present(self):
        self.assertEqual(len(self.data['results']),2)
        self.assertEqual({x['dataset']['id'] for x in self.data['results']},{'PolyAI/minds14','THU-IAR/MIntRec'})

    def test_router_beats_fixed_candidates(self):
        for x in self.data['results']:
            self.assertGreater(x['universal_accuracy'],x['always_naive_bayes'])
            self.assertGreater(x['universal_accuracy'],x['random_mean'])

    def test_ablation_does_not_claim_sota(self):
        for x in self.data['results']:
            self.assertIn('not a SOTA claim',x['claim_boundary'])


if __name__=='__main__': unittest.main()
