import json
import unittest
from pathlib import Path


class NativeCoreMinds14Tests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.data = json.loads((Path(__file__).parent / 'evidence/native_core_minds14_v1.json').read_text())

    def test_target_effect_is_not_passed_to_selector(self):
        self.assertFalse(self.data['protocol']['target_effect_passed_to_selector'])
        self.assertEqual(self.data['protocol']['adapter'], 'none')

    def test_negative_result_is_archived_not_hidden(self):
        self.assertLess(self.data['metrics']['accuracy'], 0.2)
        self.assertGreater(self.data['metrics']['abstentions'], 0)
        self.assertGreater(self.data['metrics']['coverage'], 0.0)

    def test_real_dataset_identity(self):
        self.assertEqual(self.data['dataset']['id'], 'PolyAI/minds14')
        self.assertEqual(self.data['dataset']['holdout'], 98)
        self.assertEqual(len(self.data['ledger']), 98)


if __name__ == '__main__':
    unittest.main()
