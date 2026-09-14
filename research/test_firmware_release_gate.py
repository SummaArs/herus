import json
from pathlib import Path
import unittest

from firmware_release_gate import evaluate_release


VALID_IDENTITY = {
    "board_revision": "T3-S3-v1",
    "radio_variant": "SX1262-915",
    "schematic_reference": "vendor-schematic-v1",
    "selftest_result": "pass",
}


class FirmwareReleaseGateTests(unittest.TestCase):
    def test_host_proof_is_required_even_with_valid_identity(self):
        decision = evaluate_release(host_proof_passed=False, identity_record=VALID_IDENTITY)
        self.assertEqual(decision.status, "BLOCKED")
        self.assertEqual(decision.reason, "host_only_proof_failed")

    def test_incomplete_identity_blocks_release(self):
        decision = evaluate_release(
            host_proof_passed=True,
            identity_record={**VALID_IDENTITY, "board_revision": "pending"},
        )
        self.assertEqual(decision.status, "BLOCKED")
        self.assertTrue(decision.reason.startswith("preflight:"))

    def test_wrong_radio_blocks_release(self):
        decision = evaluate_release(
            host_proof_passed=True,
            identity_record={**VALID_IDENTITY, "radio_variant": "SX1262-868"},
        )
        self.assertEqual(decision.status, "BLOCKED")
        self.assertEqual(decision.reason, "preflight:radio_variant_not_frozen")

    def test_failed_selftest_blocks_release(self):
        decision = evaluate_release(
            host_proof_passed=True,
            identity_record={**VALID_IDENTITY, "selftest_result": "fail"},
        )
        self.assertEqual(decision.status, "BLOCKED")
        self.assertEqual(decision.reason, "preflight:selftest_not_passed")

    def test_checked_in_manifest_is_not_ready_before_hardware_arrives(self):
        manifest_path = Path(__file__).with_name("evidence") / "hardware_readiness_manifest.json"
        manifest = json.loads(manifest_path.read_text())
        decision = evaluate_release(host_proof_passed=True, identity_record=manifest["identity"])
        self.assertEqual(decision.status, "BLOCKED")
        self.assertEqual(decision.reason, "preflight:physical_identity_incomplete")

    def test_ready_release_is_bench_only(self):
        decision = evaluate_release(host_proof_passed=True, identity_record=VALID_IDENTITY)
        self.assertEqual(decision.status, "READY_FOR_BENCH")
        self.assertIn("OBSERVE", decision.allowed_modes)
        self.assertIn("PROPOSE", decision.allowed_modes)
        self.assertIn("HAPTIC_FEEDBACK", decision.allowed_modes)
        self.assertIn("ACTUATE", decision.forbidden_modes)
        self.assertIn("GRANT_AUTHORITY", decision.forbidden_modes)


if __name__ == "__main__":
    unittest.main()
