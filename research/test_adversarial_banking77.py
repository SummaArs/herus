import json
import unittest
from pathlib import Path


class AdversarialBanking77Tests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.data=json.loads((Path(__file__).parent/'evidence/adversarial_banking77_v1.json').read_text())

    def test_real_holdout_and_selection_blindness(self):
        self.assertEqual(self.data['split']['test'],3080)
        self.assertFalse(self.data['protocol']['test_labels_used_for_selection'])

    def test_current_policy_does_not_pass_robustness_gate(self):
        self.assertEqual(self.data['protocol']['claim_boundary'],'text stress-test only; not certified label-preserving robustness or OOD')
        self.assertTrue(all(v['coverage']==1.0 for v in self.data['attacks'].values()))
        self.assertGreater(self.data['attacks']['irrelevant_prefix']['wrong_label_flip_rate'],0.01)
        self.assertGreater(self.data['attacks']['typo']['wrong_label_flip_rate'],0.01)


if __name__=='__main__': unittest.main()
