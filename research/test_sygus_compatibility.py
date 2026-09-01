from __future__ import annotations

from pathlib import Path
import tempfile
import unittest

from sygus_compatibility import inspect_file, inventory, summarize


class SyGuSCompatibilityTests(unittest.TestCase):
    def test_single_lia_function_is_accepted(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "ok.sl"
            path.write_text(
                "(set-logic LIA)\n(synth-fun f ((x Int)) Int)\n"
                "(declare-var x Int)\n(constraint (= (f x) x))\n",
                encoding="utf-8",
            )
            row = inspect_file(path)
            self.assertTrue(row.supported)
            self.assertEqual(row.logic, "LIA")

    def test_unsupported_features_are_rejected(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "no.sl"
            path.write_text(
                "(set-logic LIA)\n(synth-fun f ((x Int)) Int)\n"
                "(declare-var x Int)\n(constraint (ite (= x 0) 0 1))\n",
                encoding="utf-8",
            )
            row = inspect_file(path)
            self.assertFalse(row.supported)
            self.assertIn("unsupported_marker:ite", row.reasons)

    def test_missing_root_is_fail_closed(self) -> None:
        rows = inventory(Path("/does/not/exist"))
        self.assertEqual(summarize(rows), {"files": 1, "supported": 0, "rejected": 1})
        self.assertIn("missing_root", rows[0].reasons)


if __name__ == "__main__":
    unittest.main()
