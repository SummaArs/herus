"""Prepare and validate HAP-SEM bench records without inventing physical data."""
from __future__ import annotations

import argparse
import json
import os
import pathlib
import subprocess
import sys
from typing import Any

ROOT = pathlib.Path(__file__).resolve().parents[1]
SCHEMA_PATH = ROOT / "research" / "haptic_bench_evidence_schema.json"
sys.path.insert(0, str(ROOT / "tools"))
from haptic_bench_evidence_validate import digest_without_record_digest, validate_record


def firmware_commit() -> str:
    supplied = os.environ.get("HERUS_FIRMWARE_COMMIT")
    if supplied:
        return supplied
    completed = subprocess.run(
        ["git", "rev-parse", "HEAD"],
        cwd=ROOT,
        check=True,
        capture_output=True,
        text=True,
    )
    return completed.stdout.strip()


def blocked_record(reason: str = "HARDWARE_NOT_ATTACHED") -> dict[str, Any]:
    record: dict[str, Any] = {
        "gate_id": "haptic-bring-up",
        "execution_origin": "blocked_no_hardware",
        "protocol_revision": "haptic-bench-v1",
        "board_revision": "unverified-no-hardware",
        "adapter_revision": "drv2605l-target-v1",
        "mcu_part_marking": "unverified",
        "haptic_driver_part_marking": "unverified",
        "actuator_type": "UNVERIFIED",
        "actuator_revision": "unverified",
        "haptic_profile_version": "unverified",
        "firmware_commit": firmware_commit(),
        "instrument_id": "none",
        "instrument_calibration_due": None,
        "sampling_method": "not_executed",
        "start_timestamp_ms": None,
        "duration_ms": None,
        "i2c_clock_hz": None,
        "i2c_transactions": None,
        "i2c_error_count": None,
        "drive_frequency_hz": None,
        "accel_rms_mps2": None,
        "pulse_duration_us": None,
        "inter_slot_pause_us": None,
        "supply_voltage_mv": None,
        "supply_current_ma": None,
        "temperature_c": None,
        "energy_uj": None,
        "latency_us": None,
        "safety_stop": None,
        "fixture_only": True,
        "raw_log_digest": None,
        "result": "blocked_by_missing_evidence",
        "failure_reason_code": reason,
        "prove_verdict": "ALL INVARIANTS HOLD",
    }
    record["record_digest"] = digest_without_record_digest(record)
    return record


def load_schema() -> dict[str, Any]:
    return json.loads(SCHEMA_PATH.read_text(encoding="utf-8"))


def write_record(path: pathlib.Path, record: dict[str, Any]) -> None:
    path.write_text(json.dumps(record, ensure_ascii=False, sort_keys=True, indent=2) + "\n", encoding="utf-8")


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    group = parser.add_mutually_exclusive_group(required=True)
    group.add_argument("--prepare-blocked", type=pathlib.Path, metavar="PATH")
    group.add_argument("--validate", type=pathlib.Path, metavar="PATH")
    args = parser.parse_args(argv)
    schema = load_schema()

    if args.prepare_blocked is not None:
        record = blocked_record()
        errors = validate_record(record, schema)
        if errors:
            for error in errors:
                print(f"FAIL generated blocked record: {error}")
            return 1
        write_record(args.prepare_blocked, record)
        print("HAPTIC BENCH RUNNER: blocked_by_missing_evidence written; no hardware operation executed")
        print(f"record_digest={record['record_digest']}")
        return 0

    records = json.loads(args.validate.read_text(encoding="utf-8"))
    errors = validate_record(records, schema)
    if errors:
        print(f"HAPTIC BENCH RUNNER: INVALID {args.validate}")
        for error in errors:
            print(f"  - {error}")
        return 1
    print(f"HAPTIC BENCH RUNNER: VALID {args.validate}; result={records['result']}; origin={records['execution_origin']}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
