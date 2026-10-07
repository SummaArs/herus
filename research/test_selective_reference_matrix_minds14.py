import json
import unittest
from pathlib import Path

class SelectiveReferenceTests(unittest.TestCase):
    def test_all_reference_models_have_holdout_metrics(self):
        data=json.loads((Path(__file__).parent/'evidence'/'selective_reference_matrix_minds14_v1.json').read_text())
        self.assertEqual(data['target_precision'],0.95)
        self.assertEqual(data['holdout_rows'],98)
        for result in data['models'].values():
            self.assertIn('holdout',result)
            self.assertGreaterEqual(result['holdout']['coverage'],0)
            self.assertLessEqual(result['holdout']['coverage'],1)
        self.assertGreater(data['models']['random_forest']['holdout']['selective_accuracy'],0.98)

if __name__=='__main__': unittest.main()
