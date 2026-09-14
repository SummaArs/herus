from __future__ import annotations

import json
from pathlib import Path

from host_discovery import DiscoveryBudget, discover_host
from host_discovery_lab import HostOracle, build_hidden_hosts


def main() -> None:
    records = []
    for hidden in build_hidden_hosts():
        result = discover_host(
            HostOracle(hidden),
            candidate_formats=("HIR8", "HIR16", "HIR32", "JSON"),
            candidate_interfaces=("radio", "serial", "haptic", "display", "sensor"),
            latency_targets=("HIR8", "HIR16", "JSON"),
            budget=DiscoveryBudget(max_probes=32, max_bytes=10000),
        )
        truth = hidden.truth
        records.append(
            {
                "session_id": hidden.session_id,
                "truth_host_id": truth.host_id,
                "probes_used": result.probes_used,
                "bytes_used": result.bytes_used,
                "proven_formats": sorted(result.hypothesis.proven_formats),
                "proven_interfaces": sorted(result.hypothesis.proven_interfaces),
                "unknown_formats": sorted(result.hypothesis.unknown_formats),
                "unknown_interfaces": sorted(result.hypothesis.unknown_interfaces),
                "authority": result.hypothesis.authority,
                "allowed_effects": sorted(result.hypothesis.allowed_effects),
                "blocked_reasons": list(result.blocked_reasons),
                "truth_formats": sorted(truth.representation_set),
                "truth_interfaces": sorted(truth.interfaces),
                "false_positive_formats": sorted(result.hypothesis.proven_formats - truth.representation_set),
                "false_positive_interfaces": sorted(result.hypothesis.proven_interfaces - truth.interfaces),
            }
        )
    output = {
        "protocol": "autonomous_host_discovery_v1",
        "status": "host_only",
        "hosts": records,
        "claim": "partial_discovery_with_bounded_probes",
        "not_proven": ["arbitrary_new_host", "physical_discovery", "general_adaptation", "authority_inference"],
    }
    path = Path("evidence/autonomous_host_discovery_v1.json")
    path.write_text(json.dumps(output, indent=2, sort_keys=True) + "\n")
    print(json.dumps(output, indent=2, sort_keys=True))


if __name__ == "__main__":
    main()
