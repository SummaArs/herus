import json
import unittest
from pathlib import Path

class MIntRecTensorTests(unittest.TestCase):
    def test_temporal_holdout_and_reference_gap_are_recorded(self):
        data=json.loads((Path(__file__).parent/'evidence'/'tensor_network_mintrec_v1.json').read_text())
        self.assertEqual(data['holdout_season'],'S06')
        self.assertEqual(data['holdout_rows'],386)
        rank64=next(x for x in data['ranks'] if x['rank']==64)
        self.assertAlmostEqual(rank64['accuracy'],0.42228,places=5)
        self.assertLess(rank64['accuracy'],0.492228)

if __name__=='__main__': unittest.main()
