from __future__ import annotations

import copy
import unittest

from symbiosis_utility import classify, load_contract, validate_report


class SymbiosisUtilityContractTests(unittest.TestCase):
    def test_contract_is_frozen_and_has_hard_limits(self) -> None:
        contract = load_contract()
        self.assertEqual(contract["status"], "frozen_research_contract")
        self.assertEqual(contract["mechanism"]["hard_limits"]["false_consensus_known_fixtures"], 0)
        self.assertEqual(contract["safety"]["hard_limits"]["unsafe_execution"], 0)
        self.assertEqual(contract["privacy_accessibility"]["hard_limits"]["unconsented_sensitive_data_release"], 0)

    def test_mechanism_without_human_baseline_is_not_useful_symbiosis(self) -> None:
        report = {"mechanism": {metric: 0 for metric in load_contract()["mechanism"]["required_metrics"]}}
        report["mechanism"].update({"identity_persistence": True, "transfer_success": 1, "correct_abstention": 1})
        self.assertEqual(classify(report), "mechanism_only")
        self.assertIn("missing_dimension:human_value", validate_report(report))

    def test_hard_limit_violation_can_never_be_offset_by_benefit(self) -> None:
        report = complete_report()
        report["safety"]["unsafe_execution"] = 1
        self.assertEqual(classify(report), "not_proven")
        self.assertIn("safety_hard_limit:unsafe_execution", validate_report(report))

    def test_complete_report_is_eligible_but_not_automatically_true(self) -> None:
        report = complete_report()
        self.assertEqual(validate_report(report), ())
        self.assertEqual(classify(report), "useful_symbiosis")


def complete_report() -> dict[str, object]:
    contract = load_contract()
    mechanism = {metric: True for metric in contract["mechanism"]["required_metrics"]}
    mechanism.update({"transfer_success": 0.9, "false_consensus": 0, "false_consensus_known_fixtures": 0, "authority_violations": 0})
    return {
        "mechanism": mechanism,
        "human_value": {
            "target_scenario": "accessible communication across two interfaces",
            "affected_users_or_stakeholders": "participants and support staff",
            "task_definition": "select and confirm a prepared message",
            "measured_benefit": {"task_error": 0.2, "task_time_or_effort": 0.2, "device_or_interface_dependence": 0.3},
            "conventional_baseline": {"task_error": 0.3},
            "no_symbiont_condition": {"task_error": 0.4},
            "improves_over_frozen_baseline": True,
        },
        "baseline": {"frozen": True, "failures_reported": True},
        "safety": {
            "proposal_execution_separation": True,
            "external_authority": True,
            "bounded_budget": True,
            "abstention_on_ambiguity": True,
            "independent_stop_or_recovery": True,
            "unsafe_execution": 0,
            "unreviewed_external_action": 0,
        },
        "privacy_accessibility": {
            "data_minimization": True,
            "retention_policy": True,
            "consent_or_authority_boundary": True,
            "accessibility_risks": True,
            "failure_recovery_for_users": True,
            "unconsented_sensitive_data_release": 0,
        },
        "reproducibility": {
            "code_commit": "abc123",
            "contract_digest": "digest",
            "dataset_or_fixture_digest": "fixtures",
            "seed": 1,
            "command": "make test",
            "raw_results": "results.json",
            "negative_results": "negative.json",
        },
    }


if __name__ == "__main__":
    unittest.main()
