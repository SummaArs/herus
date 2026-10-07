import json
import unittest
from pathlib import Path

class TensorNetworkTests(unittest.TestCase):
    def test_tensor_baseline_and_rank_sweep_are_real(self):
        root=Path(__file__).parent/'evidence'
        base=json.loads((root/'tensor_network_minds14_v1.json').read_text())
        sweep=json.loads((root/'tensor_network_rank_sweep_minds14_v1.json').read_text())
        self.assertEqual(base['dataset'],'PolyAI/minds14')
        self.assertEqual(base['parameters'],1152)
        self.assertEqual(len(sweep['ranks']),5)
        self.assertEqual(sweep['ranks'][-1]['rank'],32)
        self.assertLess(sweep['ranks'][-1]['accuracy'],.92)
        self.assertLess(sweep['ranks'][-1]['infer_ms'],1.0)

if __name__=='__main__': unittest.main()
