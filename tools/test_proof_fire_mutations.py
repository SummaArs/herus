#!/usr/bin/env python3
"""F4 deterministic mutation checks for high-impact HERUS host controls.

Each experiment copies one C source into a private temporary directory, removes a
specific deny-by-default control, compiles the existing adversarial test against
that mutated source, and requires a nonzero test result.  This does not fuzz,
run product I/O, create telemetry, call a network service, or modify repository
files.  It answers only whether selected proofs detect selected control removal.
"""
from __future__ import annotations

import os
import pathlib
import shutil
import subprocess
import sys
import tempfile
from dataclasses import dataclass

ROOT = pathlib.Path(__file__).resolve().parents[1]
FIRMWARE = ROOT / "firmware"
CORE = FIRMWARE / "core"
CC = os.environ.get("CC", "cc")
BASE_FLAGS = ["-O2", "-Wall", "-Wextra", "-std=c11", "-Icore", "-Inet", "-Iport", "-Itest"]


@dataclass(frozen=True)
class Mutation:
    name: str
    source: str
    find: str
    replace: str
    compile_sources: tuple[str, ...]


def mutated_source(tmp: pathlib.Path, mutation: Mutation) -> pathlib.Path:
    original = (CORE / mutation.source).read_text(encoding="utf-8")
    if original.count(mutation.find) != 1:
        raise RuntimeError(f"{mutation.name}: expected one exact control occurrence")
    path = tmp / mutation.source
    path.write_text(original.replace(mutation.find, mutation.replace), encoding="utf-8")
    return path


def run_mutation(tmp: pathlib.Path, mutation: Mutation) -> bool:
    changed = mutated_source(tmp, mutation)
    sources: list[str] = []
    for source in mutation.compile_sources:
        if source == mutation.source:
            sources.append(str(changed))
        else:
            sources.append(str(CORE / source))
    binary = tmp / f"{mutation.name}.bin"
    command = [CC, *BASE_FLAGS, *sources, "-o", str(binary)]
    build = subprocess.run(command, cwd=FIRMWARE, text=True, capture_output=True, check=False)
    if build.returncode != 0:
        print(f"  FAIL F4 {mutation.name}: mutation did not compile\n{build.stderr}")
        return False
    test = subprocess.run([str(binary)], cwd=FIRMWARE, text=True, capture_output=True, check=False)
    if test.returncode == 0:
        print(f"  FAIL F4 {mutation.name}: removed control escaped its adversarial proof")
        return False
    print(f"  PASS F4 {mutation.name}: removed control was detected by its adversarial proof")
    return True


def main() -> int:
    mutations = (
        Mutation(
            name="terminal-session-floor-rejection",
            source="memory_physical_session_recovery.c",
            find="         s->committed_reservation_id == UINT32_MAX ||\n",
            replace="",
            compile_sources=(
                "memory_physical_session.c",
                "memory_physical_session_recovery.c",
                "memory_physical_session_bootstrap.c",
                "test_memory_physical_session_recovery_stress.c",
            ),
        ),
        Mutation(
            name="bootstrap-scrub-evidence",
            source="memory_physical_session_bootstrap.c",
            find="    out->active_evidence_scrubbed = 1u;\n",
            replace="    out->active_evidence_scrubbed = 0u; /* F4 mutation */\n",
            compile_sources=(
                "memory_physical_session.c",
                "memory_physical_session_recovery.c",
                "memory_physical_session_bootstrap.c",
                "test_memory_physical_session_recovery_stress.c",
            ),
        ),
        Mutation(
            name="collection-authentication-requirement",
            source="memory_collection_recovery.c",
            find="        (s->committed_authenticated != 1u || s->committed_generation == 0u))\n",
            replace="        (s->committed_generation == 0u)) /* F4 mutation */\n",
            compile_sources=(
                "memory_collection_recovery.c",
                "test_memory_collection_recovery.c",
            ),
        ),
        Mutation(
            name="model-send-authority-barrier",
            source="threat_model.c",
            find="        require_flag(snapshot->model_no_send_authority, THREAT_MODEL_FAIL_MODEL_SEND, &failures);\n",
            replace="        /* F4 mutation: model send barrier removed. */\n",
            compile_sources=(
                "threat_model.c",
                "test_threat_model.c",
            ),
        ),
        Mutation(
            name="abduction-ground-goal-barrier",
            source="symbolic_reasoner.c",
            find="    if (ground_goal->subject.kind != SR_TERM_CONSTANT ||\n        ground_goal->predicate.kind != SR_TERM_CONSTANT ||\n        ground_goal->object.kind != SR_TERM_CONSTANT) {\n",
            replace="    if (0) { /* F4 mutation: non-ground goals enter abduction. */\n",
            compile_sources=(
                "symbolic_reasoner.c",
                "test_symbolic_reasoner.c",
            ),
        ),
        Mutation(
            name="abduction-ambiguity-barrier",
            source="symbolic_reasoner.c",
            find="        } else if (walk->out->status == SR_ABDUCTION_FOUND &&\n                   !fact_equal(walk->out->missing_fact, candidate)) {\n            walk->out->status = SR_ABDUCTION_AMBIGUOUS;\n            memset(&walk->out->missing_fact, 0, sizeof(walk->out->missing_fact));\n            walk->stop = 1u;\n        }\n",
            replace="        } else if (walk->out->status == SR_ABDUCTION_FOUND &&\n                   !fact_equal(walk->out->missing_fact, candidate)) {\n            /* F4 mutation: competing hypotheses are silently tolerated. */\n        }\n",
            compile_sources=(
                "symbolic_reasoner.c",
                "test_symbolic_reasoner.c",
            ),
        ),
        Mutation(
            name="abduction-candidate-budget-barrier",
            source="symbolic_reasoner.c",
            find="        if (walk->out->candidates_examined >= walk->max_candidates) {\n            walk->out->status = SR_ABDUCTION_LIMIT;\n            walk->stop = 1u;\n            return;\n        }\n",
            replace="        if (0) { /* F4 mutation: candidate budget ignored. */\n            walk->out->status = SR_ABDUCTION_LIMIT;\n            walk->stop = 1u;\n            return;\n        }\n",
            compile_sources=(
                "symbolic_reasoner.c",
                "test_symbolic_reasoner.c",
            ),
        ),
    )

    print("\n== F4 deterministic proof-fire mutation campaign ==")
    passed = True
    with tempfile.TemporaryDirectory(prefix="herus-f4-") as directory:
        tmp = pathlib.Path(directory)
        for mutation in mutations:
            passed = run_mutation(tmp, mutation) and passed
    if not passed:
        print("PROOF-FIRE MUTATION TESTS FAILED")
        return 1
    print("PROOF-FIRE MUTATION INVARIANTS HOLD — selected authority, authentication, quarantine and exhaustion controls are detected when removed.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
