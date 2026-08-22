#!/usr/bin/env python3
"""GAN red-team for cross-module authority and source-loss composition."""
from __future__ import annotations

import os
import pathlib
import subprocess
import sys
import tempfile
from dataclasses import dataclass

ROOT = pathlib.Path(__file__).resolve().parents[1]
FIRMWARE = ROOT / "firmware"
CC = os.environ.get("CC", "cc")
FLAGS = ["-O2", "-Wall", "-Wextra", "-std=c11", "-Icore", "-Inet", "-Iport", "-Itest"]
SOURCES = (
    "net/crypto.c", "net/core_link.c", "core/voice.c", "core/intent_gate.c",
    "core/assurance.c", "core/interaction.c", "core/test_cross_failure_matrix.c",
)


@dataclass(frozen=True)
class Mutation:
    name: str
    source: str
    find: str
    replace: str


def run_mutation(directory: pathlib.Path, mutation: Mutation) -> bool:
    original_path = ROOT / mutation.source
    original = original_path.read_text(encoding="utf-8")
    if original.count(mutation.find) != 1:
        print(f"  FAIL REDTEAM {mutation.name}: control text not unique")
        return False
    mutated_path = directory / pathlib.Path(mutation.source).name
    mutated_path.write_text(original.replace(mutation.find, mutation.replace),
                            encoding="utf-8")
    sources = [str(mutated_path) if pathlib.Path(source).name == pathlib.Path(mutation.source).name
               else source for source in SOURCES]
    binary = directory / f"{mutation.name}.bin"
    build = subprocess.run([CC, *FLAGS, *sources, "-o", str(binary)],
                           cwd=FIRMWARE, text=True, capture_output=True,
                           check=False)
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
            "assurance-revocation-precedence",
            "firmware/core/assurance.c",
            "    if (snapshot->trust_revoked != 0u)\n        failures |= ASSURANCE_FAIL_REVOKED;\n",
            "    /* REDTEAM: terminal revocation gate removed. */\n",
        ),
        Mutation(
            "interaction-source-loss-scrub",
            "firmware/core/interaction.c",
            "    pending_clear(it);\n    it->state = INTERACTION_LINK_LOST;\n",
            "    /* REDTEAM: pending draft survives source loss. */\n    it->state = INTERACTION_LINK_LOST;\n",
        ),
        Mutation(
            "interaction-assurance-handoff-barrier",
            "firmware/core/interaction.c",
            "    if (assurance_decide(snapshot, &decision) != ASSURANCE_OK)\n        return INTERACTION_E_UNTRUSTED;\n",
            "    /* REDTEAM: assurance result ignored. */\n",
        ),
    )

    print("\n== HERUS cross-module deterministic red-team campaign ==")
    passed = True
    with tempfile.TemporaryDirectory(prefix="herus-cross-redteam-") as raw:
        directory = pathlib.Path(raw)
        for mutation in mutations:
            passed = run_mutation(directory, mutation) and passed
    if not passed:
        print("CROSS FAILURE REDTEAM FAILED — at least one cross-module mutant survived")
        return 1
    print("CROSS FAILURE REDTEAM: 3/3 critical cross-module mutants killed")
    return 0


if __name__ == "__main__":
    sys.exit(main())
