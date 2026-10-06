import unittest
import json
from pathlib import Path
from risk_coverage_mintrec import select_threshold

class RiskCoverageTests(unittest.TestCase):
    def test_selects_only_from_calibration_and_can_abstain(self):
        fit = [
            {'text': 'alpha alpha', 'label': 'a'},
            {'text': 'beta beta', 'label': 'b'},
        ]
        calibration = [
            {'text': 'alpha', 'label': 'a'},
            {'text': 'beta', 'label': 'b'},
        ]
        selected = select_threshold(fit, calibration, 0.8)
        self.assertIsNotNone(selected)
        self.assertIn('threshold', selected)
        self.assertGreaterEqual(selected['calibration']['selective_accuracy'], 0.8)

    def test_impossible_precision_has_no_operating_point(self):
        fit = [{'text': 'alpha', 'label': 'a'}, {'text': 'beta', 'label': 'b'}]
        calibration = [{'text': 'alpha', 'label': 'b'}, {'text': 'beta', 'label': 'a'}]
        self.assertIsNone(select_threshold(fit, calibration, 1.0))

    def test_published_evidence_declares_strict_scoring_and_points(self):
        evidence = json.loads((Path(__file__).parent / 'evidence' / 'risk_coverage_mintrec_v1.json').read_text())
        self.assertIn('fit season only', evidence['scoring_rule'])
        self.assertGreaterEqual(len(evidence['points']), 5)

if __name__ == '__main__':
    unittest.main()
