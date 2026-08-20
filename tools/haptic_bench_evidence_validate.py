"""Validate private numeric HAP-SEM/DRV2605L bench evidence."""
from __future__ import annotations

import argparse
import hashlib
import json
import math
import pathlib
import re
import sys
from typing import Any

DEFAULT_SCHEMA = pathlib.Path(__file__).resolve().parents[1] / "research" / "haptic_bench_evidence_schema.json"
SHA256_RE = re.compile(r"^[0-9a-f]{64}$")
COMMIT_RE = re.compile(r"^[0-9a-f]{40}([0-9a-f]{24})?$")
FORBIDDEN = {
    "audio", "transcript", "embedding", "identity", "location", "key",
    "pair_key", "message_content", "raw_model_prompt", "raw_model_response",
    "free_text", "participant_id",
}


def canonical_bytes(value: Any) -> bytes:
    return (json.dumps(value, ensure_ascii=False, sort_keys=True, separators=(",", ":")) + "\n").encode("utf-8")


def digest_without_record_digest(record: dict[str, Any]) -> str:
    unsigned = dict(record)
    unsigned.pop("record_digest", None)
    return hashlib.sha256(canonical_bytes(unsigned)).hexdigest()


def find_forbidden(value: Any, path: str = "$") -> str | None:
    if isinstance(value, dict):
        for key, child in value.items():
            if key in FORBIDDEN:
                return f"forbidden field {path}.{key}"
            found = find_forbidden(child, f"{path}.{key}")
            if found:
                return found
    elif isinstance(value, list):
        for index, child in enumerate(value):
            found = find_forbidden(child, f"{path}[{index}]")
            if found:
                return found
    return None


def is_number(value: Any) -> bool:
    return isinstance(value, (int, float)) and not isinstance(value, bool) and math.isfinite(value)


def check_type(name: str, value: Any, declared: str) -> str | None:
    if declared == "string":
        return None if isinstance(value, str) and value else f"{name} must be a non-empty string"
    if declared == "string_or_null":
        return None if value is None or (isinstance(value, str) and value) else f"{name} must be a non-empty string or null"
    if declared == "integer_or_null":
        return None if value is None or (isinstance(value, int) and not isinstance(value, bool)) else f"{name} must be an integer or null"
    if declared == "number_or_null":
        return None if value is None or is_number(value) else f"{name} must be a finite number or null"
    if declared == "boolean":
        return None if isinstance(value, bool) else f"{name} must be boolean"
    if declared == "boolean_or_null":
        return None if value is None or isinstance(value, bool) else f"{name} must be boolean or null"
    if declared == "date_or_null":
        return None if value is None or (isinstance(value, str) and re.fullmatch(r"\d{4}-\d{2}-\d{2}", value)) else f"{name} must be YYYY-MM-DD or null"
    if declared == "sha256_string":
        return None if isinstance(value, str) and SHA256_RE.fullmatch(value) else f"{name} must be lowercase SHA-256"
    if declared == "sha256_string_or_null":
        return None if value is None or (isinstance(value, str) and SHA256_RE.fullmatch(value)) else f"{name} must be SHA-256 or null"
    if declared == "sha1_or_sha256_string":
        return None if isinstance(value, str) and COMMIT_RE.fullmatch(value) else f"{name} must be SHA-1 or SHA-256"
    if declared == "pass_fail_blocked":
        return None if value in {"pass", "fail", "blocked_by_missing_evidence"} else f"{name} has an invalid result"
    if declared == "ALL INVARIANTS HOLD":
        return None if value == declared else f"{name} must equal {declared!r}"
    raise ValueError(f"unsupported schema type: {declared}")


def validate_record(record: Any, schema: dict[str, Any]) -> list[str]:
    errors: list[str] = []
    if not isinstance(record, dict):
        return ["record must be a JSON object"]
    forbidden = find_forbidden(record)
    if forbidden:
        errors.append(forbidden)
    required = schema["required_fields"]
    missing = sorted(set(required) - set(record))
    extra = sorted(set(record) - set(required))
    if missing:
        errors.append("missing required fields: " + ", ".join(missing))
    if extra:
        errors.append("unknown fields: " + ", ".join(extra))
    for name, declared in required.items():
        if name in record:
            try:
                error = check_type(name, record[name], declared)
            except ValueError as exc:
                errors.append(str(exc))
                continue
            if error:
                errors.append(error)
    for name, allowed in schema.get("allowed_values", {}).items():
        if name in record and record[name] not in allowed:
            errors.append(f"{name} has an unsupported value")
    if record.get("fixture_only") is True and record.get("gate_id") == "haptic-wrist-human":
        errors.append("haptic-wrist-human cannot claim fixture_only")
    if record.get("result") == "pass":
        if record.get("i2c_error_count") != 0:
            errors.append("pass requires i2c_error_count equal to zero")
        if record.get("raw_log_digest") is None:
            errors.append("pass requires raw_log_digest")
        if record.get("failure_reason_code") is not None:
            errors.append("pass cannot carry failure_reason_code")
        if record.get("safety_stop") is not True:
            errors.append("pass requires an observed safety_stop")
    elif record.get("result") in {"fail", "blocked_by_missing_evidence"}:
        if not record.get("failure_reason_code"):
            errors.append("non-pass result requires failure_reason_code")
        if record.get("raw_log_digest") is None and record.get("result") == "fail":
            errors.append("fail requires raw_log_digest")
    supplied = record.get("record_digest")
    if isinstance(supplied, str) and SHA256_RE.fullmatch(supplied):
        expected = digest_without_record_digest(record)
        if supplied != expected:
            errors.append("record_digest does not match canonical record")
    return errors


def load_records(path: pathlib.Path) -> list[tuple[str, Any]]:
    value = json.loads(path.read_text(encoding="utf-8"))
    if isinstance(value, list):
        return [(f"{path}[{i}]", item) for i, item in enumerate(value)]
    return [(str(path), value)]


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("records", nargs="+", type=pathlib.Path)
    parser.add_argument("--schema", type=pathlib.Path, default=DEFAULT_SCHEMA)
    args = parser.parse_args(argv)
    schema = json.loads(args.schema.read_text(encoding="utf-8"))
    failures = 0
    checked = 0
    for path in args.records:
        for label, record in load_records(path):
            checked += 1
            errors = validate_record(record, schema)
            if errors:
                failures += 1
                print(f"FAIL {label}: {len(errors)} validation errors")
                for error in errors:
                    print(f"  - {error}")
            else:
                print(f"PASS {label}: private numeric HAP-SEM record accepted")
    if failures:
        print(f"HAPTIC BENCH EVIDENCE INVALID — {failures}/{checked} records rejected")
        return 1
    print(f"HAPTIC BENCH EVIDENCE VALID — {checked} record(s); no product-content fields accepted")
    return 0


if __name__ == "__main__":
    sys.exit(main())
