import json
import unittest
from pathlib import Path


class InvarianceEvidenceTests(unittest.TestCase):
    def test_tradeoff_is_explicit(self):
        data=json.loads((Path(__file__).parent/'evidence/invariance_banking77_v1.json').read_text())
        self.assertFalse(data['protocol']['holdout_labels_used'])
        self.assertTrue(data['protocol']['batch_inference'])
        self.assertLess(data['attacks']['irrelevant_prefix']['wrong_label_flip_rate'], .194)
        self.assertGreater(data['attacks']['typo']['gate_abstention_rate'], .7)
        self.assertIn('ablation only', data['protocol']['claim_boundary'])


if __name__=='__main__': unittest.main()
