"""Small reproducible benchmark for the HERUS host/skill hypothesis.

Run from the repository root:
    python -m research.symbiont_v2.benchmark

This is not an AGI benchmark. It tests four narrow claims: discovery,
verification, persistent identity, and transfer through an abstract effect
contract when primitive action names change.
"""
from __future__ import annotations

import json
import time

from .core import Goal, SymbiontRuntime
from .sim_hosts import host_a, host_b


def run() -> dict[str, object]:
    started = time.perf_counter()
    runtime = SymbiontRuntime("benchmark-herus")
    source = host_a()
    evidence = runtime.discover(source)
    skill = runtime.synthesize(Goal.from_dict({"x": 1}))
    promoted = runtime.promote(skill) if skill is not None else None
    transfer = runtime.transfer(skill.skill_id, host_b()) if promoted else False
    elapsed_ms = round((time.perf_counter() - started) * 1000, 3)
    return {
        "schema": 1,
        "discovered_actions": len(evidence),
        "skill_verified": bool(skill and skill.status == "VERIFIED"),
        "skill_promoted": promoted is not None,
        "identity_persistent": runtime.herus_id == "benchmark-herus",
        "transfer_success": transfer,
        "elapsed_ms": elapsed_ms,
        "interpretation": "bounded evidence for host-independent skill grounding; not general intelligence",
    }


if __name__ == "__main__":
    print(json.dumps(run(), indent=2, sort_keys=True))
