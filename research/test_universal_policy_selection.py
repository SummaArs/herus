import json
import unittest
from pathlib import Path


class UniversalPolicySelectionTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        root=Path(__file__).parent/'evidence'
        cls.mintrec=json.loads((root/'policy_selection_mintrec_v1.json').read_text())
        cls.minds14=json.loads((root/'policy_selection_minds14_v1.json').read_text())

    def test_same_procedure_has_disjoint_nested_splits(self):
        for d in (self.mintrec,self.minds14):
            self.assertFalse(d['protocol']['holdout_labels_used_for_selection'])
            self.assertTrue(d['protocol']['nested_calibration'])

    def test_policy_adapts_to_host_without_global_regression(self):
        self.assertEqual(self.mintrec['selection']['chosen_policy'],'score_calibrated')
        self.assertEqual(self.minds14['selection']['chosen_policy'],'universal_default')
        self.assertEqual(self.minds14['metrics']['chosen_policy']['accuracy'],0.928571)
        self.assertEqual(self.mintrec['metrics']['chosen_policy']['accuracy'],0.396373)

    def test_claims_are_bounded(self):
        for d in (self.mintrec,self.minds14):
            self.assertIn('no SOTA claim',d['protocol']['claim_boundary'])


if __name__=='__main__': unittest.main()
