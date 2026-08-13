#!/usr/bin/env python3
"""Validate the HERUS pre-hardware readiness manifest without interpreting it as data.

The manifest records gates and required evidence; it is not an evidence log. A gate
cannot be marked pass without committed evidence file references. This tool never
prints message content or private identifiers because it accepts none in its schema.
"""
import argparse
import json
import os
import sys

SCHEMA = "herus-hardware-readiness-v1"
STATUSES = {"pending", "pass", "rejected"}
GATE_KEYS = {"id", "domain", "status", "question", "evidence_required", "claim_if_pass", "evidence_files"}
FORBIDDEN = {"audio", "transcript", "embedding", "identity", "location", "key", "pair_key", "message_content"}


def fail(errors, message):
    errors.append(message)


def string_list(value, label, errors, nonempty=True):
    if not isinstance(value, list) or (nonempty and not value):
        fail(errors, f"{label} must be a {'nonempty ' if nonempty else ''}list")
        return False
    if not all(isinstance(item, str) and item for item in value):
        fail(errors, f"{label} entries must be nonempty strings")
        return False
    return True


def forbidden_keys(value, path, errors, exempt=False):
    if isinstance(value, dict):
        for key, child in value.items():
            child_path = f"{path}.{key}"
            if key in FORBIDDEN and not exempt:
                fail(errors, f"forbidden private field at {child_path}")
            forbidden_keys(child, child_path, errors, exempt=(path == "privacy" and key == "product_log_forbidden"))
    elif isinstance(value, list):
        for index, child in enumerate(value):
            forbidden_keys(child, f"{path}[{index}]", errors, exempt)


def validate(manifest, base_dir):
    errors = []
    if not isinstance(manifest, dict):
        return ["manifest root must be an object"]
    if manifest.get("schema") != SCHEMA:
        fail(errors, f"schema must equal {SCHEMA}")

    release = manifest.get("release")
    if not isinstance(release, dict):
        fail(errors, "release must be an object")
    else:
        for key in ("label", "baseline_command", "baseline_requirement", "hardware_state"):
            if not isinstance(release.get(key), str) or not release[key]:
                fail(errors, f"release.{key} must be a nonempty string")
        if release.get("baseline_command") != "./prove.sh --quiet":
            fail(errors, "release.baseline_command must preserve the reproducible proof command")
        if release.get("baseline_requirement") != "must_pass_before_any_flash":
            fail(errors, "release.baseline_requirement must require proof before flash")
        if release.get("hardware_state") not in {"pre_hardware", "bench_active", "hardware_evidence_collected"}:
            fail(errors, "release.hardware_state is invalid")

    privacy = manifest.get("privacy")
    if not isinstance(privacy, dict):
        fail(errors, "privacy must be an object")
    else:
        string_list(privacy.get("telemetry_allowed"), "privacy.telemetry_allowed", errors)
        string_list(privacy.get("product_log_forbidden"), "privacy.product_log_forbidden", errors)
        if set(privacy.get("product_log_forbidden", [])) != FORBIDDEN:
            fail(errors, "privacy.product_log_forbidden must exactly preserve the HERUS forbidden fields")
        if set(privacy.get("telemetry_allowed", [])) & FORBIDDEN:
            fail(errors, "privacy.telemetry_allowed includes a forbidden private field")

    gates = manifest.get("gates")
    if not isinstance(gates, list) or not gates:
        fail(errors, "gates must be a nonempty list")
        gates = []
    ids = set()
    for index, gate in enumerate(gates):
        label = f"gates[{index}]"
        if not isinstance(gate, dict):
            fail(errors, f"{label} must be an object")
            continue
        extra = set(gate) - GATE_KEYS
        if extra:
            fail(errors, f"{label} has unsupported fields: {','.join(sorted(extra))}")
        for key in ("id", "domain", "status", "question", "claim_if_pass"):
            if not isinstance(gate.get(key), str) or not gate[key]:
                fail(errors, f"{label}.{key} must be a nonempty string")
        gate_id = gate.get("id")
        if isinstance(gate_id, str):
            if gate_id in ids:
                fail(errors, f"duplicate gate id: {gate_id}")
            ids.add(gate_id)
        if gate.get("status") not in STATUSES:
            fail(errors, f"{label}.status must be pending, pass or rejected")
        string_list(gate.get("evidence_required"), f"{label}.evidence_required", errors)
        evidence = gate.get("evidence_files", [])
        if evidence is not None and not string_list(evidence, f"{label}.evidence_files", errors, nonempty=False):
            evidence = []
        if gate.get("status") == "pass":
            if not evidence:
                fail(errors, f"{label} cannot pass without evidence_files")
            for relpath in evidence or []:
                if os.path.isabs(relpath) or relpath.startswith("../"):
                    fail(errors, f"{label} evidence path must be repository-relative")
                    continue
                if not os.path.isfile(os.path.join(base_dir, relpath)):
                    fail(errors, f"{label} evidence file is missing: {relpath}")
        elif evidence:
            fail(errors, f"{label} is not pass but lists physical evidence")

    forbidden_keys(manifest, "manifest", errors)
    return errors


def main(argv=None):
    parser = argparse.ArgumentParser(description="Validate a HERUS hardware-readiness manifest")
    parser.add_argument("manifest", help="JSON readiness manifest")
    parser.add_argument("--strict", action="store_true", help="return nonzero on validation errors")
    args = parser.parse_args(argv)
    try:
        with open(args.manifest, "r", encoding="utf-8") as handle:
            manifest = json.load(handle)
    except (OSError, json.JSONDecodeError) as exc:
        print(f"READINESS MANIFEST INVALID — cannot load: {exc}")
        return 2
    errors = validate(manifest, os.path.dirname(os.path.abspath(args.manifest)))
    if errors:
        print("READINESS MANIFEST INVALID")
        for error in errors:
            print(f"  FAIL  {error}")
        return 1 if args.strict else 0
    pending = sum(gate["status"] == "pending" for gate in manifest["gates"])
    passed = sum(gate["status"] == "pass" for gate in manifest["gates"])
    rejected = sum(gate["status"] == "rejected" for gate in manifest["gates"])
    print(f"READINESS MANIFEST VALID — {pending} pending, {passed} passed, {rejected} rejected; no physical result is inferred.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
