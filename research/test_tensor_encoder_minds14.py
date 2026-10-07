import json
import unittest
from pathlib import Path

class TensorEncoderTests(unittest.TestCase):
    def test_hybrid_result_is_recorded_without_overclaim(self):
        data=json.loads((Path(__file__).parent/'evidence'/'tensor_encoder_minds14_v1.json').read_text())
        self.assertEqual(data['model'],'frozen_multilingual_encoder_plus_tt_head')
        self.assertEqual(data['encoder_parameters'],'frozen_external')
        self.assertEqual(data['parameters'],7424)
        self.assertLess(data['accuracy'],.90)

if __name__=='__main__': unittest.main()
