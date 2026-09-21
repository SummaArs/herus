from __future__ import annotations

import dataclasses
import unittest

from research.asa_round10 import (
    ActionSignature,
    Evidence,
    Planner,
    Simulator,
    SimulatorExecutor,
    Verifier,
    capability_for,
    sample_contract,
)


class AsaRound10Tests(unittest.TestCase):
    def setUp(self) -> None:
        self.contract = sample_contract()
        self.evidence = (
            Evidence.issue(host_id="sim-host", epoch=3, sequence=1, nonce="nonce-round10", action_id="arm", argument="none", before={"armed": 0, "committed": 0}, after={"armed": 1, "committed": 0}),
            Evidence.issue(host_id="sim-host", epoch=3, sequence=2, nonce="nonce-round10", action_id="commit", argument="none", before={"armed": 1, "committed": 0}, after={"armed": 1, "committed": 1}),
        )
        self.verification = Verifier().verify(self.contract, self.evidence, now=1)

    def test_contract_and_independent_attestation(self) -> None:
        self.assertTrue(self.verification.accepted)
        self.assertIsNotNone(self.verification.attestation)
        self.assertEqual(Planner().propose(self.contract), self.contract.actions)

    def test_forged_effect_is_rejected(self) -> None:
        forged = dataclasses.replace(self.evidence[0], after=(('armed', 2), ('committed', 0)))
        result = Verifier().verify(self.contract, (forged, self.evidence[1]), now=1)
        self.assertFalse(result.accepted)
        self.assertEqual(result.reason, "EVIDENCE_SIGNATURE")

    def test_replay_gap_and_mixed_epoch_fail_closed(self) -> None:
        replay = (dataclasses.replace(self.evidence[0], sequence=2), self.evidence[1])
        self.assertEqual(Verifier().verify(self.contract, replay, now=1).reason, "HOST_EPOCH_SEQUENCE")
        mixed = (dataclasses.replace(self.evidence[0], epoch=4), self.evidence[1])
        self.assertEqual(Verifier().verify(self.contract, mixed, now=1).reason, "HOST_EPOCH_SEQUENCE")

    def test_action_collision_is_rejected(self) -> None:
        bad = dataclasses.replace(self.contract, actions=self.contract.actions + (ActionSignature("arm", "other", (), (), ()),), max_steps=3)
        self.assertEqual(Verifier().verify(bad, self.evidence, now=1).reason, "ACTION_COLLISION")

    def test_planner_has_no_executor_authority(self) -> None:
        self.assertFalse(hasattr(Planner(), "execute"))
        self.assertFalse(hasattr(Planner(), "reset"))

    def test_executor_requires_attestation_and_capability(self) -> None:
        result = SimulatorExecutor().run(self.contract, None, None, Simulator({"armed": 0, "committed": 0}), now=1)
        self.assertFalse(result.accepted)
        self.assertEqual(result.code, "ATTESTATION_REQUIRED")

    def test_simulator_executes_only_attested_sequence(self) -> None:
        result = SimulatorExecutor().run(self.contract, self.verification.attestation, capability_for(self.contract), Simulator({"armed": 0, "committed": 0}), now=1)
        self.assertTrue(result.accepted)
        self.assertEqual(dict(result.state), {"armed": 1, "committed": 1})
        self.assertEqual(result.code, "EXECUTED_SIMULATOR_ONLY")

    def test_partial_failure_rolls_back(self) -> None:
        simulator = Simulator({"armed": 0, "committed": 0}, fail_at=1)
        result = SimulatorExecutor().run(self.contract, self.verification.attestation, capability_for(self.contract), simulator, now=1)
        self.assertFalse(result.accepted)
        self.assertTrue(result.rolled_back)
        self.assertEqual(dict(result.state), {"armed": 0, "committed": 0})

    def test_timeout_and_reentrancy_roll_back(self) -> None:
        for simulator, code in ((Simulator({"armed": 0, "committed": 0}, timeout_at=1), "TIMEOUT"), (Simulator({"armed": 0, "committed": 0}, reentrant=True), "REENTRANT")):
            result = SimulatorExecutor().run(self.contract, self.verification.attestation, capability_for(self.contract), simulator, now=1)
            self.assertFalse(result.accepted)
            self.assertEqual(result.code, code)
            self.assertEqual(dict(result.state), {"armed": 0, "committed": 0})

    def test_state_divergence_rolls_back(self) -> None:
        simulator = Simulator({"armed": 0, "committed": 0}, diverge_at=0)
        result = SimulatorExecutor().run(self.contract, self.verification.attestation, capability_for(self.contract), simulator, now=1)
        self.assertFalse(result.accepted)
        self.assertEqual(result.code, "STATE_DIVERGED")
        self.assertTrue(result.rolled_back)

    def test_external_effect_is_blocked_before_mutation(self) -> None:
        action = ActionSignature("radio", "none", (), (), (), external_effect=True)
        bad = dataclasses.replace(self.contract, actions=(action,), max_steps=1)
        with self.assertRaises(ValueError):
            bad.validate()

    def test_expired_attestation_is_rejected(self) -> None:
        result = Verifier().verify(self.contract, self.evidence, now=101)
        self.assertFalse(result.accepted)
        self.assertEqual(result.reason, "EXPIRED")


if __name__ == "__main__":
    unittest.main()
