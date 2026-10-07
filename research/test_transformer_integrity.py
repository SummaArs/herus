import unittest
from pathlib import Path

class TransformerIntegrityTests(unittest.TestCase):
    def test_benchmark_source_is_nonempty_and_configurable(self):
        p=Path(__file__).parent/'transformer_mintrec_benchmark.py'
        text=p.read_text()
        self.assertGreater(len(text),1000)
        self.assertIn('HERUS_TRANSFORMER_MODEL',text)
        self.assertIn('holdout_season',text)

if __name__=='__main__': unittest.main()
