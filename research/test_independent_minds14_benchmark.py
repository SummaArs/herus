import json
import unittest
from pathlib import Path


class IndependentMinds14Tests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.data = json.loads((Path(__file__).parent / 'evidence/independent_minds14_v1.json').read_text())

    def test_real_independent_dataset_and_holdout(self):
        self.assertEqual(self.data['dataset']['id'], 'PolyAI/minds14')
        self.assertEqual(self.data['dataset']['config'], 'pt-PT')
        self.assertEqual(self.data['dataset']['holdout'], 98)
        self.assertEqual(len(self.data['ledger']), 98)

    def test_classical_baseline_is_not_hidden(self):
        self.assertGreater(self.data['baselines']['naive_bayes']['accuracy'], self.data['adapter']['metrics']['accuracy'])
        self.assertGreater(self.data['adapter']['metrics']['selective_accuracy'], self.data['baselines']['naive_bayes']['selective_accuracy'])

    def test_transformer_seeds_are_archived(self):
        self.assertEqual([r['seed'] for r in self.data['transformer']], [11, 23, 47])
        for run in self.data['transformer']:
            self.assertEqual(len(run['ledger']), 98)


if __name__ == '__main__':
    unittest.main()
