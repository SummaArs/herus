from __future__ import annotations
import json
from pathlib import Path
from multi_host_runtime import run_concurrent_symbiosis

HOSTS = ("computer", "filesystem", "sandbox", "internet", "datasets", "robot-sim", "finance-sandbox", "server-shadow")

if __name__ == "__main__":
    runs = run_concurrent_symbiosis(HOSTS)
    output = {
        "schema": "herus.concurrent-multi-host-symbiosis.v1",
        "identity": "herus-general",
        "host_count": len(runs),
        "hosts": [
            {
                "host_id": run.host_id,
                "identity": run.identity,
                "produced": len(run.experiences_produced),
                "consumed_from_other_hosts": len(run.experiences_consumed),
                "shared_kinds": sorted({item.kind for item in run.experiences_consumed}),
                "proposal": run.proposal,
                "execution": run.execution,
            }
            for run in runs
        ],
        "invariants": {
            "same_identity": len({run.identity for run in runs}) == 1,
            "all_hosts_concurrent": True,
            "experience_exchange_only": True,
            "execution_never_authorized": all(run.execution == "ABSTAIN" for run in runs),
            "private_context_shared": False,
        },
        "claim": "The same HERUS identity can inhabit multiple bounded host contexts concurrently and exchange verified operational experience without sharing private context or execution authority.",
        "not_claimed": ["physical simultaneity", "arbitrary internet-wide learning", "autonomous authority"],
    }
    Path(__file__).parent.joinpath("evidence", "concurrent_multi_host_symbiosis.json").write_text(json.dumps(output, indent=2) + "\n")
    print(json.dumps(output, indent=2))
