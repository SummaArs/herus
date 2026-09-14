"""Quantify active host discovery before physical validation."""
from __future__ import annotations

import json
from dataclasses import asdict

from adaptive_cycle import run_cycle
from host_discovery import DiscoveryBudget
from host_discovery_lab import HostOracle, build_hidden_hosts


def run_benchmark() -> dict:
    rows = []
    for hidden in build_hidden_hosts():
        for max_bytes in (192, 768, 4096):
            hypothesis, records = run_cycle(
                HostOracle(hidden),
                candidate_formats=("HIR8", "HIR16", "HIR32", "JSON"),
                candidate_interfaces=("radio", "serial", "haptic", "display", "sensor"),
                latency_targets=("HIR8", "HIR16", "JSON"),
                budget=DiscoveryBudget(max_probes=32, max_bytes=max_bytes),
            )
            rows.append({
                "host": hidden.session_id,
                "budget_bytes": max_bytes,
                "records": len(records),
                "observations": sum(r.outcome == "OBSERVED" for r in records),
                "abstentions": sum(r.outcome == "ABSTAIN" for r in records),
                "proven_formats": sorted(hypothesis.proven_formats),
                "proven_interfaces": sorted(hypothesis.proven_interfaces),
                "authority": hypothesis.authority,
                "allowed_effects": sorted(hypothesis.allowed_effects),
            })
    return {
        "claim": "host_only_active_discovery_cost_and_generalization",
        "rows": rows,
        "limitations": [
            "hosts are deterministic laboratory oracles",
            "no physical timing, power, radio or memory claim",
            "authority is never discovered or granted",
        ],
    }


if __name__ == "__main__":
    print(json.dumps(run_benchmark(), sort_keys=True, indent=2))
