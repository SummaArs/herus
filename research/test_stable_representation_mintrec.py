import json
import unittest
from pathlib import Path

class StableRepresentationTests(unittest.TestCase):
    def test_stability_selection_does_not_use_s06(self):
        d=json.loads((Path(__file__).parent/'evidence'/'stable_representation_mintrec_v1.json').read_text())
        self.assertEqual(d['selection'],'mean_and_worst_temporal_validation')
        self.assertEqual(d['holdout_split'],'S06')
        self.assertEqual(d['chosen'],'word_tfidf_svm')
        self.assertEqual(d['validation_rows'],[186,1272])
        self.assertAlmostEqual(d['holdout_refit']['word_tfidf_svm']['accuracy'],0.564767,places=5)

if __name__=='__main__': unittest.main()
