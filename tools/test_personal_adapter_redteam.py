from __future__ import annotations

import pathlib
import subprocess
import tempfile

ROOT = pathlib.Path(__file__).resolve().parents[1]
SOURCE = ROOT / "firmware" / "core" / "personal_adapter.c"
TEST = ROOT / "firmware" / "core" / "test_personal_adapter.c"

MUTATIONS = [
    {
        "name": "consent-bypass",
        "old": "    if (explicit_consent != 1u || sample->local_origin != 1u) {\n",
        "new": "    if (sample->local_origin != 1u && explicit_consent == 255u) { /* REDTEAM */\n",
        "why": "learning requires explicit human consent",
    },
    {
        "name": "origin-bypass",
        "old": "    if (explicit_consent != 1u || sample->local_origin != 1u) {\n",
        "new": "    if (explicit_consent != 1u) { /* REDTEAM */\n",
        "why": "external-origin samples cannot self-promote into personal state",
    },
    {
        "name": "revocation-bypass",
        "old": "    if (entry->tombstone != 0u) {\n",
        "new": "    if (0 && entry->tombstone != 0u) { /* REDTEAM */\n",
        "why": "forgotten preferences cannot be silently reintroduced",
    },
    {
        "name": "tie-confidence-bypass",
        "old": "    if (best_votes < PA_MIN_VOTES || out->margin_votes < PA_MIN_MARGIN) {\n",
        "new": "    if (best_votes < PA_MIN_VOTES) { /* REDTEAM */\n",
        "why": "a tied preference must abstain instead of choosing silently",
    },
    {
        "name": "reboot-quarantine-bypass",
        "old": "        profile->entries[i].active = 0u;\n",
        "new": "        profile->entries[i].active = 1u; /* REDTEAM */\n",
        "why": "reboot must quarantine active adaptation",
    },
]


def main() -> int:
    original = SOURCE.read_text(encoding="utf-8")
    killed = 0
    with tempfile.TemporaryDirectory(prefix="herus-personal-adapter-redteam-") as raw:
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
                "-Ifirmware/core", str(mutated), str(TEST), "-o", str(binary),
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
    print(f"PERSONAL ADAPTER REDTEAM: {killed}/{len(MUTATIONS)} critical mutants killed")
    return 0 if killed == len(MUTATIONS) else 1


if __name__ == "__main__":
    raise SystemExit(main())
