"""Cost and failure measurements for finite policy composition."""
from __future__ import annotations

import json
import time

from .policy_composition import run


def measure() -> dict[str, object]:
    start = time.perf_counter()
    result = run()
    elapsed_ms = round((time.perf_counter() - start) * 1000, 3)
    program = ("SIGNAL:OK=>WAIT", "SIGNAL:TIMEOUT=>ALERT", "SIGNAL:CANCEL=>SAFE")
    encoded_bytes = len("|".join(program).encode("ascii"))
    return {
        "schema": "herus.generative_lab.policy_composition_benchmark",
        "version": 1,
        "authority": "none",
        "elapsed_ms_host": elapsed_ms,
        "program_steps": len(program),
        "program_bytes_ascii": encoded_bytes,
        "passed": result["passed"],
        "failure_count": result["failure_count"],
        "allowed_effects": result["allowed_effects"],
        "limits": {
            "host_timing_is_not_hardware_timing": True,
            "ascii_length_is_not_final_wire_size": True,
            "no_actuator_execution": True,
        },
    }


if __name__ == "__main__":
    print(json.dumps(measure(), indent=2, sort_keys=True))
