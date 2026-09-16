from __future__ import annotations
import json
from pathlib import Path
from host_discovery_lab import build_hidden_hosts
from symbiotic_models import PersistentIdentity
from symbiosis_lab import run_symbiosis


def build_evidence() -> dict[str, object]:
    rows = []
    for host in build_hidden_hosts():
        run = run_symbiosis(host, PersistentIdentity("herus-general-lab"))
        rows.append({
            "session": run.host_session,
            "host_id": run.state.host.host_id,
            "representations": list(run.plan.representations),
            "interfaces": list(run.plan.interfaces),
            "skills": list(run.plan.skills),
            "status": run.plan.status,
            "blocked_reasons": list(run.discovery.blocked_reasons),
            "proposal": run.proposal,
            "execution": run.execution,
            "authority": run.state.self_model.authority,
            "world_observations_after_bind": len(run.state.world.observations),
            "state_valid": not run.state.validate(),
        })
    return {
        "schema": "herus.computational-symbiosis-lab.v1",
        "identity": "herus-general-lab",
        "hosts_tested": len(rows),
        "rows": rows,
        "claim": "The same bounded core discovers finite host capabilities, compiles a host-specific adaptation plan, proposes only within discovered scope, and never gains execution authority.",
        "not_claimed": ["physical operation", "arbitrary open-world generality", "ESP32 measurements", "wrist interaction"],
    }


if __name__ == "__main__":
    output = build_evidence()
    path = Path(__file__).parent / "evidence" / "computational_symbiosis_lab.json"
    path.write_text(json.dumps(output, indent=2, ensure_ascii=False) + "\n")
    print(json.dumps(output, indent=2, ensure_ascii=False))
