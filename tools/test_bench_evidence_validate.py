#!/usr/bin/env python3
"""Tests for the privacy-preserving HERUS bench evidence validator."""
from __future__ import annotations

import copy
import hashlib
import json
import pathlib
import subprocess
import sys
import tempfile

ROOT = pathlib.Path(__file__).resolve().parents[1]
SCHEMA = ROOT / "research" / "hardware_bench_evidence_schema.json"
VALIDATOR = ROOT / "tools" / "bench_evidence_validate.py"


def digest(record: dict) -> str:
    unsigned = dict(record)
    unsigned.pop("record_digest", None)
    encoded = (json.dumps(unsigned, ensure_ascii=False, sort_keys=True, separators=(",", ":")) + "\n").encode()
    return hashlib.sha256(encoded).hexdigest()


def valid_record() -> dict:
    return {
        "gate_id": "radio-bring-up",
        "protocol_revision": "r0.2",
        "board_revision": "T3-S3-V1.3-H595",
        "adapter_revision": "bench-01",
        "mcu_part_marking": "ESP32-S3FH4R2",
        "radio_part_marking": "SX1262",
        "frequency_profile": "declared-region-profile",
        "firmware_commit": "0123456789abcdef0123456789abcdef01234567",
        "prove_verdict": "ALL INVARIANTS HOLD",
        "instrument_id": "power-001",
        "instrument_calibration_due": "2027-01-01",
        "sampling_method": "shunt-1khz-triggered",
        "measurement_unit": "uJ",
        "start_timestamp_ms": 1000,
        "duration_ms": 250,
        "packets_sent": 20,
        "packets_received": 20,
        "distance_m": 1.0,
        "rssi_dbm": -42.5,
        "snr_db": 9.25,
        "energy_uj": 1200.0,
        "latency_ms": 15.0,
        "interruption_method": None,
        "reset_observed": None,
        "raw_log_digest": None,
        "result": "pass",
        "failure_reason_code": None,
    }


def run(record: dict) -> int:
    with tempfile.TemporaryDirectory(prefix="herus-bench-schema-") as directory:
        path = pathlib.Path(directory) / "record.json"
        path.write_text(json.dumps(record, ensure_ascii=False, sort_keys=True), encoding="utf-8")
        result = subprocess.run([sys.executable, str(VALIDATOR), "--schema", str(SCHEMA), str(path)], text=True, capture_output=True, check=False)
        print(result.stdout, end="")
        print(result.stderr, end="")
        return result.returncode


def expect(name: str, record: dict, accepted: bool) -> bool:
    code = run(record)
    passed = (code == 0) == accepted
    print(f"  {'PASS' if passed else 'FAIL'} {name}")
    return passed


def main() -> int:
    record = valid_record()
    record["record_digest"] = digest(record)
    results = [
        expect("valid canonical numeric record is accepted", record, True),
    ]

    forbidden = copy.deepcopy(record)
    forbidden["message_content"] = "must never be logged"
    forbidden["record_digest"] = digest(forbidden)
    results.append(expect("forbidden product field is rejected", forbidden, False))

    bad_digest = copy.deepcopy(record)
    bad_digest["record_digest"] = "0" * 64
    results.append(expect("tampered record digest is rejected", bad_digest, False))

    blocked_without_reason = copy.deepcopy(record)
    blocked_without_reason["result"] = "blocked_by_missing_evidence"
    blocked_without_reason["record_digest"] = digest(blocked_without_reason)
    results.append(expect("blocked record without a reason is rejected", blocked_without_reason, False))

    unknown = copy.deepcopy(record)
    unknown["private_note"] = "unknown fields are not part of evidence"
    unknown["record_digest"] = digest(unknown)
    results.append(expect("unknown schema field is rejected", unknown, False))

    if all(results):
        print("BENCH EVIDENCE VALIDATOR INVARIANTS HOLD")
        return 0
    print("BENCH EVIDENCE VALIDATOR TESTS FAILED")
    return 1


if __name__ == "__main__":
    sys.exit(main())
