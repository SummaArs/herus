import json
import tempfile
import unittest
from pathlib import Path

from critical_assurance_case import load_case, run_case
from critical_assurance_certificate import AssuranceVerdict


CASE = Path(__file__).parent / "evidence" / "reference_assurance_case.json"


class CriticalAssuranceCaseTests(unittest.TestCase):
    def test_reference_case_is_assured(self) -> None:
        certificate = run_case(CASE)
        self.assertEqual(certificate.verdict, AssuranceVerdict.ASSURED)
        self.assertEqual(len(certificate.evidence_digest), 64)
        self.assertEqual(certificate.abstract_verification.verdict.value, "VERIFIED")

    def test_missing_scope_is_rejected(self) -> None:
        data = load_case(CASE)
        del data["assurance_scope"]
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "case.json"
            path.write_text(json.dumps(data), encoding="utf-8")
            with self.assertRaisesRegex(ValueError, "invalid_or_missing_assurance_scope"):
                load_case(path)

    def test_formal_scope_rejects_c11_evidence(self) -> None:
        data = load_case(CASE)
        data["source_contract"] = {"implementation": "firmware/core/interaction.c"}
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "case.json"
            path.write_text(json.dumps(data), encoding="utf-8")
            with self.assertRaisesRegex(ValueError, "formal_only_case_declares_c11_evidence"):
                load_case(path)

    def test_c11_scope_requires_complete_evidence_declarations(self) -> None:
        data = load_case(CASE)
        data["assurance_scope"] = "c11-bound"
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "case.json"
            path.write_text(json.dumps(data), encoding="utf-8")
            with self.assertRaisesRegex(ValueError, "c11_bound_case_missing_source_contract"):
                load_case(path)

    def test_mutated_path_is_blocked(self) -> None:
        data = load_case(CASE)
        data["call_paths"][0]["status"] = "UNCOVERED"
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "case.json"
            path.write_text(json.dumps(data), encoding="utf-8")
            certificate = run_case(path)
        self.assertEqual(certificate.verdict, AssuranceVerdict.BLOCKED)

    def test_mutated_policy_is_counterexample(self) -> None:
        data = load_case(CASE)
        data["concrete_policy"][-1]["action"] = "retain"
        data["concrete"]["transitions"].append({
            "state": "c_hold", "input_symbol": "packet_loss",
            "action": "retain", "next_state": "c_hold",
        })
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "case.json"
            path.write_text(json.dumps(data), encoding="utf-8")
            certificate = run_case(path)
        self.assertEqual(certificate.verdict, AssuranceVerdict.COUNTEREXAMPLE)
        self.assertEqual(certificate.reason, "policy_refinement_counterexample")

    def test_invalid_schema_is_rejected(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "bad.json"
            path.write_text(json.dumps({"schema": "wrong"}), encoding="utf-8")
            with self.assertRaises(ValueError):
                load_case(path)


if __name__ == "__main__":
    unittest.main()
