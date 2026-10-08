import json
import unittest
from pathlib import Path


class RiskCoverageCurveTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.data = json.loads((Path(__file__).parent / 'evidence/risk_coverage_curves_v1.json').read_text())

    def test_four_real_domains_are_present(self):
        self.assertEqual(len(self.data['results']), 4)
        self.assertIn(('MInDS-14', 'pt-PT'), {(x['dataset'], x['domain']) for x in self.data['results']})

    def test_curves_have_multiple_operating_points(self):
        for result in self.data['results']:
            self.assertGreaterEqual(len(result['herus_curve']), 10)
            self.assertGreaterEqual(len(result['naive_bayes_curve']), 10)
            for point in result['herus_curve'] + result['naive_bayes_curve']:
                self.assertGreaterEqual(point['coverage'], 0.0)
                self.assertLessEqual(point['coverage'], 1.0)

    def test_claim_boundary_is_not_sota(self):
        self.assertIn('no SOTA claim', self.data['protocol']['claim_boundary'])


if __name__ == '__main__':
    unittest.main()
