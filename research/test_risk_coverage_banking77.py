import json
import unittest
from pathlib import Path

class RiskCoverageEvidenceTests(unittest.TestCase):
    def test_policy_does_not_claim_robustness(self):
        p=Path(__file__).parent/'evidence/risk_coverage_banking77_v1.json'
        d=json.loads(p.read_text())
        self.assertFalse(d['protocol']['holdout_labels_used_for_policy'])
        self.assertIn('not SOTA', d['protocol']['claim_boundary'])
        self.assertGreaterEqual(d['decision']['risk_upper'], d['decision']['calibration_risk'])

if __name__=='__main__': unittest.main()
