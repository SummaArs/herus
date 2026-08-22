from __future__ import annotations

import pathlib
import subprocess
import tempfile

ROOT = pathlib.Path(__file__).resolve().parents[1]
SOURCE = ROOT / "tools" / "intent_memory_benchmark.py"

MUTATIONS = [
    {
        "name": "conflict-override-bypass",
        "old": '        return "conflict_query"\n    if any(word in text for word in ("salvar este ponto"',
        "new": '        return "recall_memory"\n    if any(word in text for word in ("salvar este ponto"',
        "why": "conflict must not be silently routed as ordinary recall",
    },
    {
        "name": "unknown-override-bypass",
        "old": '        return "unknown_query"\n    if any(word in text for word in ("esqueça", "apague", "revogue", "retire da memória")):',
        "new": '        return "recall_memory"\n    if any(word in text for word in ("esqueça", "apague", "revogue", "retire da memória")):',
        "why": "unknown or secret-like questions must abstain",
    },
    {
        "name": "action-confirmation-bypass",
        "old": '    requires_confirmation = intent in {"action_request", "forget_memory", "share_memory"}\n',
        "new": '    requires_confirmation = intent in {"forget_memory", "share_memory"}\n',
        "why": "action requests must never bypass physical confirmation",
    },
    {
        "name": "capture-confirmation-confusion",
        "old": '    if typed and intent == "capture_memory":\n        requires_confirmation = False\n',
        "new": '    if typed and intent == "capture_memory":\n        requires_confirmation = True\n',
        "why": "capture proposal must not be mislabeled as an action confirmation",
    },
    {
        "name": "abstention-gate-bypass",
        "old": '    abstain = intent in {"unknown_query", "conflict_query"} or (\n',
        "new": '    abstain = False or (\n',
        "why": "unknown and conflict cases must remain explicit abstentions",
    },
    {
        "name": "action-evidence-leak",
        "old": '    query = word_set(utterance)\n    if intent in {"action_request", "chitchat", "capture_memory", "unknown_query", "conflict_query"}:\n',
        "new": '    query = word_set(utterance)\n    if intent == "action_request":\n        return [{"memory_id": "meeting_v2", "score": 1}]\n    if intent in {"chitchat", "capture_memory", "unknown_query", "conflict_query"}:\n',
        "why": "an action request must not receive fabricated memory evidence",
    },
    {
        "name": "preference-evidence-bypass",
        "old": '            if any(word in utterance.lower() for word in ("curt", "diret", "sem rodeios")) and memory.memory_id == "pref_concise":\n',
        "new": '            if False and any(word in utterance.lower() for word in ("curt", "diret", "sem rodeios")) and memory.memory_id == "pref_concise":\n',
        "why": "preference updates must preserve their typed evidence",
    },
    {
        "name": "revocation-target-bypass",
        "old": '            overlap += 3 if memory.superseded else 0\n',
        "new": '            overlap += 3 if not memory.superseded else 0\n',
        "why": "forgetting must target the revoked predecessor, not the current memory",
    },
]


def main() -> int:
    original = SOURCE.read_text(encoding="utf-8")
    killed = 0
    with tempfile.TemporaryDirectory(dir=ROOT / "tools", prefix="intent-redteam-") as raw:
        temp_dir = pathlib.Path(raw)
        for spec in MUTATIONS:
            if original.count(spec["old"]) != 1:
                print(f"FAIL {spec['name']}: control text is not unique")
                continue
            mutated = ROOT / "tools" / f".intent_mutant_{spec['name']}.py"
            mutated.write_text(original.replace(spec["old"], spec["new"], 1), encoding="utf-8")
            result = subprocess.run(
                ["python3", str(mutated)], cwd=ROOT, text=True,
                stdout=subprocess.PIPE, stderr=subprocess.STDOUT, check=False,
            )
            mutated.unlink(missing_ok=True)
            if result.returncode != 0:
                killed += 1
                print(f"PASS {spec['name']}: {spec['why']}")
            else:
                print(f"FAIL {spec['name']}: surviving mutant")
                print(result.stdout[-1600:])
    print(f"INTENT MEMORY REDTEAM: {killed}/{len(MUTATIONS)} critical mutants killed")
    return 0 if killed == len(MUTATIONS) else 1


if __name__ == "__main__":
    raise SystemExit(main())
