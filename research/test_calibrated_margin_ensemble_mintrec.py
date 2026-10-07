import json
import unittest
from pathlib import Path

class CalibratedMarginTests(unittest.TestCase):
    def test_weights_are_selected_on_s05_and_evaluated_on_s06(self):
        d=json.loads((Path(__file__).parent/'evidence'/'calibrated_margin_ensemble_mintrec_v1.json').read_text())
        self.assertEqual(d['selection_split'],'S05')
        self.assertEqual(d['holdout_split'],'S06')
        self.assertEqual(d['refit_splits'],['S04','S05'])
        self.assertEqual(d['grid_size'],120)
        self.assertEqual(d['weights'],[0.7,0.4])
        self.assertAlmostEqual(d['holdout']['accuracy'],0.564767,places=5)
        self.assertAlmostEqual(d['holdout']['macro_f1'],0.480153,places=5)

if __name__=='__main__': unittest.main()
