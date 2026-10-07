import json
import unittest
from pathlib import Path
from uncertainty_analysis import run, wilson

class UncertaintyTests(unittest.TestCase):
    def test_wilson_is_bounded(self):
        x=wilson(82,88)['selective_accuracy_wilson_95'] if False else wilson(82,88)
        self.assertLessEqual(x['low'],x['estimate']); self.assertLessEqual(x['estimate'],x['high'])
        self.assertGreaterEqual(x['low'],0); self.assertLessEqual(x['high'],1)
    def test_report_is_explicit_about_missing_vectors(self):
        d=run(); self.assertIn('no paired bootstrap',d['protocol'])
        self.assertEqual(len(d['results']),7)
        p=Path(__file__).parent/'evidence'/'uncertainty_analysis_v1.json'; self.assertTrue(p.exists())

if __name__=='__main__': unittest.main()
