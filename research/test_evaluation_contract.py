import json
import unittest
from pathlib import Path


class EvaluationContractTests(unittest.TestCase):
    def test_unproven_claims_are_blocked(self):
        data = json.loads((Path(__file__).parent / 'evidence/evaluation_contract_v1.json').read_text())
        self.assertEqual(data['status'], 'incomplete')
        self.assertIn('beats transformers', data['claims_blocked'])
        self.assertIn('general symbiosis', data['claims_blocked'])

    def test_score_cannot_be_called_complete(self):
        data = json.loads((Path(__file__).parent / 'evidence/evaluation_contract_v1.json').read_text())
        self.assertLess(data['current_score'], 100)
        self.assertTrue(any(item['status'] == 'pending' for item in data['required_for_next_score_increase']))


if __name__ == '__main__':
    unittest.main()
