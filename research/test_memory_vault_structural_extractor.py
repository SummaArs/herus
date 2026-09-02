import json
import tempfile
import unittest
from pathlib import Path

from memory_vault_structural_extractor import ExtractionVerdict, compare_source

ROOT = Path(__file__).parents[1]
SOURCE = ROOT / "firmware/core/memory_vault.c"
CASE = ROOT / "research/evidence/memory_vault_assurance_case.json"


class MemoryVaultStructuralExtractorTests(unittest.TestCase):
    def _mutant(self, mutate):
        source = SOURCE.read_text(encoding="utf-8")
        source = mutate(source)
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "memory_vault.c"
            path.write_text(source, encoding="utf-8")
            return compare_source(path, CASE)

    def test_real_source_matches_declared_obligations(self):
        result = compare_source(SOURCE, CASE)
        self.assertEqual(result.verdict, ExtractionVerdict.EXTRACTED_MATCH)
        self.assertEqual(result.reason, "declared_obligations_observed")
        self.assertEqual(result.missing, ())
        self.assertEqual(result.unsupported, ())
        self.assertEqual(len(result.source_digest), 64)

    def test_removed_guard_is_divergence(self):
        result = self._mutant(lambda source: source.replace("if (!auth_valid(auth, card))", "if (0)", 1))
        self.assertEqual(result.verdict, ExtractionVerdict.DIVERGENCE)
        self.assertIn("guard:auth_valid", result.missing)

    def test_sink_before_guard_is_divergence(self):
        def mutate(source):
            old = "    if (!auth_valid(auth, card)) {"
            injected = "    v->cfg.storage.store_sealed(v->cfg.storage.ctx, blob);\n"
            return source.replace(old, injected + old, 1)
        result = self._mutant(mutate)
        self.assertEqual(result.verdict, ExtractionVerdict.DIVERGENCE)
        self.assertEqual(result.reason, "guard_after_persistence_sink")

    def test_missing_function_is_divergence(self):
        result = self._mutant(lambda source: source.replace("int memory_vault_open(", "int removed_memory_vault_open(", 1))
        self.assertEqual(result.verdict, ExtractionVerdict.DIVERGENCE)
        self.assertIn("function:memory_vault_open", result.missing)

    def test_unbalanced_body_is_unknown(self):
        result = self._mutant(lambda source: source.replace("\n}\n\nconst memory_vault_metrics_t *memory_vault_metrics", "\n\nconst memory_vault_metrics_t *memory_vault_metrics", 1))
        self.assertEqual(result.verdict, ExtractionVerdict.UNKNOWN)
        self.assertIn("unbalanced_body:memory_vault_erase", result.unsupported)

    def test_unsupported_preprocessor_is_unknown(self):
        result = self._mutant(lambda source: "#define MEMORY_VAULT_DYNAMIC(x) x\n" + source)
        self.assertEqual(result.verdict, ExtractionVerdict.UNKNOWN)
        self.assertIn("ambiguous_preprocessor", result.unsupported)

    def test_invalid_case_schema_is_unknown(self):
        with tempfile.TemporaryDirectory() as directory:
            case = Path(directory) / "bad.json"
            case.write_text(json.dumps({"schema": "wrong"}), encoding="utf-8")
            result = compare_source(SOURCE, case)
        self.assertEqual(result.verdict, ExtractionVerdict.UNKNOWN)
        self.assertEqual(result.reason, "invalid_case_schema")


if __name__ == "__main__":
    unittest.main()
