from __future__ import annotations

from dataclasses import replace
import unittest

from holdout_hosts import holdout_fixtures, PublicHoldoutHost
from research.symbiont_v2 import (
    BudgetLimits,
    Coverage,
    DecisionMode,
    Goal,
    HostObservabilityContract,
    ProbeMode,
    SafetyClaim,
    SkillObservabilityContract,
    SymbiontRuntime,
)
from research.symbiont_v2.sim_hosts import host_a


class ContractedHost:
    def __init__(self, inner):
        self.inner = inner
        self.host_id = inner.host_id

    def resources(self):
        return self.inner.resources()

    def safe_action_space(self):
        return self.inner.safe_action_space()

    def observe(self):
        return self.inner.observe()

    def execute(self, action):
        return self.inner.execute(action)

    def reset(self):
        return self.inner.reset()

    def observability_contract(self):
        return HostObservabilityContract(
            schema="herus-host-observability-v1",
            coverage=Coverage.COMPLETE_DECLARED,
            probe_mode=ProbeMode.NON_MUTATING,
            effect_closure="EXTERNAL_ATTESTATION_REQUIRED",
            closure_evidence_digest="external-host-closure",
            contract_digest="host-contract-v1",
        )


class Stage5SynthesisTests(unittest.TestCase):
    def _runtime_and_skill(self):
        runtime = SymbiontRuntime("stage5-test")
        runtime.discover(host_a())
        skill = runtime.synthesize(Goal.from_dict({"x": 1}))
        self.assertIsNotNone(skill)
        runtime.promote(skill)
        return runtime, skill.skill_id

    def test_strict_blocks_opaque_effect_before_any_probe(self):
        runtime, skill_id = self._runtime_and_skill()
        host = PublicHoldoutHost(holdout_fixtures()[0])
        decision = runtime.propose_transfer_checked(
            skill_id,
            host,
            budget=BudgetLimits(probe_max=4, reset_max=4, step_max=1, cost_max=4),
            mode=DecisionMode.STRICT.value,
        )
        self.assertEqual(decision.proposal, None)
        self.assertEqual(decision.proposal_status.value, "UNSUPPORTED_BY_CONTRACT")
        self.assertEqual(decision.runtime_reason, "OBSERVABILITY_SCHEMA_MISSING")
        self.assertEqual(host.probe_count, 0)
        self.assertEqual(decision.proposal_execute_calls, 0)

    def test_compatibility_preserves_plan_but_downgrades_claim(self):
        runtime, skill_id = self._runtime_and_skill()
        host = PublicHoldoutHost(holdout_fixtures()[0])
        decision = runtime.propose_transfer_checked(skill_id, host, mode="COMPATIBILITY")
        self.assertIsNotNone(decision.proposal)
        self.assertEqual(decision.safety_claim, SafetyClaim.SAFE_BUT_UNPROVEN)
        self.assertEqual(decision.proposal_execute_calls, 0)

    def test_strict_accepts_only_external_contracts(self):
        runtime, skill_id = self._runtime_and_skill()
        skill = runtime.memory.verified_skills[skill_id]
        contract = SkillObservabilityContract(
            coverage_requirement=Coverage.COMPLETE_DECLARED,
            effect_closure="EXTERNAL_ATTESTATION_REQUIRED",
            closure_evidence_digest="external-skill-closure",
            contract_digest="skill-contract-v1",
        )
        runtime.memory.verified_skills[skill_id] = replace(skill, observability_contract=contract)
        host = ContractedHost(host_a())
        decision = runtime.propose_transfer_checked(
            skill_id,
            host,
            budget=BudgetLimits(probe_max=4, reset_max=4, step_max=1, cost_max=4),
            mode="STRICT",
        )
        self.assertEqual(decision.proposal_status.value, "PROPOSED")
        self.assertEqual(decision.safety_claim, SafetyClaim.SAFE_BUT_UNPROVEN)
        self.assertEqual(decision.proposal_execute_calls, 0)

    def test_budget_ledger_is_monotonic_and_unknown_cost_is_not_zero(self):
        limits = BudgetLimits(probe_max=1, reset_max=1, step_max=1, cost_max=None)
        self.assertEqual(limits.validate(), ())
        from research.symbiont_v2.stage5 import initial_ledger, CostStatus
        ledger = initial_ledger("s5", limits)
        next_ledger = ledger.with_probe()
        exhausted = next_ledger.with_probe()
        self.assertGreater(next_ledger.event_seq, ledger.event_seq)
        self.assertEqual(exhausted.budget_state.value, "EXHAUSTED")
        self.assertEqual(ledger.cost_status, CostStatus.UNKNOWN)
        self.assertIsNone(ledger.cost_actual)


if __name__ == "__main__":
    unittest.main()
