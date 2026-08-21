from __future__ import annotations

import pathlib
import subprocess
import tempfile

ROOT = pathlib.Path(__file__).resolve().parents[1]
SOURCE = ROOT / "firmware" / "core" / "intent_router.c"
TEST = ROOT / "firmware" / "core" / "test_intent_router.c"

MUTATIONS = [
    {
        "name": "conflict-classification-bypass",
        "old": "        return INTENT_ROUTER_CONFLICT_QUERY;\n",
        "new": "        return INTENT_ROUTER_RECALL_MEMORY; /* REDTEAM */\n",
        "why": "conflict must not become ordinary recall",
    },
    {
        "name": "unknown-classification-bypass",
        "old": "    if (contains_any(text, unknown, sizeof(unknown) / sizeof(unknown[0]))) {\n        return INTENT_ROUTER_UNKNOWN;\n    }\n",
        "new": "    if (contains_any(text, unknown, sizeof(unknown) / sizeof(unknown[0]))) {\n        return INTENT_ROUTER_RECALL_MEMORY; /* REDTEAM */\n    }\n",
        "why": "unknown or secret-like input must remain unknown",
    },
    {
        "name": "action-confirmation-bypass",
        "old": "        (intent == INTENT_ROUTER_ACTION_REQUEST ||\n         intent == INTENT_ROUTER_FORGET_MEMORY ||\n         intent == INTENT_ROUTER_SHARE_MEMORY) ? 1u : 0u;\n",
        "new": "        (intent == INTENT_ROUTER_FORGET_MEMORY ||\n         intent == INTENT_ROUTER_SHARE_MEMORY) ? 1u : 0u; /* REDTEAM */\n",
        "why": "action request must require a later physical confirmation",
    },
    {
        "name": "capture-action-confusion",
        "old": "    out->requires_confirmation =\n        (intent == INTENT_ROUTER_ACTION_REQUEST ||\n         intent == INTENT_ROUTER_FORGET_MEMORY ||\n         intent == INTENT_ROUTER_SHARE_MEMORY) ? 1u : 0u;\n",
        "new": "    out->requires_confirmation = 1u; /* REDTEAM */\n",
        "why": "capture proposal must not be mislabeled as an action gate",
    },
    {
        "name": "abstention-bypass",
        "old": "        set_intent(out, intent, INTENT_ROUTER_CONF_STRONG, INTENT_ROUTER_CONF_STRONG, 1u);\n",
        "new": "        set_intent(out, intent, INTENT_ROUTER_CONF_STRONG, INTENT_ROUTER_CONF_STRONG, 0u); /* REDTEAM */\n",
        "why": "unknown and conflict must abstain",
    },
    {
        "name": "preference-evidence-bypass",
        "old": "            matches = candidate->purpose == INTENT_ROUTER_MEMORY_PREFERENCE;\n",
        "new": "            matches = 0; /* REDTEAM */\n",
        "why": "preference updates must retain typed evidence",
    },
    {
        "name": "forget-predecessor-bypass",
        "old": "                matches = matches && candidate->superseded;\n",
        "new": "                matches = matches && !candidate->superseded; /* REDTEAM */\n",
        "why": "forget must target the predecessor, not the current generation",
    },
    {
        "name": "conflict-cause-drop",
        "old": "        if (second != NULL) {\n            add_evidence(out, second);\n        }\n",
        "new": "        if (0 && second != NULL) { /* REDTEAM */\n            add_evidence(out, second);\n        }\n",
        "why": "conflict must preserve both causal supports",
    },
]


def main() -> int:
    original = SOURCE.read_text(encoding="utf-8")
    killed = 0
    with tempfile.TemporaryDirectory(prefix="herus-intent-c11-redteam-") as raw:
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
                print(build.stdout[-1600:])
                continue
            run = subprocess.run([str(binary)], cwd=ROOT, text=True,
                                 stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                                 check=False)
            if run.returncode != 0:
                killed += 1
                print(f"PASS {spec['name']}: {spec['why']}")
            else:
                print(f"FAIL {spec['name']}: surviving mutant")
                print(run.stdout[-1600:])
    print(f"INTENT ROUTER C11 REDTEAM: {killed}/{len(MUTATIONS)} critical mutants killed")
    return 0 if killed == len(MUTATIONS) else 1


if __name__ == "__main__":
    raise SystemExit(main())
