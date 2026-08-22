from __future__ import annotations

import pathlib
import subprocess
import tempfile

ROOT = pathlib.Path(__file__).resolve().parents[1]
SOURCE = ROOT / "firmware" / "core" / "composed_dialogue.c"
TEST = ROOT / "firmware" / "core" / "test_composed_dialogue.c"
SOURCES = [
    "firmware/core/intent_router.c",
    "firmware/core/symbolic_reasoner.c",
    "firmware/core/symbolic_planner.c",
    "firmware/core/memory_semantic_evidence.c",
    "firmware/core/memory_reasoning_bridge.c",
    "firmware/core/personal_adapter.c",
    "firmware/core/generative_core.c",
    "firmware/core/haptic_language.c",
    "firmware/core/generative_haptic_bridge.c",
    "firmware/core/generative_dialogue_loop.c",
]

MUTATIONS = [
    {
        "name": "quarantine-bypass",
        "old": "    if (dialogue->memory_quarantined != 0u) return CDH_E_STATE;\n",
        "new": "    if (0) /* REDTEAM */ return CDH_E_STATE;\n",
        "why": "reboot quarantine must block old configuration",
    },
    {
        "name": "generation-replay",
        "old": "        current_generation <= dialogue->recovered_generation ||\n",
        "new": "        current_generation < dialogue->recovered_generation || /* REDTEAM */\n",
        "why": "equal recovered generation must remain replay",
    },
    {
        "name": "floor-bypass",
        "old": "        memory->generation_floor != dialogue->recovered_generation ||\n",
        "new": "        0 || /* REDTEAM */\n",
        "why": "divergent semantic floors must not rearm",
    },
    {
        "name": "expired-memory-bypass",
        "old": "    dialogue->request.current_generation = dialogue->cfg.current_generation;\n",
        "new": "    dialogue->request.current_generation = 1u; /* REDTEAM */\n",
        "why": "expired pre-reboot evidence must not become grounded",
    },
    {
        "name": "conflict-selection",
        "old": "    if (dialogue->route.intent == INTENT_ROUTER_CONFLICT_QUERY) {\n",
        "new": "    if (0) { /* REDTEAM */\n",
        "why": "conflict route must preserve contradiction rather than choose a side",
    },
    {
        "name": "reboot-retain-payload",
        "old": "int cdh_reboot(cdh_t *dialogue, uint32_t recovered_generation)\n{\n    int result;\n    if (dialogue == NULL || recovered_generation == 0u) return CDH_E_ARG;\n    result = gdl_forget(&dialogue->lifecycle);\n    if (result != GDL_OK) return CDH_E_STATE;\n    clear_transient(dialogue);\n",
        "new": "int cdh_reboot(cdh_t *dialogue, uint32_t recovered_generation)\n{\n    int result;\n    if (dialogue == NULL || recovered_generation == 0u) return CDH_E_ARG;\n    result = gdl_forget(&dialogue->lifecycle);\n    if (result != GDL_OK) return CDH_E_STATE;\n    /* REDTEAM: reboot retains route and candidate */\n",
        "why": "reboot must scrub route, request and generated candidate",
    },
]


def main() -> int:
    original = SOURCE.read_text(encoding="utf-8")
    killed = 0
    with tempfile.TemporaryDirectory(prefix="herus-composed-dialogue-redteam-") as raw:
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
                "-Ifirmware/core", *SOURCES, str(mutated), str(TEST),
                "-o", str(binary),
            ], cwd=ROOT, text=True, stdout=subprocess.PIPE,
               stderr=subprocess.STDOUT, check=False)
            if build.returncode != 0:
                print(f"FAIL {spec['name']}: mutant does not compile")
                print(build.stdout[-1400:])
                continue
            run = subprocess.run([str(binary)], cwd=ROOT, text=True,
                                 stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                                 check=False)
            if run.returncode != 0:
                killed += 1
                print(f"PASS {spec['name']}: {spec['why']}")
            else:
                print(f"FAIL {spec['name']}: surviving mutant")
                print(run.stdout[-1400:])
    print(f"COMPOSED DIALOGUE REDTEAM: {killed}/{len(MUTATIONS)} critical mutants killed")
    return 0 if killed == len(MUTATIONS) else 1


if __name__ == "__main__":
    raise SystemExit(main())
