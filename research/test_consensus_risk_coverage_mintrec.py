import unittest
from consensus_risk_coverage_mintrec import choose_threshold

class ConsensusRiskCoverageTests(unittest.TestCase):
    def test_calibration_selects_a_consensus_operating_point(self):
        fit = [
            {'text': 'alpha alpha', 'label': 'a'},
            {'text': 'beta beta', 'label': 'b'},
            {'text': 'alpha', 'label': 'a'},
            {'text': 'beta', 'label': 'b'},
        ]
        calibration = [
            {'text': 'alpha', 'label': 'a'},
            {'text': 'beta', 'label': 'b'},
        ]
        selected = choose_threshold(fit, calibration, 0.5)
        self.assertIsNotNone(selected)
        self.assertGreaterEqual(selected[2]['selective_accuracy'], 0.5)

if __name__ == '__main__':
    unittest.main()
