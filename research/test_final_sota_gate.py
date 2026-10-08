import unittest
from final_sota_gate import decide


class FinalSotaGateTests(unittest.TestCase):
    def test_current_mintrec_evidence_is_blocked(self):
        result = decide({'independent_dataset_evidence': False, 'adapter_only': True, 'matched_risk_coverage': False, 'bootstrap_complete': True})
        self.assertEqual(result['status'], 'BLOCKED')
        self.assertFalse(result['claim_allowed'])
        self.assertIn('independent_dataset_missing', result['blockers'])
        self.assertIn('herus_core_not_isolated', result['blockers'])

    def test_complete_protocol_is_only_eligible_for_review(self):
        result = decide({'independent_dataset_evidence': True, 'current_state_of_art_baseline': True, 'adapter_only': False, 'matched_risk_coverage': True, 'bootstrap_complete': True})
        self.assertEqual(result['status'], 'ELIGIBLE_FOR_INDEPENDENT_REVIEW')
        self.assertFalse(result['claim_allowed'])


if __name__ == '__main__':
    unittest.main()
