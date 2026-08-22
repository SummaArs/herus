"""Deterministic red-team mutations for HERUS magic and semantic memory.

The harness mutates real production C files in a private temporary directory,
compiles the existing adversarial suites, and requires every removed control to
be detected. It never touches the repository tree or performs product I/O.
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
FLAGS = ["-O2", "-Wall", "-Wextra", "-std=c11", "-Icore", "-fsanitize=address", "-fno-omit-frame-pointer"]


@dataclass(frozen=True)
class Mutation:
    name: str
    source: str
    find: str
    replace: str
    compile_sources: tuple[str, ...]
    test_binary: str


def run_mutation(directory: pathlib.Path, mutation: Mutation) -> bool:
    original = (CORE / mutation.source).read_text(encoding="utf-8")
    if original.count(mutation.find) != 1:
        print(f"  FAIL REDTEAM {mutation.name}: control text not unique")
        return False
    mutated = directory / mutation.source
    mutated.write_text(original.replace(mutation.find, mutation.replace), encoding="utf-8")
    binary = directory / mutation.test_binary
    sources = []
    for source in mutation.compile_sources:
        sources.append(str(mutated if source == mutation.source else CORE / source))
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
        print(result.stdout)
        print(result.stderr)
        return False
    print(f"  PASS REDTEAM {mutation.name}: sabotage was detected")
    return True


def main() -> int:
    mutations = (
        Mutation(
            "magic-sensitive-context-barrier",
            "magic_anticipation.c",
            "    if (context->privacy_class == MAGIC_PRIVACY_SENSITIVE ||\n        context->privacy_class == MAGIC_PRIVACY_THIRD_PARTY) {\n        out->status = MAGIC_SENSITIVE_BLOCK;\n        return out->status;\n    }\n",
            "    /* REDTEAM: sensitive and third-party block removed. */\n",
            ("magic_anticipation.c", "test_magic_anticipation.c",
             "memory_reasoning_bridge.c", "memory_semantic_evidence.c",
             "symbolic_reasoner.c"),
            "magic-sensitive-context-barrier.bin",
        ),
        Mutation(
            "magic-proactive-consent-barrier",
            "magic_anticipation.c",
            "    if (context->request_kind == MAGIC_REQUEST_CONTEXTUAL &&\n        (context->attention_window != 1u || context->proactive_consent != 1u)) {\n        out->status = MAGIC_SILENT;\n        return out->status;\n    }\n",
            "    if (0) { /* REDTEAM: contextual consent removed. */\n        out->status = MAGIC_SILENT;\n        return out->status;\n    }\n",
            ("magic_anticipation.c", "test_magic_anticipation.c",
             "memory_reasoning_bridge.c", "memory_semantic_evidence.c",
             "symbolic_reasoner.c"),
            "magic-proactive-consent-barrier.bin",
        ),
        Mutation(
            "magic-trigger-budget-barrier",
            "magic_trigger.c",
            "        trigger->proposals_served >= trigger->max_proposals)\n        return MAGIC_TRIGGER_SILENT;\n",
            "        0) /* REDTEAM: proposal budget removed. */\n        return MAGIC_TRIGGER_SILENT;\n",
            ("magic_trigger.c", "test_magic_trigger.c",
             "magic_anticipation.c", "memory_reasoning_bridge.c",
             "memory_semantic_evidence.c", "symbolic_reasoner.c"),
            "magic-trigger-budget-barrier.bin",
        ),
        Mutation(
            "memory-query-structural-validation-barrier",
            "memory_semantic_evidence.c",
            "    if (!index || !valid_pattern(pattern) || current_generation == 0u || !out)\n        return MSE_E_ARG;\n    if (mse_validate(index) != MSE_OK) return MSE_E_FORMAT;\n",
            "    if (!index || !valid_pattern(pattern) || current_generation == 0u || !out)\n        return MSE_E_ARG;\n    /* REDTEAM: query validation removed. */\n",
            ("memory_semantic_evidence.c", "test_memory_semantic_evidence.c"),
            "memory-structural-count-barrier.bin",
        ),
        Mutation(
            "memory-conflict-abstention-barrier",
            "memory_semantic_evidence.c",
            "    if (conflicted != 0u) {\n        out->status = MSE_QUERY_CONTRADICTED;\n        return MSE_OK;\n    }\n",
            "    if (0) { /* REDTEAM: conflict no longer dominates. */\n        out->status = MSE_QUERY_CONTRADICTED;\n        return MSE_OK;\n    }\n",
            ("memory_semantic_evidence.c", "test_memory_semantic_evidence.c"),
            "memory-conflict-abstention-barrier.bin",
        ),
    )
    print("\n== HERUS deterministic magic/memory redteam campaign ==")
    passed = True
    with tempfile.TemporaryDirectory(prefix="herus-magic-redteam-") as directory:
        path = pathlib.Path(directory)
        for mutation in mutations:
            passed = run_mutation(path, mutation) and passed
    if not passed:
        print("MAGIC REDTEAM FAILED — at least one critical magic/memory mutant survived")
        return 1
    print("MAGIC REDTEAM: 5/5 critical magic/memory mutants killed")
    return 0


if __name__ == "__main__":
    sys.exit(main())
