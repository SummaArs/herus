import json
import unittest
from pathlib import Path


class LlmMinds14BenchmarkTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.data=json.loads((Path(__file__).parent/'evidence/llm_minds14_reference_v1.json').read_text())

    def test_protocol_is_zero_shot_and_fixed(self):
        self.assertEqual(self.data['model'],'gpt-5.5')
        self.assertEqual(self.data['protocol']['mode'],'zero-shot')
        self.assertFalse(self.data['protocol']['training_on_dataset'])

    def test_ledger_is_complete_and_real(self):
        self.assertEqual(self.data['dataset']['id'],'PolyAI/minds14')
        self.assertEqual(len(self.data['ledger']),self.data['dataset']['holdout'])
        self.assertEqual(self.data['dataset']['holdout'],98)

    def test_result_is_not_mislabeled_as_sota(self):
        self.assertIn('not a transformer-equivalent',self.data['protocol']['claim_boundary'])
        self.assertLess(self.data['metrics']['accuracy'],0.1)


if __name__=='__main__': unittest.main()
