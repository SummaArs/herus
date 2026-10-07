import json
import unittest
from pathlib import Path

class AdaptiveRepresentationTests(unittest.TestCase):
    def test_selection_is_calibrated_before_temporal_holdout(self):
        p=Path(__file__).parent/'evidence'/'adaptive_representation_mintrec_v2.json'
        d=json.loads(p.read_text())
        self.assertEqual(d['selection_split'],'S05')
        self.assertEqual(d['holdout_split'],'S06')
        self.assertEqual(d['refit_rows'],1838)
        self.assertEqual(d['chosen_by_calibration'],'word_tfidf_svm')
        self.assertAlmostEqual(d['chosen_holdout']['accuracy'],0.564767,places=5)
        self.assertGreater(d['chosen_holdout']['accuracy'],0.492228)
        self.assertNotEqual(d['chosen_by_calibration'],'holdout_oracle')

if __name__=='__main__': unittest.main()
