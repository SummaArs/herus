import json
import unittest
from pathlib import Path

class ReferenceMatrixTests(unittest.TestCase):
    def test_matrix_contains_real_classical_baselines(self):
        data=json.loads((Path(__file__).parent/'evidence'/'ml_reference_matrix_minds14_v1.json').read_text())
        names={row['model'] for row in data['models']}
        self.assertEqual(data['dataset'],'PolyAI/minds14')
        self.assertEqual(data['holdout_rows'],98)
        self.assertTrue({'tfidf_logistic_regression','tfidf_linear_svm','tfidf_knn','tfidf_random_forest'} <= names)
        self.assertGreaterEqual(max(row['accuracy'] for row in data['models']),0.95)
        self.assertIn('no claim of universal superiority from one corpus',data['notes'])

if __name__=='__main__': unittest.main()
