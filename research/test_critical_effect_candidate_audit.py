import copy
import tempfile
import unittest
from pathlib import Path

from critical_effect_candidate_audit import audit, load_json


ROOT = Path(__file__).parents[1]
PROFILE = load_json(ROOT / "research/hcae_profile.json")
DISPOSITIONS = load_json(ROOT / "research/critical_effect_dispositions.json")


class CriticalEffectCandidateAuditTests(unittest.TestCase):
    def test_real_surface_has_no_unreviewed_candidate(self) -> None:
        results = audit(PROFILE, DISPOSITIONS, ROOT)
        self.assertGreaterEqual(len(results), 20)
        self.assertFalse(any(item.status == "REVIEW_REQUIRED" for item in results), results)
        self.assertTrue(any(item.status == "PROFILED" for item in results))
        self.assertTrue(any(item.status == "REVIEWED_INTERNAL" for item in results))

    def test_new_sensitive_call_requires_review(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            core = root / "firmware/core"
            net = root / "firmware/net"
            core.mkdir(parents=True)
            net.mkdir(parents=True)
            (core / "new_effect.c").write_text(
                "int publish_event(void);\nint new_path(void) { return publish_event(); }\n",
                encoding="utf-8",
            )
            results = audit({"critical_sinks": {}}, {"schema": "herus.critical-effect-dispositions.v1", "reviewed_internal": []}, root)
        self.assertEqual(len(results), 1)
        self.assertEqual(results[0].status, "REVIEW_REQUIRED")

    def test_disposition_is_exact_to_source_function_and_operation(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            core = root / "firmware/core"
            net = root / "firmware/net"
            core.mkdir(parents=True)
            net.mkdir(parents=True)
            (core / "new_effect.c").write_text(
                "int publish_event(void);\nint new_path(void) { return publish_event(); }\n",
                encoding="utf-8",
            )
            dispositions = {
                "schema": "herus.critical-effect-dispositions.v1",
                "reviewed_internal": [{
                    "source": "firmware/core/new_effect.c",
                    "function": "different_path",
                    "operation": "publish_event",
                    "reason": "wrong_callsite_must_not_suppress",
                }],
            }
            results = audit({"critical_sinks": {}}, dispositions, root)
        self.assertEqual(results[0].status, "REVIEW_REQUIRED")

    def test_exact_reviewed_internal_disposition_is_not_promoted(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            core = root / "firmware/core"
            net = root / "firmware/net"
            core.mkdir(parents=True)
            net.mkdir(parents=True)
            (core / "new_effect.c").write_text(
                "int publish_event(void);\nint new_path(void) { return publish_event(); }\n",
                encoding="utf-8",
            )
            dispositions = {
                "schema": "herus.critical-effect-dispositions.v1",
                "reviewed_internal": [{
                    "source": "firmware/core/new_effect.c",
                    "function": "new_path",
                    "operation": "publish_event",
                    "reason": "pure_test_fixture",
                }],
            }
            results = audit({"critical_sinks": {}}, dispositions, root)
        self.assertEqual(results[0].status, "REVIEWED_INTERNAL")
        self.assertNotEqual(results[0].status, "PROFILED")


if __name__ == "__main__":
    unittest.main()
