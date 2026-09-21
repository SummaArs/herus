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


class AsaRound11Tests(unittest.TestCase):
    def setUp(self) -> None:
        self.contract = sample_contract()
        self.evidence = (
            Evidence.issue(contract=self.contract, sequence=1, action_id="arm", argument="none", before={"armed": 0, "committed": 0}, after={"armed": 1, "committed": 0}),
            Evidence.issue(contract=self.contract, sequence=2, action_id="commit", argument="none", before={"armed": 1, "committed": 0}, after={"armed": 1, "committed": 1}),
        )
        self.verification = Verifier().verify(self.contract, self.evidence, now=1)
        self.token = capability_for(self.contract)

    def test_valid_contract_has_independent_attestation(self) -> None:
        self.assertTrue(self.verification.accepted)
        self.assertIsNotNone(self.verification.attestation)
        self.assertEqual(Planner().propose(self.contract), self.contract.actions)

    def test_forged_effect_is_rejected(self) -> None:
        forged = dataclasses.replace(self.evidence[0], after=(("armed", 2), ("committed", 0)))
        result = Verifier().verify(self.contract, (forged, self.evidence[1]), now=1)
        self.assertFalse(result.accepted)
        self.assertEqual(result.reason, "EVIDENCE_SIGNATURE")

    def test_replay_gap_and_mixed_epoch_fail_closed(self) -> None:
        self.assertEqual(Verifier().verify(self.contract, (dataclasses.replace(self.evidence[0], sequence=2), self.evidence[1]), now=1).reason, "IDENTITY_SEQUENCE")
        self.assertEqual(Verifier().verify(self.contract, (dataclasses.replace(self.evidence[0], epoch=4), self.evidence[1]), now=1).reason, "IDENTITY_SEQUENCE")

    def test_contract_collision_unknown_key_and_non_integer_fail(self) -> None:
        collision = dataclasses.replace(self.contract, actions=self.contract.actions + (ActionSignature("arm", "other", (), (), ()),), max_steps=3)
        self.assertEqual(Verifier().verify(collision, self.evidence, now=1).reason, "ACTION_COLLISION")
        unknown = dataclasses.replace(self.contract, actions=(ActionSignature("bad", "none", (("unknown", 0),), (), ()),), max_steps=1)
        self.assertEqual(Verifier().verify(unknown, (), now=1).reason, "UNKNOWN_STATE_KEY")
        self.assertEqual(Verifier().verify(dataclasses.replace(self.contract, schema_version=1.0), self.evidence, now=1).reason, "VERSION_TYPE")

    def test_contract_binding_rejects_cross_contract_evidence(self) -> None:
        other = dataclasses.replace(self.contract, host_digest="other-digest")
        self.assertEqual(Verifier().verify(other, self.evidence, now=1).reason, "IDENTITY_SEQUENCE")

    def test_expiry_is_half_open_and_nan_is_rejected(self) -> None:
        self.assertEqual(Verifier().verify(self.contract, self.evidence, now=100).reason, "EXPIRED")
        self.assertEqual(Verifier().verify(dataclasses.replace(self.contract, expires_at=float("nan")), self.evidence, now=1).reason, "EXPIRY_TYPE")

    def test_planner_has_no_executor_authority(self) -> None:
        self.assertFalse(hasattr(Planner(), "execute"))
        self.assertFalse(hasattr(Planner(), "reset"))

    def test_executor_requires_attestation_and_capability(self) -> None:
        result = SimulatorExecutor().run(self.contract, None, None, Simulator({"armed": 0, "committed": 0}), now=1)
        self.assertFalse(result.accepted)
        self.assertEqual(result.code, "ATTESTATION_REQUIRED")

    def test_simulator_executes_only_attested_sequence(self) -> None:
        result = SimulatorExecutor().run(self.contract, self.verification.attestation, self.token, Simulator({"armed": 0, "committed": 0}), now=1)
        self.assertTrue(result.accepted)
        self.assertEqual(dict(result.state), {"armed": 1, "committed": 1})
        self.assertEqual(result.code, "EXECUTED_SIMULATOR_ONLY")

    def test_replay_is_consumed(self) -> None:
        executor = SimulatorExecutor()
        first = executor.run(self.contract, self.verification.attestation, self.token, Simulator({"armed": 0, "committed": 0}), now=1)
        second = executor.run(self.contract, self.verification.attestation, self.token, Simulator({"armed": 0, "committed": 0}), now=1)
        self.assertTrue(first.accepted)
        self.assertEqual(second.code, "REPLAY")

    def test_expiry_extension_and_forged_token_are_rejected(self) -> None:
        extended = dataclasses.replace(self.verification.attestation, expires_at=1000)
        self.assertEqual(SimulatorExecutor().run(self.contract, extended, self.token, Simulator({"armed": 0, "committed": 0}), now=101).code, "SCOPE")
        forged = dataclasses.replace(self.token, proof="forged")
        self.assertEqual(SimulatorExecutor().run(self.contract, self.verification.attestation, forged, Simulator({"armed": 0, "committed": 0}), now=1).code, "CAPABILITY_PROOF")

    def test_partial_failure_timeout_reentrancy_and_divergence_roll_back(self) -> None:
        schedules = ((Simulator({"armed": 0, "committed": 0}, fail_at=1), "PARTIAL_FAILURE"), (Simulator({"armed": 0, "committed": 0}, timeout_at=1), "TIMEOUT"), (Simulator({"armed": 0, "committed": 0}, reentrant=True), "REENTRANT"), (Simulator({"armed": 0, "committed": 0}, diverge_at=0), "STATE_DIVERGED"))
        for simulator, code in schedules:
            result = SimulatorExecutor().run(self.contract, self.verification.attestation, self.token, simulator, now=1)
            self.assertEqual(result.code, code)
            self.assertTrue(result.rolled_back)
            self.assertEqual(dict(result.state), {"armed": 0, "committed": 0})

    def test_external_effect_is_blocked_before_mutation(self) -> None:
        bad = dataclasses.replace(self.contract, actions=(ActionSignature("radio", "none", (), (), (), external_effect=True),), max_steps=1)
        self.assertEqual(Verifier().verify(bad, (), now=1).reason, "EXTERNAL_EFFECT_BLOCKED")

    def test_simulator_type_and_malformed_now_fail_closed(self) -> None:
        result = SimulatorExecutor().run(self.contract, self.verification.attestation, self.token, object(), now=1)
        self.assertEqual(result.code, "SIMULATOR_TYPE")
        result = SimulatorExecutor().run(self.contract, self.verification.attestation, self.token, Simulator({"armed": 0, "committed": 0}), now=float("nan"))
        self.assertEqual(result.code, "NOW_TYPE")

    def test_attestation_and_verifier_identity_are_bound(self) -> None:
        bad = dataclasses.replace(self.verification.attestation, verifier_id="attacker")
        self.assertEqual(SimulatorExecutor().run(self.contract, bad, self.token, Simulator({"armed": 0, "committed": 0}), now=1).code, "VERIFIER_NOT_ALLOWED")


if __name__ == "__main__":
    unittest.main()
