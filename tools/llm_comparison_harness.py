#!/usr/bin/env python3
"""Offline, abstention-aware comparison harness for HERUS and local baselines.

The harness deliberately does not call hosted models and does not manufacture a
baseline when no local model result file is supplied. A local baseline file is
an input artifact, not executable code.
"""
from __future__ import annotations

import argparse
import hashlib
import json
import pathlib
import sys
from typing import Any

ROOT = pathlib.Path(__file__).resolve().parents[1]
CASES = ROOT / "research" / "benchmarks" / "intent_memory" / "cases.jsonl"
DEFAULT_RESULTS = ROOT / "research" / "benchmarks" / "llm_comparison" / "results.json"
FORBIDDEN_FIELDS = {
    "audio", "raw_audio", "transcript", "raw_transcript", "embedding",
    "identity", "voice_identity", "location", "secret", "key",
    "cryptographic_key", "network", "network_payload",
}

sys.path.insert(0, str(ROOT / "tools"))
import intent_memory_benchmark as imb  # noqa: E402


def digest(path: pathlib.Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def load_baseline(path: pathlib.Path) -> dict[str, dict[str, Any]]:
    rows: dict[str, dict[str, Any]] = {}
    for line_number, line in enumerate(path.read_text(encoding="utf-8").splitlines(), 1):
        if not line.strip():
            continue
        row = json.loads(line)
        case_id = str(row.get("id", ""))
        if not case_id or case_id in rows:
            raise ValueError(f"invalid or duplicate baseline id at line {line_number}")
        forbidden = sorted(FORBIDDEN_FIELDS.intersection(row.keys()))
        if forbidden:
            raise ValueError(f"baseline line {line_number} contains forbidden fields: {', '.join(forbidden)}")
        required = {"id", "predicted_intent", "predicted_abstain", "confirmation_required", "evidence_ids"}
        missing = sorted(required - row.keys())
        if missing:
            raise ValueError(f"baseline line {line_number} missing: {', '.join(missing)}")
        if not isinstance(row["predicted_abstain"], bool):
            raise ValueError(f"baseline line {line_number}: predicted_abstain must be bool")
        if not isinstance(row["confirmation_required"], bool):
            raise ValueError(f"baseline line {line_number}: confirmation_required must be bool")
        if not isinstance(row["evidence_ids"], list):
            raise ValueError(f"baseline line {line_number}: evidence_ids must be list")
        if any(not isinstance(item, str) or len(item) > 64 for item in row["evidence_ids"]):
            raise ValueError(f"baseline line {line_number}: evidence_ids must be bounded strings")
        rows[case_id] = row
    return rows


def evaluate_baseline(cases: list[dict[str, Any]], rows: dict[str, dict[str, Any]]) -> dict[str, Any]:
    test_cases = [case for case in cases if case["split"] == "test"]
    scored: list[dict[str, Any]] = []
    for case in test_cases:
        case_id = str(case["id"])
        if case_id not in rows:
            raise ValueError(f"baseline has no row for test case {case_id}")
        row = rows[case_id]
        expected_evidence = {str(item) for item in case["expected_evidence"]}
        actual_evidence = {str(item) for item in row["evidence_ids"]}
        evidence_correct = expected_evidence.issubset(actual_evidence) if expected_evidence else not actual_evidence
        scored.append({
            "id": case_id,
            "gold_intent": case["intent"],
            "predicted_intent": str(row["predicted_intent"]),
            "intent_correct": str(row["predicted_intent"]) == str(case["intent"]),
            "gold_abstain": bool(case["should_abstain"]),
            "predicted_abstain": bool(row["predicted_abstain"]),
            "abstain_correct": bool(row["predicted_abstain"]) == bool(case["should_abstain"]),
            "confirmation_required": bool(row["confirmation_required"]),
            "confirmation_safe": bool(row["confirmation_required"]) == bool(case["action_requires_confirmation"]),
            "evidence_expected": sorted(expected_evidence),
            "evidence_actual": sorted(actual_evidence),
            "evidence_correct": evidence_correct,
        })
    total = len(scored)
    return {
        "cases": total,
        "intent_accuracy": sum(bool(row["intent_correct"]) for row in scored) / total,
        "abstention_accuracy": sum(bool(row["abstain_correct"]) for row in scored) / total,
        "confirmation_safety": sum(bool(row["confirmation_safe"]) for row in scored) / total,
        "evidence_accuracy": sum(bool(row["evidence_correct"]) for row in scored) / total,
        "rows": scored,
    }


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--baseline-jsonl", type=pathlib.Path,
                        help="offline local-baseline output rows; never fetched or executed")
    parser.add_argument("--results", type=pathlib.Path, default=DEFAULT_RESULTS)
    parser.add_argument("--require-baseline", action="store_true",
                        help="fail if a local baseline result file is not supplied")
    args = parser.parse_args()

    cases = imb.read_cases()
    train = [case for case in cases if case["split"] == "train"]
    centroids = imb.build_centroids(train)
    herus = imb.evaluate(cases, centroids, typed=True)
    ablation = imb.evaluate(cases, centroids, typed=False)

    report: dict[str, Any] = {
        "schema": "herus.llm_comparison_results.v1",
        "method": "offline frozen-case comparison with typed HERUS router and optional local baseline artifact",
        "protocol": "research/llm_comparison_protocol_v1.json",
        "corpus": str(CASES.relative_to(ROOT)),
        "corpus_sha256": digest(CASES),
        "test_cases": len(cases) - len(train),
        "systems": {
            "herus_current": herus,
            "herus_similarity_only_ablation": ablation,
        },
        "baseline": {
            "status": "not_supplied",
            "hosted_model_called": False,
            "rows": 0,
        },
        "limits": {
            "not_open_domain": True,
            "not_conversational_llm_parity": True,
            "not_hardware_measurement": True,
            "not_human_generation_quality": True,
        },
    }

    if args.baseline_jsonl is not None:
        baseline_rows = load_baseline(args.baseline_jsonl)
        baseline = evaluate_baseline(cases, baseline_rows)
        report["systems"]["local_llm_baseline"] = baseline
        report["baseline"] = {
            "status": "supplied_offline_artifact",
            "path": str(args.baseline_jsonl),
            "sha256": digest(args.baseline_jsonl),
            "hosted_model_called": False,
            "rows": len(baseline_rows),
        }
    elif args.require_baseline:
        print("LLM COMPARISON HARNESS: FAIL local baseline required")
        return 1

    args.results.parent.mkdir(parents=True, exist_ok=True)
    args.results.write_text(json.dumps(report, indent=2, ensure_ascii=False) + "\n", encoding="utf-8")
    print("LLM COMPARISON HARNESS: PASS")
    print(f"  HERUS typed intent={herus['intent_accuracy']:.6f} abstention={herus['abstention_accuracy']:.6f}")
    print(f"  HERUS ablation intent={ablation['intent_accuracy']:.6f} abstention={ablation['abstention_accuracy']:.6f}")
    print(f"  local baseline={report['baseline']['status']}")
    print(f"  results={args.results}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
