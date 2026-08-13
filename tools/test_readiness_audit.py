#!/usr/bin/env python3
"""Executable proof for the Grand Finale hardware-readiness manifest auditor."""
import copy
import json
import os
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, os.path.join(ROOT, "tools"))
import readiness_audit  # noqa: E402

FAILED = False


def ok(condition, text):
    global FAILED
    print(f"  {'PASS' if condition else 'FAIL':<4} {text}")
    if not condition:
        FAILED = True


def manifest():
    path = os.path.join(ROOT, "research", "hardware_readiness_manifest.json")
    with open(path, "r", encoding="utf-8") as handle:
        return json.load(handle)


def main():
    base = manifest()
    print("\n== H10  readiness manifest evidence and privacy gates ==")
    ok(not readiness_audit.validate(base, ROOT),
       "H10 the frozen pre-hardware manifest is valid while every physical gate remains pending")

    bad_pass = copy.deepcopy(base)
    bad_pass["gates"][0]["status"] = "pass"
    errors = readiness_audit.validate(bad_pass, ROOT)
    ok(any("cannot pass without evidence_files" in error for error in errors),
       "H10 a physical gate cannot be marked pass without a repository evidence file")

    private = copy.deepcopy(base)
    private["gates"][1]["audio"] = "forbidden"
    errors = readiness_audit.validate(private, ROOT)
    ok(any("forbidden private field" in error for error in errors),
       "H10 audio, transcript and other prohibited product-log fields are rejected by schema")

    invalid_unit = copy.deepcopy(base)
    invalid_unit["privacy"]["telemetry_allowed"].append("pair_key")
    errors = readiness_audit.validate(invalid_unit, ROOT)
    ok(any("includes a forbidden private field" in error for error in errors),
       "H10 the allow-list cannot silently acquire a secret or identity-bearing field")

    if FAILED:
        print("READINESS AUDIT TESTS FAILED")
        return 1
    print("READINESS AUDIT INVARIANTS HOLD — pending hardware evidence is explicit, private and non-fabricated.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
