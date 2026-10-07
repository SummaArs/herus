import json
import unittest
from pathlib import Path

class UnsupervisedAlignmentTests(unittest.TestCase):
    def test_alignment_does_not_claim_a_gain(self):
        data=json.loads((Path(__file__).parent/'evidence'/'unsupervised_alignment_minds14_v1.json').read_text())
        self.assertFalse(data['target_labels_used'])
        self.assertEqual(data['holdout']['accuracy'], data['baseline_frozen_encoder']['accuracy'])
        self.assertIn('no measurable improvement', data['conclusion'])

if __name__=='__main__': unittest.main()
