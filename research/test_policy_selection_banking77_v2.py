import json
import unittest
from pathlib import Path


class Banking77ConfidenceGateTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.data=json.loads((Path(__file__).parent/'evidence/policy_selection_banking77_v2.json').read_text())

    def test_uncertain_validation_gain_keeps_default(self):
        d=self.data
        ci=d['selection']['validation_bootstrap']
        self.assertLessEqual(ci['lower_95'],0)
        self.assertEqual(d['selection']['chosen_policy'],'default')
        self.assertEqual(d['metrics']['chosen_policy']['accuracy'],d['metrics']['universal_default']['accuracy'])

    def test_gate_is_paired_and_holdout_blind(self):
        self.assertTrue(self.data['protocol']['paired_bootstrap'])
        self.assertFalse(self.data['protocol']['holdout_labels_used_for_selection'])


if __name__=='__main__': unittest.main()
