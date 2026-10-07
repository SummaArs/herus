import json
import unittest
from pathlib import Path

class EnsembleRepresentationTests(unittest.TestCase):
    def test_selection_is_leak_free_and_ensemble_result_is_reported(self):
        d=json.loads((Path(__file__).parent/'evidence'/'ensemble_representation_mintrec_v1.json').read_text())
        self.assertEqual(d['selection_split'],'S05')
        self.assertEqual(d['holdout_split'],'S06')
        self.assertEqual(d['chosen'],'word_svm')
        self.assertAlmostEqual(d['holdout_refit']['word_svm']['accuracy'],0.564767,places=5)
        self.assertAlmostEqual(d['holdout_refit']['all_vote']['accuracy'],0.572539,places=5)
        self.assertLess(d['holdout_refit']['all_vote']['accuracy'],d['holdout_refit']['word_logistic']['accuracy'])

if __name__=='__main__': unittest.main()
