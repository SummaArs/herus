from __future__ import annotations

import pathlib
import subprocess
import tempfile

ROOT = pathlib.Path(__file__).resolve().parents[1]
SOURCE = ROOT / "firmware" / "core" / "generative_dialogue_loop.c"
TEST = ROOT / "firmware" / "core" / "test_generative_dialogue_loop.c"
HAPTIC = ROOT / "firmware" / "core" / "haptic_language.c"
BRIDGE = ROOT / "firmware" / "core" / "generative_haptic_bridge.c"

MUTATIONS = [
    {
        "name": "session-bypass",
        "old": "gdl_status_t gdl_confirm(gdl_t *loop, uint32_t physical_session_id,\n                         uint32_t now_ms)\n{\n    (void)now_ms;\n    if (loop == NULL || physical_session_id == 0u) return GDL_E_ARG;\n    if (loop->state != GDL_CONFIRMATION_PENDING)\n        return GDL_E_STATE;\n    if (physical_session_id != loop->physical_session_id)\n        return GDL_E_PHYSICAL;\n",
        "new": "gdl_status_t gdl_confirm(gdl_t *loop, uint32_t physical_session_id,\n                         uint32_t now_ms)\n{\n    (void)now_ms;\n    if (loop == NULL || physical_session_id == 0u) return GDL_E_ARG;\n    if (loop->state != GDL_CONFIRMATION_PENDING)\n        return GDL_E_STATE;\n    if (0) /* REDTEAM */\n        return GDL_E_PHYSICAL;\n",
        "why": "only the matching physical session may confirm",
    },
    {
        "name": "confirmation-replay",
        "old": "    loop->state = GDL_CONFIRMED;\n    loop->physical_session_id = 0u;\n",
        "new": "    loop->state = GDL_CONFIRMATION_PENDING; /* REDTEAM */\n    loop->physical_session_id = 0u;\n",
        "why": "confirmation must be a one-shot terminal transition",
    },
    {
        "name": "timeout-retain-payload",
        "old": "    clear_payload(loop);\n    loop->state = GDL_TIMED_OUT;\n    loop->metrics.timed_out++;\n",
        "new": "    /* REDTEAM: timeout leaves stale candidate */\n    loop->state = GDL_TIMED_OUT;\n    loop->metrics.timed_out++;\n",
        "why": "timeout cannot leave a candidate or session armed",
    },
    {
        "name": "abort-retain-payload",
        "old": "    clear_payload(loop);\n    loop->state = GDL_ABORTED;\n    loop->metrics.interrupted++;\n",
        "new": "    /* REDTEAM: abort leaves stale candidate */\n    loop->state = GDL_ABORTED;\n    loop->metrics.interrupted++;\n",
        "why": "interruption must erase transient generation state",
    },
    {
        "name": "forget-retain-candidate",
        "old": "gdl_status_t gdl_forget(gdl_t *loop)\n{\n    if (loop == NULL) return GDL_E_ARG;\n    clear_payload(loop);\n    loop->state = GDL_CLEARED;\n",
        "new": "gdl_status_t gdl_forget(gdl_t *loop)\n{\n    if (loop == NULL) return GDL_E_ARG;\n    memset(&loop->signal, 0, sizeof(loop->signal)); /* REDTEAM */\n    loop->state = GDL_CLEARED;\n",
        "why": "privacy forget must clear candidate and haptic state together",
    },
]


def main() -> int:
    original = SOURCE.read_text(encoding="utf-8")
    killed = 0
    with tempfile.TemporaryDirectory(prefix="herus-generative-dialogue-loop-redteam-") as raw:
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
                "-Ifirmware/core", str(HAPTIC), str(BRIDGE), str(mutated),
                str(TEST), "-o", str(binary),
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
    print(f"GENERATIVE DIALOGUE LOOP REDTEAM: {killed}/{len(MUTATIONS)} critical mutants killed")
    return 0 if killed == len(MUTATIONS) else 1


if __name__ == "__main__":
    raise SystemExit(main())
