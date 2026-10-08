import json
import unittest
from pathlib import Path


class MultiHostTransformerEvidenceTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.data = json.loads((Path(__file__).parent / 'evidence/multi_host_transformer_mintrec_v1.json').read_text())

    def test_same_three_hosts_and_seeds_are_present(self):
        self.assertEqual([h['host_id'] for h in self.data['hosts']], ['S04', 'S05', 'S06'])
        for host in self.data['hosts']:
            self.assertEqual([run['seed'] for run in host['runs']], [11, 23, 47])
            self.assertEqual(host['mean_coverage'], 1.0)

    def test_calibrated_metrics_are_holdout_metrics(self):
        for host in self.data['hosts']:
            for run in host['runs']:
                self.assertIn('selective_metrics', run)
                self.assertIn('confidence_threshold', run)
                self.assertGreater(run['holdout_count'], 0)

    def test_claim_boundary_does_not_call_transformer_sota(self):
        self.assertIn('no SOTA claim', self.data['protocol']['claim_boundary'])


if __name__ == '__main__':
    unittest.main()
