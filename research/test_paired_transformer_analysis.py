import json
import unittest
from pathlib import Path

class PairedTransformerTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.data=json.loads((Path(__file__).parent/'evidence'/'paired_transformer_analysis_v1.json').read_text())
    def test_all_models_share_holdout(self):
        self.assertEqual(self.data['holdout_rows'],386)
        self.assertEqual(set(self.data['models']),{'naive_bayes','herus_context_memory','distilbert_multilingual'})
    def test_comparisons_have_ordered_intervals(self):
        for result in self.data['comparisons'].values():
            self.assertLessEqual(result['ci95_low'],result['ci95_high'])
    def test_herus_selective_claim_is_not_full_coverage_claim(self):
        herus=self.data['models']['herus_context_memory']
        self.assertEqual(herus['selective_accuracy'],1.0)
        self.assertLess(herus['coverage'],0.1)

if __name__=='__main__': unittest.main()
