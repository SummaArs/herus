from __future__ import annotations

import unittest

from hardware_gate_preflight import evaluate


class HardwareGatePreflightTests(unittest.TestCase):
    def test_pending_record_is_blocked(self) -> None:
        result = evaluate({"board_revision": "pending", "radio_variant": "pending", "schematic_reference": "pending", "selftest_result": "pending"})
        self.assertEqual(result.status, "BLOCKED")
        self.assertEqual(result.reason, "physical_identity_incomplete")

    def test_wrong_radio_is_blocked(self) -> None:
        result = evaluate({"board_revision": "v1.3", "radio_variant": "SX1276-915", "schematic_reference": "sch-v1.3", "selftest_result": "pass"})
        self.assertEqual((result.status, result.reason), ("BLOCKED", "radio_variant_not_frozen"))

    def test_passed_identity_is_ready_only_for_bench(self) -> None:
        result = evaluate({"board_revision": "v1.3", "radio_variant": "SX1262-915", "schematic_reference": "sch-v1.3", "selftest_result": "pass"})
        self.assertEqual((result.status, result.reason), ("READY_FOR_BENCH", "identity_and_selftest_present"))


if __name__ == "__main__":
    unittest.main()
