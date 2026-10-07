import json
import unittest
from pathlib import Path

class MultilingualEncoderTests(unittest.TestCase):
    def test_frozen_encoder_uses_unlabeled_target_holdout(self):
        data=json.loads((Path(__file__).parent/'evidence'/'multilingual_encoder_minds14_v1.json').read_text())
        self.assertTrue(data['frozen'])
        self.assertEqual(data['holdout_language'], 'pt-PT')
        self.assertEqual(data['fit_language'], 'en-US')
        self.assertEqual(data['holdout_rows'], 604)
        self.assertGreater(data['holdout']['accuracy'], 0.4)
        self.assertIn('no target labels used', data['limits'])

if __name__=='__main__': unittest.main()
