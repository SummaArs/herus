from __future__ import annotations

import json
import pathlib
import subprocess
import tempfile

ROOT = pathlib.Path(__file__).resolve().parents[1]
CASES = ROOT / "research" / "benchmarks" / "intent_memory" / "cases.jsonl"

INTENT_IDS = {
    "unknown_query": 0,
    "recall_memory": 1,
    "capture_memory": 2,
    "action_request": 3,
    "forget_memory": 4,
    "update_preference": 5,
    "share_memory": 6,
    "conflict_query": 7,
    "chitchat": 8,
}


def main() -> int:
    cases = [json.loads(line) for line in CASES.read_text(encoding="utf-8").splitlines() if line.strip()]
    expected = {case["id"]: case for case in cases if case["split"] == "test"}
    with tempfile.TemporaryDirectory(prefix="herus-intent-cross-") as raw:
        binary = pathlib.Path(raw) / "intent_router_cross_probe"
        build = subprocess.run([
            "cc", "-O2", "-Wall", "-Wextra", "-Werror", "-std=c11",
            "-Ifirmware/core", "firmware/core/intent_router.c",
            "firmware/core/intent_router_cross_probe.c", "-o", str(binary),
        ], cwd=ROOT, text=True, stdout=subprocess.PIPE,
           stderr=subprocess.STDOUT, check=False)
        if build.returncode != 0:
            print("INTENT ROUTER CROSS: compile failed")
            print(build.stdout)
            return 1
        run = subprocess.run([str(binary)], cwd=ROOT, text=True,
                             stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                             check=False)
        if run.returncode != 0:
            print("INTENT ROUTER CROSS: probe failed")
            print(run.stdout)
            return 1
    failures = []
    rows = run.stdout.splitlines()
    for line in rows:
        fields = line.split()
        if len(fields) < 6 or fields[0] != "CASE":
            failures.append((line, "malformed"))
            continue
        case_id = fields[1]
        case = expected.get(case_id)
        if case is None:
            failures.append((line, "unexpected case"))
            continue
        actual_intent = int(fields[2])
        actual_abstain = int(fields[3])
        actual_confirmation = int(fields[4])
        actual_evidence_count = int(fields[5])
        actual_evidence = [int(value) for value in fields[6:]]
        expected_intent = INTENT_IDS[str(case["intent"])]
        evidence_map = {
            "meeting_v1": 1,
            "meeting_v2": 2,
            "pref_concise": 3,
            "project_local": 5,
        }
        expected_evidence = [evidence_map[str(value)] for value in case["expected_evidence"]]
        if (actual_intent != expected_intent or
                actual_abstain != int(bool(case["should_abstain"])) or
                actual_confirmation != int(bool(case["action_requires_confirmation"])) or
                actual_evidence_count != len(actual_evidence) or
                not set(expected_evidence).issubset(set(actual_evidence)) or
                (not expected_evidence and actual_evidence)):
            failures.append((case_id, {"expected_intent": expected_intent, "actual_intent": actual_intent, "expected_abstain": case["should_abstain"], "actual_abstain": actual_abstain, "expected_confirmation": case["action_requires_confirmation"], "actual_confirmation": actual_confirmation, "expected_evidence": expected_evidence, "actual_evidence_count": actual_evidence_count, "actual_evidence": actual_evidence}))
    if len(rows) != len(expected):
        failures.append(("case-count", {"expected": len(expected), "actual": len(rows)}))
    if failures:
        print(f"INTENT ROUTER CROSS: FAIL {len(failures)}")
        for failure in failures:
            print(failure)
        return 1
    print(f"INTENT ROUTER CROSS: PASS {len(rows)}/{len(expected)} held-out cases")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
