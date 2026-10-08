import unittest
from universal_llm_admission import assess


class UniversalLlmAdmissionTests(unittest.TestCase):
    def test_holdout_only_candidate_is_blocked(self):
        result=assess()
        self.assertEqual(result['status'],'BLOCKED')
        self.assertIn('missing_independent_calibration_ledger',result['reasons'])
        self.assertFalse(result['router_admitted'])

    def test_claim_boundary_is_explicit(self):
        self.assertIn('cannot be routed',assess()['claim_boundary'])


if __name__=='__main__': unittest.main()
