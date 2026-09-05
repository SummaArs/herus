import tempfile
import unittest
from pathlib import Path

from critical_c11_structural_audit import audit, load_profile


REAL_PROFILE = Path(__file__).parent / "hcae_profile.json"


def profile(source: str = "firmware/x.c") -> dict:
    return {
        "schema": "hcae.profile.v1",
        "critical_sinks": {
            "x": {
                "source": source,
                "function": "protected_sink",
                "operation": "critical_effect(",
                "guards": ["authority_valid("],
                "structural_guards": [
                    {"id": "authority", "call": "authority_valid", "reject_if": "falsy"}
                ],
            }
        },
    }


class CriticalC11StructuralAuditTests(unittest.TestCase):
    def run_source(self, source: str, custom_profile: dict | None = None):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            path = root / "firmware" / "x.c"
            path.parent.mkdir(parents=True)
            path.write_text(source, encoding="utf-8")
            return audit(custom_profile or profile(), root)

    def test_real_hcae_profile_is_structurally_covered(self) -> None:
        root = Path(__file__).parents[1]
        results = audit(load_profile(REAL_PROFILE), root)
        self.assertEqual(len(results), 13)
        self.assertTrue(all(item.status == "STRUCTURALLY_COVERED" for item in results), results)

    def test_rejecting_guard_dominates_sink(self) -> None:
        results = self.run_source(
            "int authority_valid(void); int critical_effect(void);\n"
            "int protected_sink(void) { if (!authority_valid()) return -1; return critical_effect(); }\n"
        )
        self.assertEqual(results[0].status, "STRUCTURALLY_COVERED")

    def test_non_rejecting_guard_does_not_dominate(self) -> None:
        results = self.run_source(
            "int authority_valid(void); int critical_effect(void);\n"
            "int protected_sink(void) { if (!authority_valid()) { int observed = 1; (void)observed; } return critical_effect(); }\n"
        )
        self.assertEqual(results[0].status, "UNCOVERED")
        self.assertEqual(results[0].detail, "guard_rejection_semantics_not_proven")

    def test_inverted_guard_is_uncovered(self) -> None:
        results = self.run_source(
            "int authority_valid(void); int critical_effect(void);\n"
            "int protected_sink(void) { if (authority_valid()) return -1; return critical_effect(); }\n"
        )
        self.assertEqual(results[0].status, "UNCOVERED")

    def test_guard_after_sink_is_uncovered(self) -> None:
        results = self.run_source(
            "int authority_valid(void); int critical_effect(void);\n"
            "int protected_sink(void) { int rc = critical_effect(); if (!authority_valid()) return -1; return rc; }\n"
        )
        self.assertEqual(results[0].status, "UNCOVERED")
        self.assertIn("non_dominating_guards", results[0].detail)

    def test_function_pointer_sink_is_unknown(self) -> None:
        results = self.run_source(
            "int authority_valid(void); int critical_effect(void);\n"
            "int protected_sink(void) { int (*effect)(void) = critical_effect; if (!authority_valid()) return -1; return effect(); }\n"
        )
        self.assertEqual(results[0].status, "UNKNOWN")
        self.assertEqual(results[0].detail, "direct_sink_call_not_found")

    def test_macro_expanded_sink_is_unknown(self) -> None:
        results = self.run_source(
            "int authority_valid(void); int critical_effect(void);\n"
            "#define EFFECT() critical_effect()\n"
            "int protected_sink(void) { if (!authority_valid()) return -1; return EFFECT(); }\n"
        )
        self.assertEqual(results[0].status, "UNKNOWN")
        self.assertEqual(results[0].detail, "macro_expanded_sink")

    def test_missing_clang_is_unknown(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            path = root / "firmware" / "x.c"
            path.parent.mkdir(parents=True)
            path.write_text(
                "int authority_valid(void); int critical_effect(void);\n"
                "int protected_sink(void) { if (!authority_valid()) return -1; return critical_effect(); }\n",
                encoding="utf-8",
            )
            results = audit(profile(), root, clang="herus-clang-that-does-not-exist")
        self.assertEqual(results[0].status, "UNKNOWN")
        self.assertEqual(results[0].detail, "clang_unavailable")


if __name__ == "__main__":
    unittest.main()
