from __future__ import annotations
import unittest
from research.stage4.execution_contracts import ExecutionEnvelope, ExecutionStatus, RecoveryAttestation
from research.stage4.executor import ExternalExecutorAdapter
from research.stage4.ledger import AppendOnlyLedger

class ExternalExecutorTests(unittest.TestCase):
    def envelope(self, nonce='1', authority='external-authority'):
        return ExecutionEnvelope('p','a','h','op',1,nonce,10,2,2,authority,0)
    def test_authority_nonce_and_replay(self):
        ex=ExternalExecutorAdapter(); self.assertEqual(ex.execute(self.envelope(authority='runtime')), ExecutionStatus.REJECTED)
        good=self.envelope(); self.assertEqual(ex.execute(good), ExecutionStatus.COMMITTED)
        self.assertEqual(ex.execute(good), ExecutionStatus.REJECTED)
    def test_ttl_and_revocation(self):
        ex=ExternalExecutorAdapter(); ex.clock=10; self.assertEqual(ex.execute(self.envelope()), ExecutionStatus.REJECTED)
        ex=ExternalExecutorAdapter(); ex.revoke(); self.assertEqual(ex.execute(self.envelope()), ExecutionStatus.REJECTED)
    def test_partial_failure_needs_recovery_and_no_retry(self):
        ex=ExternalExecutorAdapter(); self.assertEqual(ex.execute(self.envelope(), partial_at=1), ExecutionStatus.FAILED_PARTIAL)
        att=RecoveryAttestation('op',1,'RESOLVED_PARTIAL','independent', 'ledger',1)
        self.assertEqual(ex.recover(att), ExecutionStatus.RECOVERY_RESOLVED)
    def test_ack_absence_is_unknown(self):
        ex=ExternalExecutorAdapter(); self.assertEqual(ex.execute(self.envelope(), ack=False), ExecutionStatus.UNKNOWN_OUTCOME)
    def test_ledger_is_append_only(self):
        ledger=AppendOnlyLedger('scope'); first=ledger.append({'event':'reserve'}); second=ledger.append({'event':'commit'})
        self.assertNotEqual(first, second); self.assertEqual(len(ledger.events),2); self.assertEqual(ledger.events[1]['seq'],2)

if __name__=='__main__': unittest.main()
