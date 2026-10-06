import json
import unittest
from pathlib import Path

class Minds14EvidenceTests(unittest.TestCase):
    def test_real_evidence_has_external_holdout_and_all_methods(self):
        path = Path(__file__).parent / 'evidence' / 'minds14_real_benchmark_v1.json'
        data = json.loads(path.read_text())
        self.assertEqual(data['dataset']['id'], 'PolyAI/minds14')
        self.assertGreater(data['dataset']['holdout'], 0)
        self.assertIn('naive_bayes', data['methods'])
        self.assertIn('consensus_selective', data['methods'])
        self.assertIn('no speaker-independent split verified', data['limits'])

if __name__ == '__main__':
    unittest.main()
