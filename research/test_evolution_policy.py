import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parent
POLICY = ROOT / "evidence" / "wide_cycle_07" / "evolution_policy.md"
INVENTORY = ROOT / "evidence" / "wide_cycle_07" / "contract_inventory.json"


class EvolutionPolicyTests(unittest.TestCase):
    def test_policy_contains_non_negotiable_questions_and_classes(self):
        text = POLICY.read_text(encoding="utf-8")
        for marker in (
            "# Política de evolução incremental do HERUS",
            "PATCH_NO_CONTRACT_CHANGE",
            "FINITE_ADDITIVE",
            "VERSIONED_MIGRATION",
            "BREAKING_SECURITY_FIX",
            "EXPERIMENTAL_HOST_ONLY",
            "Qual problema real está sendo resolvido?",
            "A mudança é reversível?",
            "Qual custo novo aparece?",
            "`OTHER`, `AMBIGUOUS` e `CONFLICT`",
            "não é um valor absoluto",
            "BLOCKED",
        ):
            with self.subTest(marker=marker):
                self.assertIn(marker, text)

    def test_policy_never_turns_compatibility_into_authority(self):
        text = POLICY.read_text(encoding="utf-8")
        self.assertIn("Sem autoridade", text)
        self.assertIn("confirmação física", text)
        self.assertIn("Rótulos externos não têm autoridade HERUS", text)
        self.assertIn("não entra no núcleo", text)

    def test_inventory_records_non_universal_compatibility(self):
        import json

        inventory = json.loads(INVENTORY.read_text(encoding="utf-8"))
        self.assertEqual(inventory["baseline_commit"], "ff84ce6c16681e0e9115d4c96584d2edeca1ec64")
        classes = {contract["id"]: contract["class"] for contract in inventory["contracts"]}
        self.assertEqual(classes["hcp_wire"], "IMMUTABLE_BASE_WITH_FORWARD_EXTENSION")
        self.assertEqual(classes["semantic_ir"], "VERSIONED_MIGRATABLE")
        self.assertEqual(classes["memory_vault_and_collection"], "VERSIONED_MIGRATABLE")
        self.assertIn("not universally backward compatible", inventory["conclusion"])


if __name__ == "__main__":
    unittest.main()
