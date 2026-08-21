from __future__ import annotations

import pathlib
import subprocess
import tempfile

ROOT = pathlib.Path(__file__).resolve().parents[1]
SOURCE = ROOT / "firmware" / "core" / "generative_core.c"
TEST = ROOT / "firmware" / "core" / "test_generative_core.c"

MUTATIONS = [
    {
        "name": "policy-bypass",
        "old": "    if (request->policy_blocked != 0u) {\n",
        "new": "    if (0 && request->policy_blocked != 0u) { /* REDTEAM */\n",
        "why": "policy blocks must remain outside generation",
    },
    {
        "name": "unknown-abstention-bypass",
        "old": "    if (queried == SR_E_NO_EVIDENCE) {\n",
        "new": "    if (0 && queried == SR_E_NO_EVIDENCE) { /* REDTEAM */\n",
        "why": "missing evidence must not become an unsupported answer kind",
    },
    {
        "name": "conflict-abstention-bypass",
        "old": "    if (queried == SR_E_CONTRADICTION) {\n",
        "new": "    if (0 && queried == SR_E_CONTRADICTION) { /* REDTEAM */\n",
        "why": "contradictory evidence must remain explicitly blocked",
    },
    {
        "name": "unknown-lexeme-invention",
        "old": "    if (entry == NULL || entry->text == NULL || entry->length == 0u ||\n        entry->length > GC_MAX_LEXEM_BYTES) {\n",
        "new": "    if (entry != NULL && entry->text != NULL && entry->length > 0u &&\n        entry->length <= GC_MAX_LEXEM_BYTES) { /* REDTEAM */\n",
        "why": "unregistered symbols must not be verbalized as invented language",
    },
    {
        "name": "plan-confirmation-bypass",
        "old": "    out->requires_confirmation = out->plan.confirmation_count > 0u ? 1u : 0u;\n",
        "new": "    out->requires_confirmation = 0u; /* REDTEAM */\n",
        "why": "consequential plan steps must require physical confirmation",
    },
    {
        "name": "plan-authority-bypass",
        "old": "    out->authority = out->plan.confirmation_count > 0u ?\n                     GC_AUTH_CONFIRMATION_REQUIRED : GC_AUTH_PRESENTATION_ONLY;\n",
        "new": "    out->authority = GC_AUTH_PRESENTATION_ONLY; /* REDTEAM */\n",
        "why": "generation cannot erase the plan's confirmation boundary",
    },
]


def main() -> int:
    original = SOURCE.read_text(encoding="utf-8")
    killed = 0
    with tempfile.TemporaryDirectory(prefix="herus-generative-redteam-") as raw:
        work = pathlib.Path(raw)
        for spec in MUTATIONS:
            if original.count(spec["old"]) != 1:
                print(f"FAIL {spec['name']}: control text is not unique")
                continue
            mutated = work / f"{spec['name']}.c"
            binary = work / f"{spec['name']}.bin"
            mutated.write_text(original.replace(spec["old"], spec["new"], 1), encoding="utf-8")
            build = subprocess.run([
                "cc", "-O2", "-Wall", "-Wextra", "-Werror", "-std=c11",
                "-Ifirmware/core", "firmware/core/symbol_registry.c",
                "firmware/core/symbolic_reasoner.c", "firmware/core/symbolic_planner.c",
                "firmware/core/memory_semantic_evidence.c",
                "firmware/core/memory_reasoning_bridge.c",
                "firmware/core/personal_adapter.c",
                str(mutated), str(TEST), "-o", str(binary),
            ], cwd=ROOT, text=True, stdout=subprocess.PIPE,
               stderr=subprocess.STDOUT, check=False)
            if build.returncode != 0:
                print(f"FAIL {spec['name']}: mutant does not compile")
                print(build.stdout[-1200:])
                continue
            run = subprocess.run([str(binary)], cwd=ROOT, text=True,
                                 stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                                 check=False)
            if run.returncode != 0:
                killed += 1
                print(f"PASS {spec['name']}: {spec['why']}")
            else:
                print(f"FAIL {spec['name']}: surviving mutant")
                print(run.stdout[-1200:])
    print(f"GENERATIVE CORE REDTEAM: {killed}/{len(MUTATIONS)} critical mutants killed")
    return 0 if killed == len(MUTATIONS) else 1


if __name__ == "__main__":
    raise SystemExit(main())
