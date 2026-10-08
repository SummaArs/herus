"""Fail-closed validator for the real multi-host adaptation gate."""
from __future__ import annotations
import json
from pathlib import Path

REQUIRED_HOST_FIELDS = {
    "host_id", "fit_count", "holdout_count", "target_feedback_count",
    "retained_evidence", "quarantined_evidence", "leakage_detected",
    "seeds", "herus_score", "baseline_score", "score_higher_is_better",
    "baseline_definition", "seed_runs",
}


def validate(payload: dict) -> tuple[bool, list[str]]:
    errors: list[str] = []
    if payload.get("schema") != "herus-multi-host-gate-v1":
        errors.append("schema_invalid")
    if payload.get("data_origin") != "real_world":
        errors.append("real_world_data_required")
    if not payload.get("dataset_id"):
        errors.append("dataset_identity_required")
    hosts = payload.get("hosts")
    if not isinstance(hosts, list) or len(hosts) < 3:
        errors.append("at_least_three_hosts_required")
        hosts = hosts if isinstance(hosts, list) else []
    ids = [item.get("host_id") for item in hosts]
    if len(ids) != len(set(ids)):
        errors.append("duplicate_host_id")
    for item in hosts:
        missing = sorted(REQUIRED_HOST_FIELDS - set(item))
        if missing:
            errors.append(f"{item.get('host_id', '<unknown>')}:missing:{','.join(missing)}")
            continue
        if item["fit_count"] <= 0 or item["holdout_count"] <= 0:
            errors.append(f"{item['host_id']}:empty_split")
        if item["target_feedback_count"] < 0:
            errors.append(f"{item['host_id']}:negative_feedback_count")
        if item["leakage_detected"]:
            errors.append(f"{item['host_id']}:leakage_detected")
        if not isinstance(item["seeds"], list) or len(item["seeds"]) < 3:
            errors.append(f"{item['host_id']}:at_least_three_seeds_required")
        if not isinstance(item["seed_runs"], list) or len(item["seed_runs"]) != len(item["seeds"]):
            errors.append(f"{item['host_id']}:seed_ledger_incomplete")
        else:
            for run in item["seed_runs"]:
                if "adapter" not in run or "coverage" not in run["adapter"] or "selective_accuracy" not in run["adapter"]:
                    errors.append(f"{item['host_id']}:adapter_coverage_or_precision_missing")
                if "strongest_baseline_selective_accuracy" not in run:
                    errors.append(f"{item['host_id']}:strong_baseline_missing")
        if item["retained_evidence"] < 0 or item["quarantined_evidence"] < 0:
            errors.append(f"{item['host_id']}:negative_evidence_count")
        if not isinstance(item["score_higher_is_better"], bool):
            errors.append(f"{item['host_id']}:score_direction_required")
    return not errors, errors


def decision(payload: dict) -> dict:
    valid, errors = validate(payload)
    if not valid:
        return {
            "schema": "herus-multi-host-gate-decision-v1",
            "status": "BLOCKED",
            "scientific_claim": "not_proven",
            "errors": errors,
            "claim_allowed": False,
        }
    return {
        "schema": "herus-multi-host-gate-decision-v1",
        "status": "READY_FOR_ANALYSIS",
        "scientific_claim": "mechanism_only",
        "errors": [],
        "claim_allowed": False,
        "reason": "valid_protocol_is_not_evidence_of_SOTA",
    }


def main(path: str) -> int:
    payload = json.loads(Path(path).read_text(encoding="utf-8"))
    print(json.dumps(decision(payload), indent=2, sort_keys=True))
    return 0 if validate(payload)[0] else 1


if __name__ == "__main__":
    raise SystemExit(main("research/evidence/multi_host_gate_v1.json"))
