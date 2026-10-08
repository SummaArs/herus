import json
import unittest
from pathlib import Path


class ScoreCalibratedUniversalTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.data=json.loads((Path(__file__).parent/'evidence/score_calibrated_universal_v1.json').read_text())

    def test_two_datasets_and_bounded_claim(self):
        self.assertEqual(len(self.data['results']),2)
        for result in self.data['results']:
            self.assertIn('no SOTA claim',result['protocol']['claim_boundary'])

    def test_calibration_helps_mintrec(self):
        r=next(x for x in self.data['results'] if x['dataset']=='THU-IAR/MIntRec')
        self.assertGreater(r['metrics']['calibrated_universal']['accuracy'],r['metrics']['naive_bayes']['accuracy'])

    def test_regression_is_not_hidden_on_minds14(self):
        r=next(x for x in self.data['results'] if x['dataset']=='PolyAI/minds14')
        self.assertEqual(r['metrics']['calibrated_universal']['accuracy'],r['metrics']['naive_bayes']['accuracy'])


if __name__=='__main__': unittest.main()
