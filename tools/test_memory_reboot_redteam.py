#!/usr/bin/env python3
"""GAN red-team for the post-reboot memory boundary."""
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
    "core/memory_semantic_evidence.c",
    "core/memory_physical_session.c",
    "core/memory_physical_session_recovery.c",
    "core/memory_physical_session_bootstrap.c",
    "core/memory_reboot_boundary.c",
    "core/test_memory_reboot_boundary.c",
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
    target = pathlib.Path(mutation.source).name
    sources = [str(mutated_path) if pathlib.Path(source).name == target else source
               for source in SOURCES]
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
            "reboot-semantic-scrub",
            "firmware/core/memory_reboot_boundary.c",
            "    scrub_index(index);\n    rc = memory_physical_session_bootstrap(gate, session_cfg, snapshot, &bootstrap);\n",
            "    rc = memory_physical_session_bootstrap(gate, session_cfg, snapshot, &bootstrap);\n",
        ),
        Mutation(
            "reboot-argument-failure-scrub",
            "firmware/core/memory_reboot_boundary.c",
            "    if (!gate || !index || !session_cfg || !snapshot || !out) {\n        scrub_failure(gate, index, out);\n        return MEMORY_REBOOT_BOUNDARY_E_ARG;\n    }\n",
            "    if (!gate || !index || !session_cfg || !snapshot || !out) {\n        return MEMORY_REBOOT_BOUNDARY_E_ARG; /* REDTEAM */\n    }\n",
        ),
        Mutation(
            "reboot-null-index-gate",
            "firmware/core/memory_reboot_boundary.c",
            "    if (!gate || !index || !session_cfg || !snapshot || !out) {\n",
            "    if (!gate || !session_cfg || !snapshot || !out) {\n",
        ),
        Mutation(
            "reboot-scrub-result-claim",
            "firmware/core/memory_reboot_boundary.c",
            "    out->semantic_index_scrubbed = 1u;\n",
            "    /* REDTEAM: result no longer proves semantic scrub. */\n",
        ),
    )

    print("\n== HERUS memory reboot boundary red-team campaign ==")
    passed = True
    with tempfile.TemporaryDirectory(prefix="herus-reboot-redteam-") as raw:
        directory = pathlib.Path(raw)
        for mutation in mutations:
            passed = run_mutation(directory, mutation) and passed
    if not passed:
        print("MEMORY REBOOT REDTEAM FAILED — a stale-context mutant survived")
        return 1
    print("MEMORY REBOOT REDTEAM: 4/4 critical reboot-boundary mutants killed")
    return 0


if __name__ == "__main__":
    sys.exit(main())
