import json
import unittest
from pathlib import Path
from paired_holdout_analysis import run

class PairedHoldoutTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls): cls.data=run()
    def test_ledger_covers_holdout(self):
        self.assertEqual(len(self.data['prediction_ledger']),self.data['holdout_rows'])
    def test_interval_is_ordered(self):
        x=self.data['paired_comparison']; self.assertLessEqual(x['ci95_low'],x['ci95_high'])
    def test_evidence_is_saved(self):
        self.assertTrue((Path(__file__).parent/'evidence'/'paired_holdout_analysis_v1.json').exists())

if __name__=='__main__': unittest.main()
