"""Small deterministic benchmark for host-probe planning.

The benchmark is intentionally host-only. It reports wall-clock time for a fixed
number of identical planning calls and does not claim physical energy savings.
"""
from __future__ import annotations

import json
import os
import statistics
import time

from host_discovery import HostHypothesis
if os.environ.get("HERUS_BENCH_BASELINE") == "1":
    from host_probe_planner_baseline import choose_next_probe
else:
    from host_probe_planner import choose_next_probe


ITERATIONS = 200_000
REPEATS = 5


def workload() -> HostHypothesis:
    return HostHypothesis(
        session_id="optimization-bench",
        proven_formats=frozenset(),
        proven_interfaces=frozenset(),
        max_payload_bytes=None,
        latencies_ms={},
        unknown_formats=frozenset({"hir-v1", "hir-v2", "json", "cbor"}),
        unknown_interfaces=frozenset({"button", "haptic", "radio", "serial"}),
    )


def run_once() -> float:
    hypothesis = workload()
    formats = ("hir-v1", "hir-v2", "json", "cbor", "unused")
    interfaces = ("button", "haptic", "radio", "serial", "unused")
    targets = ("control", "telemetry", "bulk", "unused")
    start = time.perf_counter()
    checksum = 0
    for sequence in range(ITERATIONS):
        plan = choose_next_probe(
            hypothesis,
            candidate_formats=formats,
            candidate_interfaces=interfaces,
            latency_targets=targets,
            next_sequence=sequence,
        )
        if plan is None:
            raise AssertionError("benchmark workload unexpectedly exhausted")
        checksum += plan.expected_information + plan.estimated_bytes + len(plan.request.argument)
    elapsed = time.perf_counter() - start
    if checksum <= 0:
        raise AssertionError("invalid benchmark checksum")
    return elapsed


def main() -> None:
    samples = [run_once() for _ in range(REPEATS)]
    print(json.dumps({
        "iterations": ITERATIONS,
        "repeats": REPEATS,
        "seconds": samples,
        "median_seconds": statistics.median(samples),
        "calls_per_second": ITERATIONS / statistics.median(samples),
    }, sort_keys=True, indent=2))


if __name__ == "__main__":
    main()
