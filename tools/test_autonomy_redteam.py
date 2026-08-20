"""Deterministic red-team mutations for HERUS autonomy policy.

Each mutant removes one deny-by-default control from the real C implementation,
compiles the existing autonomy-policy adversarial suite, and requires the suite
to fail. A mutant that still passes is a surviving attack and fails this harness.
"""
from __future__ import annotations

import os
import pathlib
import subprocess
import sys
import tempfile
from dataclasses import dataclass

ROOT = pathlib.Path(__file__).resolve().parents[1]
CORE = ROOT / "firmware" / "core"
FIRMWARE = ROOT / "firmware"
CC = os.environ.get("CC", "cc")
FLAGS = ["-O2", "-Wall", "-Wextra", "-std=c11", "-Icore"]


@dataclass(frozen=True)
class Mutation:
    name: str
    find: str
    replace: str


def run_mutation(directory: pathlib.Path, mutation: Mutation) -> bool:
    original = (CORE / "autonomy_policy.c").read_text(encoding="utf-8")
    if original.count(mutation.find) != 1:
        print(f"  FAIL REDTEAM {mutation.name}: control text not unique")
        return False
    mutated = directory / "autonomy_policy.c"
    mutated.write_text(original.replace(mutation.find, mutation.replace), encoding="utf-8")
    binary = directory / f"{mutation.name}.bin"
    command = [
        CC,
        *FLAGS,
        str(mutated),
        "core/test_autonomy_policy.c",
        "-o",
        str(binary),
    ]
    build = subprocess.run(command, cwd=FIRMWARE, text=True,
                           capture_output=True, check=False)
    if build.returncode != 0:
        print(f"  FAIL REDTEAM {mutation.name}: mutant did not compile")
        print(build.stderr)
        return False
    result = subprocess.run([str(binary)], cwd=FIRMWARE, text=True,
                            capture_output=True, check=False)
    if result.returncode == 0:
        print(f"  FAIL REDTEAM {mutation.name}: sabotage survived")
        return False
    print(f"  PASS REDTEAM {mutation.name}: sabotage was detected")
    return True


def main() -> int:
    mutations = (
        Mutation(
            "sensitive-context-barrier",
            "    if (envelope->sensitive_context || envelope->third_party_context) {\n        if (envelope->proactive || envelope->level >= HERUS_A2_CONTEXTUAL)\n            return HERUS_POLICY_REJECTED;\n    }\n",
            "    /* REDTEAM: primary sensitive barrier removed. */\n",
        ),
        Mutation(
            "proactive-consent-barrier",
            "    if (envelope->level == HERUS_A0_SILENT ||\n        (envelope->proactive &&\n         (envelope->attention_window != 1u ||\n          envelope->proactive_consent != 1u)))\n        return HERUS_POLICY_SILENT;\n",
            "    if (envelope->level == HERUS_A0_SILENT)\n        return HERUS_POLICY_SILENT; /* REDTEAM */\n",
        ),
        Mutation(
            "one-shot-confirmation-barrier",
            "        envelope->confirmation_consumed == 1u)\n        return HERUS_POLICY_REVOKED;\n",
            "        0) /* REDTEAM: reusable confirmation */\n        return HERUS_POLICY_REVOKED;\n",
        ),
        Mutation(
            "scope-barrier",
            "    if (envelope->level == HERUS_A0_SILENT && envelope->scope != HERUS_SCOPE_NONE)\n        return HERUS_POLICY_SCOPE;\n",
            "    /* REDTEAM: silent scope barrier removed. */\n",
        ),
        Mutation(
            "canonical-policy-bit-barrier",
            "        !canonical_bool(envelope->proactive) ||\n",
            "        /* REDTEAM: proactive bit not canonicalized. */\n",
        ),
        Mutation(
            "confirmation-proposal-binding",
            "    if (envelope->proposal_id == 0u || envelope->proposal_id != proposal_id ||\n        envelope->confirmation_id != confirmation_id ||\n",
            "    if (envelope->proposal_id == 0u ||\n        /* REDTEAM: confirmation binding removed. */\n",
        ),
    )
    print("\n== HERUS deterministic autonomy red-team campaign ==")
    passed = True
    with tempfile.TemporaryDirectory(prefix="herus-autonomy-redteam-") as directory:
        path = pathlib.Path(directory)
        for mutation in mutations:
            passed = run_mutation(path, mutation) and passed
    if not passed:
        print("AUTONOMY REDTEAM FAILED — at least one critical mutant survived")
        return 1
    print("AUTONOMY REDTEAM: 6/6 critical autonomy mutants killed")
    return 0


if __name__ == "__main__":
    sys.exit(main())
