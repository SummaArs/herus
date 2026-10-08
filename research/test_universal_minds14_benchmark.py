import json
import unittest
from pathlib import Path


class UniversalMinds14BenchmarkTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.data=json.loads((Path(__file__).parent/'evidence/universal_minds14_v1.json').read_text())

    def test_protocol_has_disjoint_calibration_and_holdout(self):
        self.assertEqual(self.data['dataset']['calibration'],89)
        self.assertEqual(self.data['dataset']['holdout'],98)
        self.assertFalse(self.data['protocol']['holdout_labels_used_for_decision'])

    def test_universal_ledger_contains_paired_predictions(self):
        ledger=self.data['ledger']
        self.assertEqual(len(ledger),98)
        for row in ledger:
            self.assertIn('universal_prediction',row)
            self.assertIn('nb_correct',row)
            self.assertIn('centroid_correct',row)

    def test_claim_boundary_is_conservative(self):
        self.assertIn('no general SOTA claim',self.data['protocol']['claim_boundary'])
        self.assertGreaterEqual(self.data['metrics']['universal']['accuracy'],self.data['metrics']['naive_bayes']['accuracy'])


if __name__=='__main__': unittest.main()
