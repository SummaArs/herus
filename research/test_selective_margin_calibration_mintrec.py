import json
import unittest
from pathlib import Path

class SelectiveCalibrationTests(unittest.TestCase):
    def test_thresholds_are_calibrated_on_s05_and_risk_is_measured_on_s06(self):
        d=json.loads((Path(__file__).parent/'evidence'/'selective_margin_calibration_mintrec_v1.json').read_text())
        self.assertEqual(d['calibration_split'],'S05')
        self.assertEqual(d['holdout_split'],'S06')
        self.assertEqual(d['math']['risk'],'1 - selective_accuracy')
        self.assertGreaterEqual(d['holdout']['0.9']['selective_accuracy'],0.90)
        self.assertGreaterEqual(d['holdout']['0.95']['selective_accuracy'],0.90)
        self.assertLess(d['holdout']['0.95']['coverage'],0.25)
        self.assertEqual(d['holdout']['0.95']['accepted'],88)

if __name__=='__main__': unittest.main()
