#!/usr/bin/env python3
"""GAN red-team for semantic-memory degradation and contextual magic."""
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
    "core/memory_semantic_evidence.c", "core/memory_reasoning_bridge.c",
    "core/symbolic_reasoner.c", "core/magic_anticipation.c",
    "core/test_degradation_matrix.c",
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
            "semantic-capacity-gate",
            "firmware/core/memory_semantic_evidence.c",
            "    if (index->evidence_count >= MSE_MAX_EVIDENCE) {\n        index->rejected++;\n        return MSE_E_FULL;\n    }\n",
            "    /* REDTEAM: full-memory rejection removed. */\n",
        ),
        Mutation(
            "magic-consent-gate",
            "firmware/core/magic_anticipation.c",
            "    if (context->request_kind == MAGIC_REQUEST_CONTEXTUAL &&\n        (context->attention_window != 1u || context->proactive_consent != 1u)) {\n        out->status = MAGIC_SILENT;\n        return out->status;\n    }\n",
            "    /* REDTEAM: proactive-consent gate removed. */\n",
        ),
        Mutation(
            "magic-conflict-abstention-gate",
            "firmware/core/magic_anticipation.c",
            "    if (composed == MRB_CONTRADICTED) {\n        out->status = MAGIC_CONTRADICTION;\n        out->explanation_available = 1u;\n        out->requires_confirmation = 1u;\n        return out->status;\n    }\n",
            "    if (composed == MRB_CONTRADICTED) {\n        out->status = MAGIC_RECALL; /* REDTEAM */\n        return out->status;\n    }\n",
        ),
        Mutation(
            "magic-ambiguity-abstention-gate",
            "firmware/core/magic_anticipation.c",
            "    if (composed == MRB_AMBIGUOUS) {\n        out->status = MAGIC_ABSTAIN;\n        out->explanation_available = 1u;\n        return out->status;\n    }\n",
            "    if (composed == MRB_AMBIGUOUS) {\n        out->status = MAGIC_RECALL; /* REDTEAM */\n        return out->status;\n    }\n",
        ),
        Mutation(
            "semantic-conflict-marking-gate",
            "firmware/core/memory_semantic_evidence.c",
            "            item->status = MSE_EVIDENCE_CONFLICTED;\n            index->evidence[inserted].status = MSE_EVIDENCE_CONFLICTED;\n            index->conflicts++;\n",
            "            /* REDTEAM: conflicting facts are left active. */\n",
        ),
    )

    print("\n== HERUS semantic degradation deterministic red-team campaign ==")
    passed = True
    with tempfile.TemporaryDirectory(prefix="herus-degradation-redteam-") as raw:
        directory = pathlib.Path(raw)
        for mutation in mutations:
            passed = run_mutation(directory, mutation) and passed
    if not passed:
        print("DEGRADATION REDTEAM FAILED — at least one capacity/context mutant survived")
        return 1
    print("DEGRADATION REDTEAM: 5/5 critical capacity/context mutants killed")
    return 0


if __name__ == "__main__":
    sys.exit(main())
