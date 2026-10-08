import json
import unittest
from pathlib import Path


class CombinedCoverageBootstrapTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.data = json.loads((Path(__file__).parent / 'evidence/combined_coverage_bootstrap_v1.json').read_text())

    def test_two_real_datasets_are_present(self):
        self.assertEqual({x['dataset'] for x in self.data['results']}, {'MIntRec', 'MInDS-14'})
        self.assertEqual(len(self.data['results']), 4)

    def test_every_result_has_bootstrap_and_exact_subset(self):
        for result in self.data['results']:
            self.assertGreater(result['comparison']['examples'], 0)
            self.assertEqual(result['comparison']['iterations'], 4000)
            self.assertGreaterEqual(result['herus_coverage'], 0.0)
            self.assertLessEqual(result['herus_coverage'], 1.0)

    def test_claim_boundary_is_selection_not_general_superiority(self):
        self.assertIn('selection effect', self.data['protocol']['claim_boundary'])


if __name__ == '__main__':
    unittest.main()
