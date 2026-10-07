import json
import unittest
from pathlib import Path
from conformal_mintrec import run

class ConformalTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls): cls.data=run()
    def test_all_targets_are_reported(self): self.assertEqual(len(self.data['results']),4)
    def test_sets_never_claim_guaranteed_exact_accuracy(self):
        for x in self.data['results']:
            self.assertIn('test_set_coverage',x); self.assertIn('singleton_accuracy',x)
    def test_evidence_exists(self): self.assertTrue((Path(__file__).parent/'evidence'/'conformal_mintrec_v1.json').exists())
if __name__=='__main__': unittest.main()
