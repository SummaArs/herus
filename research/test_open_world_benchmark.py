import hashlib
import unittest

from open_world_benchmark import (
    CapabilityEvidence,
    Decision,
    HostCase,
    HiddenOpenWorldOracle,
    asa_adapt,
    default_cases,
    evaluate_suite,
    gofai_baseline,
)


class OpenWorldBenchmarkTests(unittest.TestCase):
    def test_closed_baseline_blocks_all_unseen_capabilities(self):
        results = evaluate_suite(default_cases())
        for baseline, _asa in results.values():
            self.assertEqual(baseline.decision, Decision.BASELINE_BLOCKED)
            self.assertIsNone(baseline.proposal)

    def test_asa_discovers_and_verifies_unseen_capabilities(self):
        results = evaluate_suite(default_cases())
        for _baseline, asa in results.values():
            self.assertEqual(asa.decision, Decision.ADAPTED)
            self.assertIsNotNone(asa.proposal)
            self.assertTrue(asa.proposal.verified)
            self.assertFalse(asa.proposal.executed)
            self.assertEqual(asa.proposal.authority, "NONE")
            self.assertFalse(asa.proposal.execution_authorized)
            self.assertTrue(asa.proposal.evidence_digest)
            self.assertEqual(asa.reason, "new_skill_verified_proposal_only")

    def test_transfer_is_not_tied_to_one_host_identity(self):
        first = asa_adapt(HiddenOpenWorldOracle(default_cases()[0]))
        second = asa_adapt(HiddenOpenWorldOracle(default_cases()[1]))
        self.assertNotEqual(first.proposal.host, second.proposal.host)
        self.assertEqual(first.decision, Decision.ADAPTED)
        self.assertEqual(second.decision, Decision.ADAPTED)

    def test_ambiguous_catalog_abstains(self):
        case = default_cases()[0]
        oracle = HiddenOpenWorldOracle(case)
        oracle.enumerate_capabilities = lambda: ("a", "b")
        decision = asa_adapt(oracle)
        self.assertEqual(decision.decision, Decision.REFUSED)
        self.assertEqual(decision.reason, "ambiguous_capability_catalog")

    def test_empty_catalog_abstains(self):
        case = default_cases()[0]
        oracle = HiddenOpenWorldOracle(case)
        oracle.enumerate_capabilities = lambda: ()
        decision = asa_adapt(oracle)
        self.assertEqual(decision.decision, Decision.REFUSED)
        self.assertEqual(decision.reason, "ambiguous_capability_catalog")

    def test_unknown_description_abstains(self):
        case = default_cases()[0]
        oracle = HiddenOpenWorldOracle(case)
        oracle.describe = lambda _capability: (_ for _ in ()).throw(ValueError("unverified"))
        decision = asa_adapt(oracle)
        self.assertEqual(decision.decision, Decision.REFUSED)
        self.assertIsNone(decision.proposal)

    def test_contradictory_description_abstains(self):
        case = default_cases()[0]
        oracle = HiddenOpenWorldOracle(case)
        descriptions = iter((
            "bounded capability for task=sync-context",
            "bounded capability for task=other-task",
        ))
        oracle.describe = lambda _capability: next(descriptions)
        decision = asa_adapt(oracle)
        self.assertEqual(decision.decision, Decision.REFUSED)
        self.assertEqual(decision.reason, "evidence_conflict")

    def test_tampered_evidence_digest_abstains(self):
        case = default_cases()[0]
        oracle = HiddenOpenWorldOracle(case)

        def forged_evidence(_capability):
            return CapabilityEvidence(
                host=case.name,
                capability=case.capability,
                task=case.task,
                description=f"bounded capability for task={case.task}",
                revision=case.revision,
                source="hidden-oracle-v1",
                source_digest="forged",
            )

        oracle.collect_evidence = forged_evidence
        decision = asa_adapt(oracle)
        self.assertEqual(decision.decision, Decision.REFUSED)
        self.assertEqual(decision.reason, "evidence_integrity_or_binding_failed")

    def test_stale_revision_abstains(self):
        case = default_cases()[0]
        oracle = HiddenOpenWorldOracle(case)

        def stale_evidence(_capability):
            evidence = CapabilityEvidence(
                host=case.name,
                capability=case.capability,
                task=case.task,
                description=f"bounded capability for task={case.task}",
                revision=0,
                source="hidden-oracle-v1",
                source_digest="",
            )
            return CapabilityEvidence(**evidence.unsigned(), source_digest=hashlib.sha256(evidence.canonical()).hexdigest())

        oracle.collect_evidence = stale_evidence
        decision = asa_adapt(oracle)
        self.assertEqual(decision.decision, Decision.REFUSED)
        self.assertEqual(decision.reason, "evidence_integrity_or_binding_failed")

    def test_independent_verifier_rejection_abstains(self):
        case = default_cases()[0]
        oracle = HiddenOpenWorldOracle(case)
        oracle.verify = lambda _capability, _task: False
        decision = asa_adapt(oracle)
        self.assertEqual(decision.decision, Decision.REFUSED)
        self.assertEqual(decision.reason, "independent_skill_verification_failed")

    def test_effect_is_never_executed_by_benchmark(self):
        for case in default_cases():
            decision = asa_adapt(HiddenOpenWorldOracle(case))
            self.assertFalse(decision.proposal.executed)
            self.assertFalse(decision.proposal.execution_authorized)

    def test_baseline_known_operator_is_not_called_adaptation(self):
        known = HostCase("known", "haptic", "signal", "pulse")
        decision = gofai_baseline(known)
        self.assertEqual(decision.decision, Decision.ADAPTED)
        self.assertEqual(decision.reason, "known_operator")


if __name__ == "__main__":
    unittest.main()
