import json
import unittest
from pathlib import Path

class WideTensorTests(unittest.TestCase):
    def test_rank_64_reaches_svm_accuracy_with_lower_inference_cost(self):
        data=json.loads((Path(__file__).parent/'evidence'/'tensor_network_wide_minds14_v1.json').read_text())
        rank64=next(x for x in data['ranks'] if x['rank']==64)
        self.assertGreaterEqual(rank64['accuracy'],0.959184)
        self.assertGreater(rank64['macro_f1'],0.9555)
        self.assertLess(rank64['infer_ms'],2.0)
        self.assertLess(rank64['parameters'],20000)

if __name__=='__main__': unittest.main()
