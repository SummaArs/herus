"""Canonical bench-record validator for the physical gate.

The validator is intentionally metadata-only: it rejects product content and
requires explicit nulls for unavailable measurements.
"""
from __future__ import annotations

import hashlib
import json
from typing import Any

REQUIRED = {
    "gate_id", "protocol_revision", "board_revision", "adapter_revision",
    "mcu_part_marking", "radio_part_marking", "frequency_profile",
    "firmware_commit", "prove_verdict", "instrument_id",
    "instrument_calibration_due", "sampling_method", "measurement_unit",
    "start_timestamp_ms", "duration_ms", "packets_sent", "packets_received",
    "distance_m", "rssi_dbm", "snr_db", "energy_uj", "latency_ms",
    "interruption_method", "reset_observed", "raw_log_digest", "result",
    "failure_reason_code", "record_digest",
}
FORBIDDEN = {"audio", "transcript", "embedding", "identity", "location", "key", "pair_key", "message_content", "raw_model_prompt", "raw_model_response"}
RESULTS = {"pass", "fail", "blocked_by_missing_evidence"}


def canonical_without_digest(record: dict[str, Any]) -> bytes:
    body = {key: value for key, value in record.items() if key != "record_digest"}
    return json.dumps(body, sort_keys=True, separators=(",", ":"), ensure_ascii=False).encode("utf-8")


def validate_bench_record(record: Any) -> tuple[str, ...]:
    if not isinstance(record, dict):
        return ("record_not_object",)
    issues: list[str] = []
    missing = REQUIRED - set(record)
    issues.extend(f"missing:{key}" for key in sorted(missing))
    issues.extend(f"forbidden:{key}" for key in sorted(FORBIDDEN & set(record)))
    if record.get("prove_verdict") != "ALL INVARIANTS HOLD":
        issues.append("prove_baseline_failed")
    if record.get("result") not in RESULTS:
        issues.append("invalid_result")
    expected = hashlib.sha256(canonical_without_digest(record)).hexdigest()
    if record.get("record_digest") != expected:
        issues.append("record_digest_mismatch")
    return tuple(issues)


def sign_bench_record(record: dict[str, Any]) -> dict[str, Any]:
    signed = dict(record)
    signed["record_digest"] = hashlib.sha256(canonical_without_digest(signed)).hexdigest()
    return signed
