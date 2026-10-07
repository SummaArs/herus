"""Validate and summarize a clustered causal-ablation sampling design."""
from __future__ import annotations
import json
from collections import Counter
from pathlib import Path

PROTOCOL = Path(__file__).parent / "evidence" / "causal_sampling_protocol_v1.json"
IDENTITY = Path(__file__).parent / "evidence" / "mintrec_s06_identity_manifest_v1.json"


def validate(protocol: dict[str, object]) -> list[str]:
    errors: list[str] = []
    estimand = protocol.get("estimand", {})
    sampling = protocol.get("sampling", {})
    if protocol.get("status") != "protocol_only":
        errors.append("status_must_remain_protocol_only")
    if estimand.get("name") != "paired_intervention_effect":
        errors.append("estimand_not_paired_intervention")
    if sampling.get("method") != "cluster_bootstrap_within_S06":
        errors.append("cluster_bootstrap_required")
    if sampling.get("cluster_key") != "episode":
        errors.append("episode_cluster_key_required")
    if sampling.get("resample_rows_individually") is not False:
        errors.append("row_level_resampling_forbidden")
    if int(sampling.get("iterations", 0)) < 10000:
        errors.append("minimum_bootstrap_iterations_missing")
    required = set(protocol.get("identification_requirements", []))
    for item in ("same example_id is evaluated by treatment and control", "treatment and control code versions are frozen before reading outcomes"):
        if item not in required:
            errors.append("identification_requirement_missing:" + item)
    return errors


def summarize_identity(path: Path = IDENTITY) -> dict[str, int]:
    data = json.loads(path.read_text(encoding="utf-8"))
    clusters = Counter(row["episode"] for row in data["records"])
    return {
        "holdout_rows": len(data["records"]),
        "episode_clusters": len(clusters),
        "largest_cluster": max(clusters.values()),
        "singleton_clusters": sum(value == 1 for value in clusters.values()),
    }


def run() -> dict[str, object]:
    protocol = json.loads(PROTOCOL.read_text(encoding="utf-8"))
    errors = validate(protocol)
    return {
        "decision": "PASS_FOR_DESIGN_ONLY" if not errors else "BLOCK",
        "execution_result_present": False,
        "errors": errors,
        "identity_summary": summarize_identity(),
        "interpretation": "clustered sampling improves uncertainty for repeated episodes; it does not identify a universal causal effect",
    }


if __name__ == "__main__":
    print(json.dumps(run(), ensure_ascii=False, indent=2, sort_keys=True))
