import json
import unittest
from pathlib import Path


class PolicySelectionMIntRecTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.data=json.loads((Path(__file__).parent/'evidence/policy_selection_mintrec_v1.json').read_text())

    def test_nested_split_is_disjoint_and_bounded(self):
        d=self.data
        self.assertEqual(d['selection']['calibration_split'],{'tune':636,'validation':636})
        self.assertFalse(d['protocol']['holdout_labels_used_for_selection'])
        self.assertIn('no SOTA claim',d['protocol']['claim_boundary'])

    def test_validation_selects_calibrated_policy(self):
        s=self.data['selection']
        self.assertEqual(s['chosen_policy'],'score_calibrated')
        self.assertGreater(s['validation_accuracy']['score_calibrated'],s['validation_accuracy']['universal_default'])

    def test_selected_policy_beats_default_on_holdout(self):
        m=self.data['metrics']
        self.assertGreater(m['chosen_policy']['accuracy'],m['universal_default']['accuracy'])


if __name__=='__main__': unittest.main()
