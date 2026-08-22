"""Adversarial tests for the HAP-SEM pre-energization checklist."""
from __future__ import annotations

import copy
import json
import pathlib
import sys

ROOT = pathlib.Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "tools"))
from haptic_preflight import blocked_record, digest_without_record_digest, load_schema, validate

SCHEMA = load_schema()


def check(label: str, condition: bool, failures: list[str]) -> None:
    print(f"  {'PASS' if condition else 'FAIL'}  {label}")
    if not condition:
        failures.append(label)


def main() -> int:
    failures: list[str] = []
    blocked = blocked_record()
    check("no-hardware template is valid and blocked", not validate(blocked, SCHEMA), failures)

    tampered = copy.deepcopy(blocked)
    tampered["short_check_passed"] = True
    check("tampering a checklist breaks its digest", any("record_digest" in e for e in validate(tampered, SCHEMA)), failures)

    ready = copy.deepcopy(blocked)
    ready["execution_origin"] = "blocked_no_hardware"
    ready["result"] = "ready_for_probe"
    ready["failure_reason_code"] = None
    ready["record_digest"] = digest_without_record_digest(ready)
    check("blocked origin cannot become ready", any("blocked_no_hardware" in e for e in validate(ready, SCHEMA)), failures)

    forbidden = copy.deepcopy(blocked)
    forbidden["free_text"] = "not allowed"
    check("free-text product data is rejected", any("forbidden field" in e for e in validate(forbidden, SCHEMA)), failures)

    complete = copy.deepcopy(blocked)
    complete["execution_origin"] = "physical_operator"
    complete["result"] = "ready_for_probe"
    complete["failure_reason_code"] = None
    for field in SCHEMA["all_true_for_ready"]:
        complete[field] = True
    complete["record_digest"] = digest_without_record_digest(complete)
    check("a complete physical checklist is structurally accepted", not validate(complete, SCHEMA), failures)

    print(f"HAPTIC PREFLIGHT: {5 - len(failures)} pass, {len(failures)} fail")
    return 1 if failures else 0


if __name__ == "__main__":
    raise SystemExit(main())
