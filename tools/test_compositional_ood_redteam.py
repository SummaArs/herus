from __future__ import annotations

import pathlib
import subprocess
import tempfile

ROOT = pathlib.Path(__file__).resolve().parents[1]
SOURCE = ROOT / "firmware" / "core" / "symbolic_reasoner.c"
TEST = ROOT / "firmware" / "core" / "test_compositional_ood.c"
SOURCES = [
    "firmware/core/symbol_registry.c",
    "firmware/core/memory_semantic_evidence.c",
    "firmware/core/memory_reasoning_bridge.c",
    "firmware/core/personal_adapter.c",
    "firmware/core/symbolic_planner.c",
    "firmware/core/generative_core.c",
]

MUTATIONS = [
    {
        "name": "contradiction-bypass",
        "old": "    if (hits > 0u && opposite_hits > 0u) {\n",
        "new": "    if (0) { /* REDTEAM */\n",
        "why": "opposite evidence must block a confident answer",
    },
    {
        "name": "absence-bypass",
        "old": "    if (hits == 0u) {\n",
        "new": "    if (0) { /* REDTEAM */\n",
        "why": "missing evidence must abstain",
    },
    {
        "name": "ambiguity-bypass",
        "old": "    if (hits > 1u) {\n",
        "new": "    if (0) { /* REDTEAM */\n",
        "why": "multiple matches must remain ambiguous",
    },
    {
        "name": "budget-bypass",
        "old": "    if (target->derivation_steps >= max_steps) {\n",
        "new": "    if (0) { /* REDTEAM */\n",
        "why": "bounded search must expose a limit",
    },
    {
        "name": "depth-erasure",
        "old": "    out->depth = r->meta[index].depth;\n",
        "new": "    out->depth = 0u; /* REDTEAM */\n",
        "why": "held-out deeper composition must preserve derivation depth",
    },
    {
        "name": "provenance-erasure",
        "old": "    out->evidence_count = r->meta[index].parent_count;\n",
        "new": "    out->evidence_count = 0u; /* REDTEAM */\n",
        "why": "derived generation must retain bounded proof roots",
    },
]


def main() -> int:
    original = SOURCE.read_text(encoding="utf-8")
    killed = 0
    with tempfile.TemporaryDirectory(prefix="herus-compositional-ood-redteam-") as raw:
        work = pathlib.Path(raw)
        for spec in MUTATIONS:
            if original.count(spec["old"]) != 1:
                print(f"FAIL {spec['name']}: control text is not unique")
                continue
            mutated = work / f"{spec['name']}.c"
            binary = work / f"{spec['name']}.bin"
            mutated.write_text(original.replace(spec["old"], spec["new"], 1), encoding="utf-8")
            build = subprocess.run(
                [
                    "cc", "-O2", "-Wall", "-Wextra", "-Werror", "-std=c11",
                    "-Ifirmware/core", *SOURCES, str(mutated), str(TEST),
                    "-o", str(binary),
                ], cwd=ROOT, text=True, stdout=subprocess.PIPE,
                stderr=subprocess.STDOUT, check=False,
            )
            if build.returncode != 0:
                print(f"FAIL {spec['name']}: mutant does not compile")
                print(build.stdout[-1500:])
                continue
            run = subprocess.run([str(binary)], cwd=ROOT, text=True,
                                 stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                                 check=False)
            if run.returncode != 0:
                killed += 1
                print(f"PASS {spec['name']}: {spec['why']}")
            else:
                print(f"FAIL {spec['name']}: surviving mutant")
                print(run.stdout[-1500:])
    print(f"COMPOSITIONAL OOD REDTEAM: {killed}/{len(MUTATIONS)} critical mutants killed")
    return 0 if killed == len(MUTATIONS) else 1


if __name__ == "__main__":
    raise SystemExit(main())
