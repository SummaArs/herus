"""Validate and prepare the HAP-SEM pre-energization checklist."""
from __future__ import annotations

import argparse
import hashlib
import json
import os
import pathlib
import re
import sys
from typing import Any

ROOT = pathlib.Path(__file__).resolve().parents[1]
SCHEMA_PATH = ROOT / "research" / "haptic_preflight_schema.json"
SHA256_RE = re.compile(r"^[0-9a-f]{64}$")
FORBIDDEN = {"audio", "transcript", "identity", "location", "key", "free_text", "participant_id"}


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


def load_schema() -> dict[str, Any]:
    return json.loads(SCHEMA_PATH.read_text(encoding="utf-8"))


def validate(record: Any, schema: dict[str, Any]) -> list[str]:
    if not isinstance(record, dict):
        return ["record must be an object"]
    errors: list[str] = []
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
        if name not in record:
            continue
        value = record[name]
        if declared == "boolean" and not isinstance(value, bool):
            errors.append(f"{name} must be boolean")
        elif declared == "string" and (not isinstance(value, str) or not value):
            errors.append(f"{name} must be a non-empty string")
        elif declared == "string_or_null" and value is not None and (not isinstance(value, str) or not value):
            errors.append(f"{name} must be a non-empty string or null")
        elif declared == "sha256_string" and (not isinstance(value, str) or not SHA256_RE.fullmatch(value)):
            errors.append(f"{name} must be lowercase SHA-256")
        elif value != declared and declared.startswith("herus-"):
            errors.append(f"{name} must equal {declared!r}")
    for name, allowed in schema.get("allowed_values", {}).items():
        if name in record and record[name] not in allowed:
            errors.append(f"{name} has an unsupported value")
    if record.get("execution_origin") == "blocked_no_hardware":
        if record.get("result") != "blocked_by_missing_evidence":
            errors.append("blocked_no_hardware requires blocked_by_missing_evidence")
    if record.get("result") == "ready_for_probe":
        if record.get("execution_origin") != "physical_operator":
            errors.append("ready_for_probe requires physical_operator")
        for name in schema["all_true_for_ready"]:
            if record.get(name) is not True:
                errors.append(f"ready_for_probe requires {name}=true")
        if record.get("failure_reason_code") is not None:
            errors.append("ready_for_probe cannot carry failure_reason_code")
    if record.get("result") == "blocked_by_missing_evidence" and not record.get("failure_reason_code"):
        errors.append("blocked result requires failure_reason_code")
    supplied = record.get("record_digest")
    if isinstance(supplied, str) and SHA256_RE.fullmatch(supplied):
        if supplied != digest_without_record_digest(record):
            errors.append("record_digest does not match canonical record")
    return errors


def blocked_record() -> dict[str, Any]:
    record: dict[str, Any] = {
        "schema": "herus-haptic-preflight-v1",
        "execution_origin": "blocked_no_hardware",
        "board_identity_confirmed": False,
        "radio_variant_confirmed": False,
        "schematic_confirmed": False,
        "pin_map_confirmed": False,
        "continuity_power_off": False,
        "short_check_passed": False,
        "pullups_verified": False,
        "enable_low_measured": False,
        "source_current_limit_set": False,
        "single_power_source": False,
        "safety_stop_tested": False,
        "radio_disabled": False,
        "fixture_only": True,
        "result": "blocked_by_missing_evidence",
        "failure_reason_code": "HARDWARE_NOT_ATTACHED",
    }
    record["record_digest"] = digest_without_record_digest(record)
    return record


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    group = parser.add_mutually_exclusive_group(required=True)
    group.add_argument("--prepare-blocked", type=pathlib.Path)
    group.add_argument("--validate", type=pathlib.Path)
    args = parser.parse_args(argv)
    schema = load_schema()
    if args.prepare_blocked is not None:
        record = blocked_record()
        errors = validate(record, schema)
        if errors:
            for error in errors:
                print(f"FAIL generated preflight: {error}")
            return 1
        args.prepare_blocked.write_text(json.dumps(record, sort_keys=True, indent=2) + "\n", encoding="utf-8")
        print("HAPTIC PREFLIGHT: blocked_by_missing_evidence written; no electrical operation executed")
        return 0
    record = json.loads(args.validate.read_text(encoding="utf-8"))
    errors = validate(record, schema)
    if errors:
        print(f"HAPTIC PREFLIGHT: INVALID {args.validate}")
        for error in errors:
            print(f"  - {error}")
        return 1
    print(f"HAPTIC PREFLIGHT: VALID {args.validate}; result={record['result']}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
