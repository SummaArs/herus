import json
import tempfile
import unittest
from pathlib import Path

from critical_assurance_case import load_case, run_case
from critical_assurance_certificate import AssuranceVerdict


CASE = Path(__file__).parent / "evidence" / "memory_vault_assurance_case.json"


class MemoryVaultAssuranceCaseTests(unittest.TestCase):
    def test_real_memory_vault_contract_is_assured(self) -> None:
        certificate = run_case(CASE)
        self.assertEqual(certificate.verdict, AssuranceVerdict.ASSURED)
        self.assertEqual(certificate.abstract_verification.verdict.value, "VERIFIED")
        self.assertEqual(certificate.concrete_verification.verdict.value, "VERIFIED")
        self.assertEqual(certificate.machine_refinement.verdict.value, "REFINED")
        self.assertEqual(certificate.policy_refinement.verdict.value, "REFINED")

    def test_removed_persistence_coverage_blocks(self) -> None:
        data = load_case(CASE)
        data["call_paths"][0]["status"] = "UNCOVERED"
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "case.json"
            path.write_text(json.dumps(data), encoding="utf-8")
            certificate = run_case(path)
        self.assertEqual(certificate.verdict, AssuranceVerdict.BLOCKED)

    def test_concrete_block_action_mutation_is_counterexample(self) -> None:
        data = load_case(CASE)
        data["concrete_policy"][1]["action"] = "RETAIN_STATE"
        data["concrete"]["transitions"].append({
            "state": "MEMORY_VAULT_UNINITIALIZED",
            "input_symbol": "BACKEND_LOAD_FAIL",
            "action": "RETAIN_STATE",
            "next_state": "MEMORY_VAULT_UNINITIALIZED",
        })
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "case.json"
            path.write_text(json.dumps(data), encoding="utf-8")
            certificate = run_case(path)
        self.assertEqual(certificate.verdict, AssuranceVerdict.COUNTEREXAMPLE)

    def test_source_scope_is_explicit(self) -> None:
        data = load_case(CASE)
        self.assertEqual(data["source_contract"]["header"], "firmware/core/memory_vault.h")
        self.assertEqual(data["source_contract"]["implementation"], "firmware/core/memory_vault.c")


if __name__ == "__main__":
    unittest.main()
