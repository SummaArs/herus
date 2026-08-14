#!/usr/bin/env python3
"""Validate HERUS local software provenance without pretending to attest a release.

The manifest is deliberately a local, unsigned inventory and input-digest record.
It catches accidental or unreviewed change to listed repository inputs, but it
cannot authenticate itself, the builder, a Git ref, a host toolchain, an action,
or an artifact. A signed provenance system remains a separate future gate.
"""
import argparse
import hashlib
import json
import os
import re
import sys

SCHEMA = "herus-software-provenance-v1"
TRUST_STATE = "local_unattested"
RELEASE_KEYS = {"source_repository", "source_state", "baseline_command", "artifact_scope"}
INPUT_KEYS = {"path", "kind", "sha256", "role"}
COMPONENT_KEYS = {"id", "kind", "source", "version", "integrity", "direct"}
ASSURANCE_KEYS = {
    "external_attestation", "sbom_coverage", "reproducible_build", "build_isolation"
}
GATE_KEYS = {"id", "status", "question", "evidence_required", "claim_if_pass", "evidence_files"}
ROOT_KEYS = {"schema", "trust_state", "release", "protected_inputs", "components", "assurance", "gates"}
STATUSES = {"active", "pending", "rejected"}
HEX64 = re.compile(r"^[0-9a-f]{64}$")
FORBIDDEN = {
    "audio", "transcript", "embedding", "identity", "location", "key", "pair_key",
    "message_content", "token", "secret", "password", "private_key", "credential"
}


def fail(errors, message):
    errors.append(message)


def canonical_bool(value):
    return isinstance(value, bool)


def safe_relative_path(value):
    if not isinstance(value, str) or not value or os.path.isabs(value):
        return False
    normalized = os.path.normpath(value)
    return (normalized == value and not normalized.startswith("../") and
            normalized not in {".", ".."})


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
                fail(errors, f"forbidden sensitive field at {child_path}")
            forbidden_keys(child, child_path, errors, exempt=False)
    elif isinstance(value, list):
        for index, child in enumerate(value):
            forbidden_keys(child, f"{path}[{index}]", errors, exempt)


def sha256_file(path):
    digest = hashlib.sha256()
    with open(path, "rb") as handle:
        for block in iter(lambda: handle.read(65536), b""):
            digest.update(block)
    return digest.hexdigest()


def sha256_tree(path):
    """Hash regular source files with their normalized relative names and bytes.

    Build products and interpreter caches are deliberately excluded because they
    are not source inputs and vary between runs. Symlinks, special files and any
    directory outside the declared tree are not followed or represented.
    """
    digest = hashlib.sha256()
    for root, directories, files in os.walk(path):
        directories[:] = sorted(
            item for item in directories if item not in {"build", "__pycache__", ".git"}
        )
        for name in sorted(files):
            full_path = os.path.join(root, name)
            if os.path.islink(full_path) or not os.path.isfile(full_path):
                continue
            relative = os.path.relpath(full_path, path).replace(os.sep, "/")
            digest.update(relative.encode("utf-8"))
            digest.update(b"\\0")
            with open(full_path, "rb") as handle:
                for block in iter(lambda: handle.read(65536), b""):
                    digest.update(block)
            digest.update(b"\\0")
    return digest.hexdigest()


def sha256_input(path, kind):
    if os.path.islink(path):
        return None
    if kind == "file" and os.path.isfile(path):
        return sha256_file(path)
    if kind == "tree" and os.path.isdir(path):
        return sha256_tree(path)
    return None


def validate_release(release, errors):
    if not isinstance(release, dict):
        fail(errors, "release must be an object")
        return
    extra = set(release) - RELEASE_KEYS
    if extra:
        fail(errors, f"release has unsupported fields: {','.join(sorted(extra))}")
    for key in RELEASE_KEYS:
        if not isinstance(release.get(key), str) or not release[key]:
            fail(errors, f"release.{key} must be a nonempty string")
    if release.get("baseline_command") != "./prove.sh --quiet":
        fail(errors, "release.baseline_command must preserve the global proof command")
    if release.get("source_state") != "unattested_worktree_inputs":
        fail(errors, "release.source_state must not claim immutable or authenticated source")
    if release.get("artifact_scope") != "source_and_host_proof_inputs_only":
        fail(errors, "release.artifact_scope must not claim a target artifact")


def validate_inputs(inputs, base_dir, errors):
    if not isinstance(inputs, list) or not inputs:
        fail(errors, "protected_inputs must be a nonempty list")
        return
    paths = set()
    for index, item in enumerate(inputs):
        label = f"protected_inputs[{index}]"
        if not isinstance(item, dict):
            fail(errors, f"{label} must be an object")
            continue
        extra = set(item) - INPUT_KEYS
        if extra:
            fail(errors, f"{label} has unsupported fields: {','.join(sorted(extra))}")
        path = item.get("path")
        if not safe_relative_path(path):
            fail(errors, f"{label}.path must be a safe repository-relative path")
            continue
        if path in paths:
            fail(errors, f"duplicate protected input: {path}")
        paths.add(path)
        if item.get("kind") not in {"file", "tree"}:
            fail(errors, f"{label}.kind must be file or tree")
            continue
        if not isinstance(item.get("role"), str) or not item["role"]:
            fail(errors, f"{label}.role must be a nonempty string")
        expected = item.get("sha256")
        if not isinstance(expected, str) or not HEX64.fullmatch(expected):
            fail(errors, f"{label}.sha256 must be a lowercase SHA-256 digest")
            continue
        full_path = os.path.join(base_dir, path)
        actual = sha256_input(full_path, item["kind"])
        if actual is None:
            fail(errors, f"{label} protected input is missing or has wrong kind: {path}")
            continue
        if actual != expected:
            fail(errors, f"{label} digest mismatch: {path}")


def validate_components(components, errors):
    if not isinstance(components, list) or not components:
        fail(errors, "components must be a nonempty list")
        return
    ids = set()
    for index, component in enumerate(components):
        label = f"components[{index}]"
        if not isinstance(component, dict):
            fail(errors, f"{label} must be an object")
            continue
        extra = set(component) - COMPONENT_KEYS
        if extra:
            fail(errors, f"{label} has unsupported fields: {','.join(sorted(extra))}")
        for key in ("id", "kind", "source", "version", "integrity"):
            if not isinstance(component.get(key), str) or not component[key]:
                fail(errors, f"{label}.{key} must be a nonempty string")
        component_id = component.get("id")
        if isinstance(component_id, str):
            if component_id in ids:
                fail(errors, f"duplicate component id: {component_id}")
            ids.add(component_id)
        if not canonical_bool(component.get("direct")):
            fail(errors, f"{label}.direct must be boolean")
        if component.get("integrity") not in {
            "local_digest_declared", "environment_unattested", "revision_pinned"
        }:
            fail(errors, f"{label}.integrity has an unsupported trust claim")


def validate_assurance(assurance, errors):
    if not isinstance(assurance, dict):
        fail(errors, "assurance must be an object")
        return
    extra = set(assurance) - ASSURANCE_KEYS
    if extra:
        fail(errors, f"assurance has unsupported fields: {','.join(sorted(extra))}")
    required = {
        "external_attestation": "pending",
        "sbom_coverage": "declared_direct_inputs_only",
        "reproducible_build": "not_claimed",
        "build_isolation": "not_claimed",
    }
    for key, expected in required.items():
        if assurance.get(key) != expected:
            fail(errors, f"assurance.{key} must remain {expected}")


def validate_gates(gates, base_dir, errors):
    if not isinstance(gates, list) or not gates:
        fail(errors, "gates must be a nonempty list")
        return
    ids = set()
    for index, gate in enumerate(gates):
        label = f"gates[{index}]"
        if not isinstance(gate, dict):
            fail(errors, f"{label} must be an object")
            continue
        extra = set(gate) - GATE_KEYS
        if extra:
            fail(errors, f"{label} has unsupported fields: {','.join(sorted(extra))}")
        for key in ("id", "status", "question", "claim_if_pass"):
            if not isinstance(gate.get(key), str) or not gate[key]:
                fail(errors, f"{label}.{key} must be a nonempty string")
        gate_id = gate.get("id")
        if isinstance(gate_id, str):
            if gate_id in ids:
                fail(errors, f"duplicate gate id: {gate_id}")
            ids.add(gate_id)
        if gate.get("status") not in STATUSES:
            fail(errors, f"{label}.status must be active, pending or rejected")
        string_list(gate.get("evidence_required"), f"{label}.evidence_required", errors)
        evidence = gate.get("evidence_files", [])
        if evidence is not None and not string_list(evidence, f"{label}.evidence_files", errors, nonempty=False):
            evidence = []
        if gate.get("status") == "active":
            for relpath in evidence or []:
                if not safe_relative_path(relpath):
                    fail(errors, f"{label} evidence path must be safe and repository-relative")
                elif not os.path.isfile(os.path.join(base_dir, relpath)):
                    fail(errors, f"{label} evidence file is missing: {relpath}")
        elif evidence:
            fail(errors, f"{label} is not active but lists evidence_files")


def validate(manifest, base_dir):
    errors = []
    if not isinstance(manifest, dict):
        return ["manifest root must be an object"]
    extra = set(manifest) - ROOT_KEYS
    if extra:
        fail(errors, f"manifest has unsupported fields: {','.join(sorted(extra))}")
    if manifest.get("schema") != SCHEMA:
        fail(errors, f"schema must equal {SCHEMA}")
    if manifest.get("trust_state") != TRUST_STATE:
        fail(errors, "trust_state must remain local_unattested until external evidence exists")
    validate_release(manifest.get("release"), errors)
    validate_inputs(manifest.get("protected_inputs"), base_dir, errors)
    validate_components(manifest.get("components"), errors)
    validate_assurance(manifest.get("assurance"), errors)
    validate_gates(manifest.get("gates"), base_dir, errors)
    forbidden_keys(manifest, "manifest", errors)
    return errors


def main(argv=None):
    parser = argparse.ArgumentParser(description="Validate HERUS local software provenance manifest")
    parser.add_argument("manifest", nargs="?", help="JSON provenance manifest")
    parser.add_argument("--strict", action="store_true", help="return nonzero on validation errors")
    parser.add_argument("--print-digest", choices=("file", "tree"), help="print canonical SHA-256 for one input")
    parser.add_argument("--input-path", help="path used with --print-digest")
    args = parser.parse_args(argv)
    if args.print_digest:
        if not args.input_path:
            parser.error("--input-path is required with --print-digest")
        digest = sha256_input(args.input_path, args.print_digest)
        if digest is None:
            print("PROVENANCE INPUT INVALID — path is missing or has wrong kind")
            return 2
        print(digest)
        return 0
    if not args.manifest:
        parser.error("manifest is required unless --print-digest is used")
    try:
        with open(args.manifest, "r", encoding="utf-8") as handle:
            manifest = json.load(handle)
    except (OSError, json.JSONDecodeError) as exc:
        print(f"PROVENANCE MANIFEST INVALID — cannot load: {exc}")
        return 2
    # This repository stores the manifest in research/ while every protected
    # path is intentionally relative to the repository root, not to research/.
    # The function itself still receives an explicit base directory for fixtures.
    repository_root = os.path.abspath(os.path.join(os.path.dirname(args.manifest), ".."))
    errors = validate(manifest, repository_root)
    if errors:
        print("PROVENANCE MANIFEST INVALID")
        for error in errors:
            print(f"  FAIL  {error}")
        return 1 if args.strict else 0
    active = sum(gate["status"] == "active" for gate in manifest["gates"])
    pending = sum(gate["status"] == "pending" for gate in manifest["gates"])
    print(f"PROVENANCE MANIFEST VALID — {active} active, {pending} pending; unsigned local inputs only.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
