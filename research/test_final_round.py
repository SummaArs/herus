from __future__ import annotations
import copy
import unittest
from research.bridge_causal import Budget, CASES, run_case, campaign
from research.bridge_product import CONDITIONS, run_synthetic
from research.social.runner import load_protocol, validate_protocol, main
from research.symbiont_v2.stage5 import BudgetLimits

class FinalRoundTests(unittest.TestCase):
    def test_causal_oracle_ignores_expected_negative_label(self):
        a=run_case(CASES[1]); b=copy.deepcopy(a); b['expected_negative']=False
        self.assertEqual(a['oracle'], b['oracle'])
        self.assertEqual(a['oracle']['verdict'], 'BLOCKED')
    def test_probe_budget_zero_means_zero_probe(self):
        result=run_case(CASES[0], Budget(0,0))
        self.assertEqual(result['runtime']['probes'],0); self.assertEqual(result['runtime']['trace'],[])
    def test_gap_variants_have_same_public_runtime_but_conservative_oracle(self):
        a=run_case(CASES[0]); b=run_case(CASES[1])
        self.assertEqual(a['runtime']['trace'], b['runtime']['trace'])
        self.assertNotEqual(a['oracle'], b['oracle'])
        self.assertEqual(b['oracle']['verdict'], 'BLOCKED')
    def test_campaign_is_rederivable(self):
        result=campaign(); self.assertEqual(len(result['cases']),4)
        for case in result['cases']:
            self.assertTrue(case['result_digest']); self.assertEqual(case['runtime']['probes'], len(case['runtime']['trace']))
    def test_bridge_requires_preview_before_action(self):
        for condition in CONDITIONS:
            result=run_synthetic(condition, 'cancel')
            self.assertEqual(result['state'],'CANCELLED'); self.assertEqual(result['external_effects'],0)
    def test_social_protocol_is_draft_not_ready(self):
        self.assertEqual(validate_protocol(load_protocol()), ())
        self.assertEqual(main()['status'], 'DRAFT_PROTOCOL_NO_HUMAN_DATA')

if __name__ == '__main__': unittest.main()
