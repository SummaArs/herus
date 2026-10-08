import json
import unittest
from pathlib import Path


class Banking77PolicySelectionTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.data=json.loads((Path(__file__).parent/'evidence/policy_selection_banking77_v1.json').read_text())

    def test_real_third_host_protocol(self):
        d=self.data
        self.assertEqual(d['dataset']['labels'],77)
        self.assertEqual(d['dataset']['holdout'],3080)
        self.assertTrue(d['protocol']['train_test_separate'])
        self.assertFalse(d['protocol']['holdout_labels_used_for_selection'])

    def test_validation_choice_does_not_hide_holdout_regression(self):
        d=self.data
        self.assertEqual(d['selection']['chosen_policy'],'score_calibrated')
        self.assertGreater(d['selection']['validation_accuracy']['score_calibrated'],d['selection']['validation_accuracy']['universal_default'])
        self.assertGreater(d['metrics']['universal_default']['accuracy'],d['metrics']['score_calibrated']['accuracy'])

    def test_claim_is_bounded(self):
        self.assertIn('no SOTA claim',self.data['protocol']['claim_boundary'])


if __name__=='__main__': unittest.main()
