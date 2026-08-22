#!/usr/bin/env python3
"""Mutation tests for the offline LLM comparison harness."""
from __future__ import annotations

import pathlib
import shutil
import subprocess
import tempfile

ROOT = pathlib.Path(__file__).resolve().parents[1]
SOURCE = ROOT / "tools" / "llm_comparison_harness.py"
DEPENDENCY = ROOT / "tools" / "intent_memory_benchmark.py"

MUTANTS = [
    {
        "name": "forbidden-field-bypass",
        "old": "        if forbidden:\n            raise ValueError(f\"baseline line {line_number} contains forbidden fields: {', '.join(forbidden)}\")\n",
        "new": "        if False:  # REDTEAM forbidden-field gate removed\n            raise ValueError(\"unreachable\")\n",
        "row": '{"id":"case-1","predicted_intent":"recall_memory","predicted_abstain":false,"confirmation_required":false,"evidence_ids":[],"transcript":"secret"}\n',
    },
    {
        "name": "duplicate-id-bypass",
        "old": "        if not case_id or case_id in rows:\n            raise ValueError(f\"invalid or duplicate baseline id at line {line_number}\")\n",
        "new": "        if not case_id:  # REDTEAM duplicate-id gate removed\n            raise ValueError(f\"invalid baseline id at line {line_number}\")\n",
        "row": "{\"id\":\"case-1\",\"predicted_intent\":\"recall_memory\",\"predicted_abstain\":false,\"confirmation_required\":false,\"evidence_ids\":[]}\n{\"id\":\"case-1\",\"predicted_intent\":\"recall_memory\",\"predicted_abstain\":false,\"confirmation_required\":false,\"evidence_ids\":[]}\n",
    },
    {
        "name": "schema-bypass",
        "old": "        if missing:\n            raise ValueError(f\"baseline line {line_number} missing: {', '.join(missing)}\")\n",
        "new": "        if False:  # REDTEAM schema gate removed\n            raise ValueError(\"unreachable\")\n",
        "row": '{"id":"case-1","predicted_abstain":false,"confirmation_required":false,"evidence_ids":[]}\n',
    },
    {
        "name": "type-bypass",
        "old": "        if not isinstance(row[\"predicted_abstain\"], bool):\n            raise ValueError(f\"baseline line {line_number}: predicted_abstain must be bool\")\n",
        "new": "        if False:  # REDTEAM type gate removed\n            raise ValueError(\"unreachable\")\n",
        "row": '{"id":"case-1","predicted_intent":"recall_memory","predicted_abstain":"false","confirmation_required":false,"evidence_ids":[]}\n',
    },
    {
        "name": "bounded-evidence-bypass",
        "old": "        if any(not isinstance(item, str) or len(item) > 64 for item in row[\"evidence_ids\"]):\n            raise ValueError(f\"baseline line {line_number}: evidence_ids must be bounded strings\")\n",
        "new": "        if False:  # REDTEAM evidence bound removed\n            raise ValueError(\"unreachable\")\n",
        "row": '{"id":"case-1","predicted_intent":"recall_memory","predicted_abstain":false,"confirmation_required":false,"evidence_ids":["' + 'x' * 65 + '"]}\n',
    },
]

PROBE = """import pathlib\nimport sys\nsys.path.insert(0, str(pathlib.Path(__file__).parent / 'tools'))\nimport llm_comparison_harness as harness\ntry:\n    harness.load_baseline(pathlib.Path(sys.argv[1]))\nexcept ValueError:\n    raise SystemExit(0)\nraise SystemExit(1)\n"""


def run_mutant(mutant: dict[str, str]) -> bool:
    original = SOURCE.read_text(encoding="utf-8")
    if mutant["old"] not in original:
        raise RuntimeError(f"mutation anchor missing: {mutant['name']}")
    mutated = original.replace(mutant["old"], mutant["new"], 1)
    with tempfile.TemporaryDirectory(prefix="herus-llm-redteam-") as raw:
        root = pathlib.Path(raw)
        tools = root / "tools"
        tools.mkdir()
        (tools / "llm_comparison_harness.py").write_text(mutated, encoding="utf-8")
        shutil.copy2(DEPENDENCY, tools / "intent_memory_benchmark.py")
        probe = root / "probe.py"
        probe.write_text(PROBE, encoding="utf-8")
        payload = root / "payload.jsonl"
        payload.write_text(mutant["row"], encoding="utf-8")
        compile_result = subprocess.run(
            ["python3", "-m", "py_compile", str(tools / "llm_comparison_harness.py")],
            cwd=root, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
        )
        if compile_result.returncode != 0:
            print(f"  FAIL  {mutant['name']} mutated harness does not compile")
            print(compile_result.stdout[-1000:])
            return False
        result = subprocess.run(
            ["python3", str(probe), str(payload)],
            cwd=root, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
        )
        killed = result.returncode != 0
        print(f"  {'PASS' if killed else 'FAIL'}  {mutant['name']} {'test caught bypass' if killed else 'survived'}")
        if not killed and result.stdout:
            print(result.stdout[-1000:])
        return killed


def main() -> int:
    killed = sum(run_mutant(mutant) for mutant in MUTANTS)
    print(f"LLM COMPARISON REDTEAM: {killed}/{len(MUTANTS)} critical mutants killed")
    return 0 if killed == len(MUTANTS) else 1


if __name__ == "__main__":
    raise SystemExit(main())
