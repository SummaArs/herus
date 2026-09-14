from __future__ import annotations

import json
from pathlib import Path

from host_discovery import DiscoveryBudget, discover_host
from host_discovery_lab import HostOracle, build_hidden_hosts


def run() -> dict:
    rows = []
    candidate_formats = ("HIR8", "HIR16", "HIR32", "JSON")
    candidate_interfaces = ("radio", "serial", "haptic", "display", "sensor")
    latency_targets = ("HIR8", "HIR16", "JSON")
    for hidden in build_hidden_hosts():
        for budget in (
            DiscoveryBudget(2, 10000),
            DiscoveryBudget(8, 10000),
            DiscoveryBudget(32, 10000),
        ):
            result = discover_host(
                HostOracle(hidden),
                candidate_formats=candidate_formats,
                candidate_interfaces=candidate_interfaces,
                latency_targets=latency_targets,
                budget=budget,
            )
            truth_formats = hidden.truth.representation_set
            truth_interfaces = hidden.truth.interfaces
            proven = result.hypothesis.proven_formats | result.hypothesis.proven_interfaces
            truth = truth_formats | truth_interfaces
            rows.append(
                {
                    "session_id": hidden.session_id,
                    "max_probes": budget.max_probes,
                    "probes_used": result.probes_used,
                    "bytes_used": result.bytes_used,
                    "coverage": round(len(proven & truth) / len(truth), 4),
                    "false_positive_count": len(proven - truth),
                    "abstained": bool(result.blocked_reasons) or bool(result.hypothesis.unknown_formats | result.hypothesis.unknown_interfaces),
                    "authority": result.hypothesis.authority,
                    "allowed_effects": sorted(result.hypothesis.allowed_effects),
                    "blocked_reasons": list(result.blocked_reasons),
                }
            )
    return {
        "protocol": "autonomous_host_discovery_benchmark_v1",
        "status": "host_only",
        "rows": rows,
        "claims": [
            "bounded_cost_is_measured",
            "abstention_is_measured",
            "false_positive_count_is_measured",
        ],
        "not_proven": ["physical_host_discovery", "arbitrary_host_generalization", "authority_inference"],
    }


def main() -> None:
    result = run()
    Path("evidence/autonomous_host_discovery_benchmark_v1.json").write_text(
        json.dumps(result, indent=2, sort_keys=True) + "\n"
    )
    print(json.dumps(result, indent=2, sort_keys=True))


if __name__ == "__main__":
    main()
