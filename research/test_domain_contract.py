from __future__ import annotations

import unittest

from domain_contract import Domain, EffectLevel, contract_for


class DomainContractTests(unittest.TestCase):
    def test_finance_is_sandbox_only(self) -> None:
        contract = contract_for(Domain.FINANCE_SANDBOX)
        evidence = {"host_digest", "data_digest", "sandbox_id"}
        self.assertTrue(contract.permits(EffectLevel.SIMULATE, evidence))
        self.assertFalse(contract.permits(EffectLevel.CANARY, evidence))
        self.assertIn("PLACE_ORDER", contract.forbidden_effects)

    def test_critical_requires_human_review(self) -> None:
        contract = contract_for(Domain.CRITICAL)
        base = {"host_digest", "sensor_digest"}
        self.assertFalse(contract.permits(EffectLevel.SHADOW, base))
        self.assertTrue(contract.permits(EffectLevel.SHADOW, base | {"human_review"}))
        self.assertFalse(contract.permits(EffectLevel.ACTIVE, base | {"human_review"}))

    def test_unknown_vocabulary_is_not_contract_compatible(self) -> None:
        contract = contract_for(Domain.ROBOTICS)
        self.assertNotIn("PLACE_ORDER", contract.vocabulary)
        self.assertTrue(contract.permits(EffectLevel.SIMULATE, {"host_digest", "world_digest", "safety_check"}))


if __name__ == "__main__":
    unittest.main()
