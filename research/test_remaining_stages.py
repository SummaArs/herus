from __future__ import annotations
import unittest
from research.extended_holdout import run_extended_holdout
from research.social.runner import load_protocol, validate_protocol, main

class RemainingStageTests(unittest.TestCase):
    def test_extended_holdout_is_reserved_and_fail_closed(self):
        result=run_extended_holdout()
        self.assertEqual(len(result['records']),9)
        self.assertEqual(result['classification'],'not_proven')
        self.assertTrue(all(r['proposal_execute_calls']==0 for r in result['records']))
    def test_social_protocol_is_ready_without_fabricated_results(self):
        self.assertEqual(validate_protocol(load_protocol()),())
        result=main(); self.assertEqual(result['status'],'DRAFT_PROTOCOL_NO_HUMAN_DATA'); self.assertIsNone(result['human_results'])

if __name__=='__main__': unittest.main()
