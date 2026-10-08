import json
import unittest
from pathlib import Path


class UniversalMIntRecBenchmarkTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.data=json.loads((Path(__file__).parent/'evidence/universal_mintrec_v1.json').read_text())

    def test_temporal_protocol_and_holdout(self):
        d=self.data['dataset']
        self.assertEqual(d['fit_season'],'S04')
        self.assertEqual(d['calibration_season'],'S05')
        self.assertEqual(d['holdout_season'],'S06')
        self.assertEqual(d['holdout'],386)
        self.assertFalse(self.data['protocol']['holdout_labels_used_for_decision'])

    def test_universal_beats_nb_on_replication(self):
        self.assertGreater(self.data['metrics']['universal']['accuracy'],self.data['metrics']['naive_bayes']['accuracy'])
        self.assertEqual(len(self.data['ledger']),386)

    def test_claim_is_bounded(self):
        self.assertIn('no general SOTA claim',self.data['protocol']['claim_boundary'])


if __name__=='__main__': unittest.main()
