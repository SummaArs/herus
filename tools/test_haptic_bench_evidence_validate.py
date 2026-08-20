"""Adversarial tests for the HAP-SEM bench-evidence validator."""
from __future__ import annotations

import copy
import json
import pathlib
import sys

ROOT = pathlib.Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "tools"))

from haptic_bench_evidence_validate import digest_without_record_digest, validate_record

SCHEMA = json.loads((ROOT / "research" / "haptic_bench_evidence_schema.json").read_text(encoding="utf-8"))


def fixture() -> dict:
    record = {
        "gate_id": "haptic-bring-up",
        "execution_origin": "physical_operator",
        "protocol_revision": "haptic-bench-v1",
        "board_revision": "lilygo-t3-s3-v1.3-unverified",
        "adapter_revision": "drv2605l-adapter-v1",
        "mcu_part_marking": "ESP32-S3-unverified",
        "haptic_driver_part_marking": "DRV2605L-unverified",
        "actuator_type": "LRA",
        "actuator_revision": "fixture-lra-v1",
        "haptic_profile_version": "hap-sem-lra-v1",
        "firmware_commit": "0123456789abcdef0123456789abcdef01234567",
        "prove_verdict": "ALL INVARIANTS HOLD",
        "instrument_id": "bench-instrument-unverified",
        "instrument_calibration_due": "2099-01-01",
        "sampling_method": "fixture-only-placeholder",
        "start_timestamp_ms": 1,
        "duration_ms": 2,
        "i2c_clock_hz": 100000,
        "i2c_transactions": 3,
        "i2c_error_count": 0,
        "drive_frequency_hz": 175.0,
        "accel_rms_mps2": 0.1,
        "pulse_duration_us": 1000.0,
        "inter_slot_pause_us": 500.0,
        "supply_voltage_mv": 3300.0,
        "supply_current_ma": 10.0,
        "temperature_c": 25.0,
        "energy_uj": 1.0,
        "latency_us": 100.0,
        "safety_stop": True,
        "fixture_only": True,
        "raw_log_digest": "a" * 64,
        "result": "pass",
        "failure_reason_code": None,
    }
    record["record_digest"] = digest_without_record_digest(record)
    return record


def check(label: str, condition: bool, failures: list[str]) -> None:
    print(f"  {'PASS' if condition else 'FAIL'}  {label}")
    if not condition:
        failures.append(label)


def main() -> int:
    failures: list[str] = []
    valid = fixture()
    check("valid fixture record is accepted", not validate_record(valid, SCHEMA), failures)

    tampered = copy.deepcopy(valid)
    tampered["energy_uj"] = 2.0
    check("tampered numeric evidence breaks the digest", any("record_digest" in e for e in validate_record(tampered, SCHEMA)), failures)

    forbidden = copy.deepcopy(valid)
    forbidden["audio"] = "must never enter a record"
    check("product-content field is rejected", any("forbidden field" in e for e in validate_record(forbidden, SCHEMA)), failures)

    wrong_enum = copy.deepcopy(valid)
    wrong_enum["actuator_type"] = "UNKNOWN"
    check("unknown actuator type is rejected", any("actuator_type" in e for e in validate_record(wrong_enum, SCHEMA)), failures)

    i2c_error = copy.deepcopy(valid)
    i2c_error["i2c_error_count"] = 1
    i2c_error["record_digest"] = digest_without_record_digest(i2c_error)
    check("pass with I2C errors is rejected", any("i2c_error_count" in e for e in validate_record(i2c_error, SCHEMA)), failures)

    no_stop = copy.deepcopy(valid)
    no_stop["safety_stop"] = None
    no_stop["record_digest"] = digest_without_record_digest(no_stop)
    check("pass without observed safety stop is rejected", any("safety_stop" in e for e in validate_record(no_stop, SCHEMA)), failures)

    blocked = copy.deepcopy(valid)
    blocked["result"] = "blocked_by_missing_evidence"
    blocked["failure_reason_code"] = None
    blocked["record_digest"] = digest_without_record_digest(blocked)
    check("blocked record requires an explicit reason", any("failure_reason_code" in e for e in validate_record(blocked, SCHEMA)), failures)

    stub = copy.deepcopy(valid)
    stub["execution_origin"] = "host_stub"
    stub["record_digest"] = digest_without_record_digest(stub)
    check("host stub cannot claim a physical pass", any("host_stub" in e for e in validate_record(stub, SCHEMA)), failures)

    no_hardware = copy.deepcopy(valid)
    no_hardware["execution_origin"] = "blocked_no_hardware"
    no_hardware["result"] = "pass"
    no_hardware["record_digest"] = digest_without_record_digest(no_hardware)
    check("blocked hardware origin cannot claim pass", any("blocked_no_hardware" in e for e in validate_record(no_hardware, SCHEMA)), failures)

    print(f"HAPTIC BENCH EVIDENCE VALIDATOR: {9 - len(failures)} pass, {len(failures)} fail")
    return 1 if failures else 0


if __name__ == "__main__":
    raise SystemExit(main())
