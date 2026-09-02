import json
import tempfile
import unittest
from pathlib import Path

from critical_call_path_audit import audit


class CriticalCallPathAuditTests(unittest.TestCase):
    def _profile(self, source="unit.c", allowed=None):
        return {
            "schema": "hcae.call-path.v1",
            "protected_calls": {
                "send": {
                    "source": source,
                    "callee": "send_raw",
                    "allowed_callers": allowed or ["send_assured"],
                }
            },
        }

    def _run(self, source, profile=None):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            (root / "unit.c").write_text(source, encoding="utf-8")
            return audit(profile or self._profile(), root)

    def test_declared_wrapper_is_covered(self):
        results = self._run(
            "int send_raw(void) { return 0; }\n"
            "int send_assured(void) { return send_raw(); }\n"
        )
        self.assertEqual(results[0].status, "COVERED")

    def test_direct_caller_is_uncovered(self):
        results = self._run(
            "int send_raw(void) { return 0; }\n"
            "int bypass(void) { return send_raw(); }\n"
            "int send_assured(void) { return 0; }\n"
        )
        self.assertEqual(results[0].status, "UNCOVERED")
        self.assertEqual(results[0].caller, "bypass")

    def test_missing_source_is_unknown(self):
        results = self._run("", self._profile(source="missing.c"))
        self.assertEqual(results[0].status, "UNKNOWN")
        self.assertEqual(results[0].detail, "source_missing")

    def test_missing_wrapper_is_unknown(self):
        results = self._run("int bypass(void) { return send_raw(); }\n")
        self.assertEqual(results[0].status, "UNKNOWN")
        self.assertIn("allowed_caller_missing", results[0].detail)

    def test_no_observable_call_is_unknown(self):
        results = self._run(
            "int send_raw(void) { return 0; }\n"
            "int send_assured(void) { return 0; }\n"
        )
        self.assertEqual(results[0].status, "UNKNOWN")
        self.assertEqual(results[0].detail, "no_protected_call_found")

    def test_profile_is_serializable(self):
        self.assertEqual(json.loads(json.dumps(self._profile()))["schema"], "hcae.call-path.v1")


if __name__ == "__main__":
    unittest.main()
