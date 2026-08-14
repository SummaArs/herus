#!/usr/bin/env python3
"""Adversarial tests for the HERUS unsigned local provenance auditor."""
import copy
import json
import os
import sys
import tempfile

sys.path.insert(0, os.path.dirname(__file__))
import provenance_audit as audit

FAILED = False


def ok(condition, what):
    global FAILED
    print(f"  {'PASS' if condition else 'FAIL':<4} {what}")
    if not condition:
        FAILED = True


def has(errors, text):
    return any(text in error for error in errors)


def manifest_for(root):
    with open(os.path.join(root, "input.txt"), "w", encoding="utf-8") as handle:
        handle.write("stable input\n")
    os.mkdir(os.path.join(root, "source"))
    with open(os.path.join(root, "source", "module.c"), "w", encoding="utf-8") as handle:
        handle.write("int main(void) { return 0; }\n")
    os.mkdir(os.path.join(root, "source", "build"))
    with open(os.path.join(root, "source", "build", "ignored.o"), "w", encoding="utf-8") as handle:
        handle.write("ephemeral\n")
    return {
        "schema": audit.SCHEMA,
        "trust_state": audit.TRUST_STATE,
        "release": {
            "source_repository": "https://example.invalid/herus",
            "source_state": "unattested_worktree_inputs",
            "baseline_command": "./prove.sh --quiet",
            "artifact_scope": "source_and_host_proof_inputs_only",
        },
        "protected_inputs": [
            {
                "path": "input.txt",
                "kind": "file",
                "sha256": audit.sha256_file(os.path.join(root, "input.txt")),
                "role": "fixture input",
            },
            {
                "path": "source",
                "kind": "tree",
                "sha256": audit.sha256_tree(os.path.join(root, "source")),
                "role": "fixture tree",
            },
        ],
        "components": [
            {
                "id": "python3-stdlib",
                "kind": "runtime",
                "source": "fixture environment",
                "version": "Python 3",
                "integrity": "environment_unattested",
                "direct": True,
            }
        ],
        "assurance": {
            "external_attestation": "pending",
            "sbom_coverage": "declared_direct_inputs_only",
            "reproducible_build": "not_claimed",
            "build_isolation": "not_claimed",
        },
        "gates": [
            {
                "id": "local-input-digest",
                "status": "active",
                "question": "Do fixture inputs match the unsigned local digest record?",
                "evidence_required": ["fixture audit"],
                "claim_if_pass": "Only the fixture inputs match at audit time.",
                "evidence_files": ["input.txt"],
            }
        ],
    }


def main():
    with tempfile.TemporaryDirectory() as root:
        manifest = manifest_for(root)
        ok(audit.validate(manifest, root) == [],
           "T13 unsigned manifest accepts only a canonical local input inventory")

        changed_file = copy.deepcopy(manifest)
        with open(os.path.join(root, "input.txt"), "w", encoding="utf-8") as handle:
            handle.write("modified input\n")
        errors = audit.validate(changed_file, root)
        ok(has(errors, "digest mismatch: input.txt"),
           "T13 modified protected file is rejected against unchanged local reference")
        with open(os.path.join(root, "input.txt"), "w", encoding="utf-8") as handle:
            handle.write("stable input\n")

        changed_tree = copy.deepcopy(manifest)
        with open(os.path.join(root, "source", "module.c"), "w", encoding="utf-8") as handle:
            handle.write("int main(void) { return 1; }\n")
        errors = audit.validate(changed_tree, root)
        ok(has(errors, "digest mismatch: source"),
           "T13 modified source tree is rejected while build cache is not treated as source")
        with open(os.path.join(root, "source", "module.c"), "w", encoding="utf-8") as handle:
            handle.write("int main(void) { return 0; }\n")

        duplicate_component = copy.deepcopy(manifest)
        duplicate_component["components"].append(copy.deepcopy(duplicate_component["components"][0]))
        errors = audit.validate(duplicate_component, root)
        ok(has(errors, "duplicate component id"),
           "T13 duplicate component identity cannot be hidden in the inventory")

        secret_field = copy.deepcopy(manifest)
        secret_field["components"][0]["token"] = "not-permitted"
        errors = audit.validate(secret_field, root)
        ok(has(errors, "unsupported fields") and has(errors, "forbidden sensitive field"),
           "T13 token-like metadata is rejected rather than entering provenance")

        exaggerated_trust = copy.deepcopy(manifest)
        exaggerated_trust["trust_state"] = "slsa_build_l2"
        errors = audit.validate(exaggerated_trust, root)
        ok(has(errors, "trust_state must remain local_unattested"),
           "T13 local manifest cannot self-upgrade to signed or SLSA-like trust")

        unsafe_path = copy.deepcopy(manifest)
        unsafe_path["protected_inputs"][0]["path"] = "../outside.txt"
        errors = audit.validate(unsafe_path, root)
        ok(has(errors, "safe repository-relative path"),
           "T13 path escape cannot make an external file part of the trusted input set")

        implicit_root = copy.deepcopy(manifest)
        implicit_root["protected_inputs"][0]["path"] = "."
        errors = audit.validate(implicit_root, root)
        ok(has(errors, "safe repository-relative path"),
           "T13 implicit repository root cannot absorb unlisted local files into provenance")

        missing_evidence = copy.deepcopy(manifest)
        missing_evidence["gates"][0]["evidence_files"] = ["missing.txt"]
        errors = audit.validate(missing_evidence, root)
        ok(has(errors, "evidence file is missing"),
           "T13 active local gate requires versioned evidence that actually exists")

        pinned_revision = copy.deepcopy(manifest)
        pinned_revision["components"][0]["integrity"] = "revision_pinned"
        ok(audit.validate(pinned_revision, root) == [],
           "T13 immutable dependency revision is represented without self-claiming builder trust")

        moving_tag = copy.deepcopy(manifest)
        moving_tag["components"][0]["integrity"] = "tag_unpinned"
        errors = audit.validate(moving_tag, root)
        ok(has(errors, "unsupported trust claim"),
           "T13 mutable dependency tag cannot be mistaken for a pinned revision")

        wrong_kind = copy.deepcopy(manifest)
        wrong_kind["protected_inputs"][0]["kind"] = "blob"
        errors = audit.validate(wrong_kind, root)
        ok(has(errors, "kind must be file or tree"),
           "T13 unsupported input representation fails closed")

    if FAILED:
        print("PROVENANCE AUDIT TESTS FAILED")
        return 1
    print("PROVENANCE AUDIT INVARIANTS HOLD — local references remain unsigned, bounded and fail-closed.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
