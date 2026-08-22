"""GAN red-team for post-reboot semantic reindexing."""
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
    "core/test_memory_post_reboot_reindex.c",
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
            "reindex-floor-gate",
            "firmware/core/memory_semantic_evidence.c",
            "    if (observed_generation == 0u ||\n        observed_generation <= index->generation_floor) {\n        index->rejected++;\n        return MSE_E_ROLLBACK;\n    }\n",
            "    /* REDTEAM: stale post-reboot evidence accepted. */\n",
        ),
        Mutation(
            "reindex-supersession",
            "firmware/core/memory_semantic_evidence.c",
            "        if (item->status == MSE_EVIDENCE_ACTIVE && same_fact(&item->fact, fact)) {\n",
            "        if (0) { /* REDTEAM: exact supersession removed. */\n",
        ),
        Mutation(
            "reindex-expiry",
            "firmware/core/memory_semantic_evidence.c",
            "            current_generation > item->valid_until_generation) {\n",
            "            0) { /* REDTEAM: expiry removed. */\n",
        ),
        Mutation(
            "reindex-conflict-marking",
            "firmware/core/memory_semantic_evidence.c",
            "        if (conflicting_fact(&item->fact, fact, index->is_functional,\n                             index->policy_user)) {\n",
            "        if (0) { /* REDTEAM: conflict marking removed. */\n",
        ),
        Mutation(
            "reindex-conflict-abstention",
            "firmware/core/memory_semantic_evidence.c",
            "    if (conflicted != 0u) {\n",
            "    if (0) { /* REDTEAM: contradiction no longer abstains. */\n",
        ),
    )

    print("\n== HERUS post-reboot reindex red-team campaign ==")
    passed = True
    with tempfile.TemporaryDirectory(prefix="herus-reindex-redteam-") as raw:
        directory = pathlib.Path(raw)
        for mutation in mutations:
            passed = run_mutation(directory, mutation) and passed
    if not passed:
        print("POST-REBOOT REINDEX REDTEAM FAILED — a stale or silent-update mutant survived")
        return 1
    print("POST-REBOOT REINDEX REDTEAM: 5/5 critical reindex mutants killed")
    return 0


if __name__ == "__main__":
    sys.exit(main())
