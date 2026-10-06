import json
import unittest
from pathlib import Path

class CrossLanguageEvidenceTests(unittest.TestCase):
    def test_evidence_is_external_and_reports_failure(self):
        data=json.loads((Path(__file__).parent/'evidence'/'minds14_cross_language_v1.json').read_text())
        self.assertEqual(data['dataset']['fit_language'], 'en-US')
        self.assertEqual(data['dataset']['holdout_language'], 'pt-PT')
        self.assertEqual(data['dataset']['shared_labels'], 14)
        self.assertLess(data['holdout']['consensus_selective']['selective_accuracy'], 0.5)
        self.assertIn('single language pair', data['limits'])

if __name__=='__main__': unittest.main()
