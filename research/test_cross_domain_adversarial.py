from __future__ import annotations

import unittest

from adaptation_reconcile import Observation, reconcile
from domain_contract import Domain, EffectLevel, contract_for
from domain_readiness import Evidence, Readiness, highest_allowed


class CrossDomainAdversarialTests(unittest.TestCase):
    def full_evidence(self, **changes):
        values = dict(data_provenance=True, hidden_tests=True, adversarial_tests=True, drift_policy=True, rollback=True, human_review=True, external_authority=True, physical_validation=True)
        values.update(changes)
        return Evidence(**values)

    def test_finance_evidence_cannot_promote_robotics_effect(self) -> None:
        finance = contract_for(Domain.FINANCE_SANDBOX)
        self.assertFalse(finance.permits(EffectLevel.CANARY, {"host_digest", "data_digest", "sandbox_id"}))
        self.assertNotIn("MOVE_ROBOT", finance.vocabulary)

    def test_mixed_host_observations_are_blocked(self) -> None:
        observations = [
            Observation("a", 1, "host-a", "latency", 20.0, "evidence-a"),
            Observation("b", 1, "host-b", "latency", 20.0, "evidence-a"),
        ]
        result = reconcile(observations)
        self.assertEqual((result.status, result.reason), ("BLOCKED", "scope_conflict"))

    def test_physical_validation_does_not_promote_critical_domain(self) -> None:
        readiness = highest_allowed("critical", self.full_evidence(physical_validation=True))
        self.assertEqual(readiness, Readiness.SHADOW)

    def test_missing_rollback_blocks_server_canary(self) -> None:
        readiness = highest_allowed("server", self.full_evidence(rollback=False))
        self.assertEqual(readiness, Readiness.SHADOW)


if __name__ == "__main__":
    unittest.main()
