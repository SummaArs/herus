from __future__ import annotations

import unittest

from bench_record import sign_bench_record, validate_bench_record


class BenchRecordTests(unittest.TestCase):
    def base(self) -> dict[str, object]:
        return {
            "gate_id": "B1",
            "protocol_revision": "hardware-entry-gate-v1",
            "board_revision": "pending",
            "adapter_revision": "pending",
            "mcu_part_marking": "pending",
            "radio_part_marking": "pending",
            "frequency_profile": "pending",
            "firmware_commit": "pending",
            "prove_verdict": "ALL INVARIANTS HOLD",
            "instrument_id": "none",
            "instrument_calibration_due": None,
            "sampling_method": "metadata-only-prehardware",
            "measurement_unit": "none",
            "start_timestamp_ms": None,
            "duration_ms": None,
            "packets_sent": None,
            "packets_received": None,
            "distance_m": None,
            "rssi_dbm": None,
            "snr_db": None,
            "energy_uj": None,
            "latency_ms": None,
            "interruption_method": None,
            "reset_observed": None,
            "raw_log_digest": None,
            "result": "blocked_by_missing_evidence",
            "failure_reason_code": "PIN_MAP_UNVERIFIED",
        }

    def test_signed_record_is_valid(self) -> None:
        record = sign_bench_record(self.base())
        self.assertEqual(validate_bench_record(record), ())

    def test_forbidden_content_and_tampering_are_rejected(self) -> None:
        record = sign_bench_record(self.base())
        record["message_content"] = "never store this"
        record["result"] = "pass"
        self.assertIn("forbidden:message_content", validate_bench_record(record))
        self.assertIn("record_digest_mismatch", validate_bench_record(record))

    def test_missing_proof_cannot_pass(self) -> None:
        record = sign_bench_record(self.base())
        record["prove_verdict"] = "UNKNOWN"
        record = sign_bench_record(record)
        self.assertIn("prove_baseline_failed", validate_bench_record(record))


if __name__ == "__main__":
    unittest.main()
