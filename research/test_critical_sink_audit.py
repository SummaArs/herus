from __future__ import annotations

from pathlib import Path
import tempfile
import unittest

from critical_sink_audit import audit, load_profile


class CriticalSinkAuditTests(unittest.TestCase):
    def test_real_profile_covers_declared_sinks_lexically(self) -> None:
        root = Path(__file__).resolve().parents[1]
        profile = load_profile(root / "research" / "hcae_profile.json")
        results = audit(profile, root)
        self.assertEqual(len(results), 13)
        self.assertTrue(all(item.status == "COVERED" for item in results), results)

    def test_missing_source_is_unknown(self) -> None:
        profile = {"critical_sinks": {"x": {"source": "missing.c", "function": "f", "operation": "sink(", "guards": ["guard("]}}}
        result = audit(profile, Path("/tmp"))[0]
        self.assertEqual(result.status, "UNKNOWN")
        self.assertEqual(result.detail, "source_missing")

    def test_removed_guard_is_uncovered(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            (root / "x.c").write_text("int f(void) { return sink(); }\n", encoding="utf-8")
            profile = {"critical_sinks": {"x": {"source": "x.c", "function": "f", "operation": "sink(", "guards": ["guard("]}}}
            result = audit(profile, root)[0]
            self.assertEqual(result.status, "UNCOVERED")
            self.assertEqual(result.detail, "missing_guard:guard(")

    def test_semantic_mismatch_guard_is_required(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            (root / "x.c").write_text(
                "int f(void) { if (access->id != c->id) return 1; return sink(); }\n",
                encoding="utf-8",
            )
            profile = {"critical_sinks": {"x": {
                "source": "x.c", "function": "f", "operation": "sink(",
                "guards": ["return 1;"],
                "semantic_guards": [{"kind": "rejects_mismatch", "expression": "access->id != c->id"}],
            }}}
            self.assertEqual(audit(profile, root)[0].status, "COVERED")

    def test_inverted_semantic_mismatch_guard_is_uncovered(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            (root / "x.c").write_text(
                "int f(void) { if (access->id == c->id) return 1; return sink(); }\n",
                encoding="utf-8",
            )
            profile = {"critical_sinks": {"x": {
                "source": "x.c", "function": "f", "operation": "sink(",
                "guards": ["return 1;"],
                "semantic_guards": [{"kind": "rejects_mismatch", "expression": "access->id != c->id"}],
            }}}
            result = audit(profile, root)[0]
            self.assertEqual(result.status, "UNCOVERED")
            self.assertIn("missing_semantic_guard", result.detail)

    def test_guard_after_operation_is_uncovered(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            (root / "x.c").write_text("int f(void) { sink(); guard(); return 0; }\n", encoding="utf-8")
            profile = {"critical_sinks": {"x": {"source": "x.c", "function": "f", "operation": "sink(", "guards": ["guard("]}}}
            result = audit(profile, root)[0]
            self.assertEqual(result.status, "UNCOVERED")
            self.assertEqual(result.detail, "guard_after_operation:guard(")


if __name__ == "__main__":
    unittest.main()
