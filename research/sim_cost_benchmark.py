"""Host-only cost measurement; never presented as ESP32 performance."""
from __future__ import annotations
import json
import time

from symbiotic_intelligence import MicroMLP, Pattern
from sim_local_learning import LocalPrototypeBank


def run(iterations: int = 10000) -> dict[str, object]:
    pattern = Pattern((4, 0, 0, 0), "bench", "d")
    model = MicroMLP()
    start = time.perf_counter_ns()
    for _ in range(iterations):
        model.infer(pattern)
    infer_ns = (time.perf_counter_ns() - start) / iterations
    bank = LocalPrototypeBank()
    start = time.perf_counter_ns()
    for _ in range(min(iterations, 8)):
        bank.update(pattern, "ARRIVE")
    update_ns = (time.perf_counter_ns() - start) / min(iterations, 8)
    return {"schema": "herus.sim.cost.v1", "iterations": iterations, "infer_mean_ns_host": round(infer_ns, 2), "update_mean_ns_host": round(update_ns, 2), "authority": "NONE", "hardware_claim": False}


if __name__ == "__main__":
    print(json.dumps(run(), indent=2, sort_keys=True))
