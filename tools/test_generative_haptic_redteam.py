from __future__ import annotations

import pathlib
import subprocess
import tempfile

ROOT = pathlib.Path(__file__).resolve().parents[1]
SOURCE = ROOT / "firmware" / "core" / "generative_haptic_bridge.c"
TEST = ROOT / "firmware" / "core" / "test_generative_haptic_bridge.c"
HAPTIC = ROOT / "firmware" / "core" / "haptic_language.c"

MUTATIONS = [
    {
        "name": "actionable-bypass",
        "old": "    out->actionable = 0u;\n",
        "new": "    out->actionable = 1u; /* REDTEAM */\n",
        "why": "haptic presentation cannot become execution authority",
    },
    {
        "name": "policy-as-success",
        "old": "        out->event.state = HL_STATE_DENIED;\n        out->abstained = 1u;\n        return HL_OK;\n",
        "new": "        out->event.state = HL_STATE_CONFIRMED; /* REDTEAM */\n        out->abstained = 0u;\n        return HL_OK;\n",
        "why": "policy block must remain a visible privacy denial",
    },
    {
        "name": "abstention-as-ack",
        "old": "        out->event.state = HL_STATE_UNKNOWN;\n        out->abstained = 1u;\n        return HL_OK;\n",
        "new": "        out->event.state = HL_STATE_CONFIRMED; /* REDTEAM */\n        out->abstained = 0u;\n        return HL_OK;\n",
        "why": "unknown and conflict cannot be encoded as confident success",
    },
    {
        "name": "plan-as-confirmed",
        "old": "        out->event.state = HL_STATE_PENDING;\n        out->confirmation_required = 1u;\n        return HL_OK;\n",
        "new": "        out->event.state = HL_STATE_CONFIRMED; /* REDTEAM */\n        out->confirmation_required = 0u;\n        return HL_OK;\n",
        "why": "a plan must remain pending until physical confirmation",
    },
]


def main() -> int:
    original = SOURCE.read_text(encoding="utf-8")
    killed = 0
    with tempfile.TemporaryDirectory(prefix="herus-generative-haptic-redteam-") as raw:
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
                "-Ifirmware/core", str(HAPTIC), str(mutated), str(TEST),
                "-o", str(binary),
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
    print(f"GENERATIVE HAPTIC REDTEAM: {killed}/{len(MUTATIONS)} critical mutants killed")
    return 0 if killed == len(MUTATIONS) else 1


if __name__ == "__main__":
    raise SystemExit(main())
