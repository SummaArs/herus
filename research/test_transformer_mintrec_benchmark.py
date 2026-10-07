import json
import unittest
from pathlib import Path


class TransformerEvidenceTests(unittest.TestCase):
    def test_real_transformer_evidence_is_temporal_and_complete(self):
        data = json.loads((Path(__file__).parent / 'evidence/transformer_mintrec_v1.json').read_text())
        self.assertEqual(data['protocol']['fit_season'], 'S04')
        self.assertEqual(data['protocol']['validation_season'], 'S05')
        self.assertEqual(data['protocol']['holdout_season'], 'S06')
        self.assertEqual(data['model'], 'google/bert_uncased_L-2_H-128_A-2')
        self.assertGreater(data['runtime_seconds'], 0)
        self.assertGreaterEqual(data['holdout_metrics']['coverage'], 0.99)

    def test_transformer_result_is_not_mislabeled_as_state_of_art(self):
        data = json.loads((Path(__file__).parent / 'evidence/transformer_mintrec_v1.json').read_text())
        self.assertTrue(any('not state of the art' in item for item in data['limits']))


if __name__ == '__main__':
    unittest.main()
