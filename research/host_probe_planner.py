"""Finite active probe planner for host discovery.

The planner chooses only from the finite allowed vocabulary. It never probes
authority or executes effects; it prioritizes unresolved observations by a
stable cost-aware score.
"""
from __future__ import annotations

from dataclasses import dataclass

from host_discovery import HostHypothesis
from host_discovery_lab import ProbeRequest


@dataclass(frozen=True)
class ProbePlan:
    request: ProbeRequest
    expected_information: int
    estimated_bytes: int


def choose_next_probe(
    hypothesis: HostHypothesis,
    *,
    candidate_formats: tuple[str, ...],
    candidate_interfaces: tuple[str, ...],
    latency_targets: tuple[str, ...],
    next_sequence: int,
) -> ProbePlan | None:
    """Choose the cheapest unresolved probe with deterministic tie-breaking."""
    candidates: list[ProbePlan] = []
    for fmt in sorted(hypothesis.unknown_formats & set(candidate_formats)):
        candidates.append(ProbePlan(ProbeRequest("supports_format", fmt, next_sequence), 2, 96))
    for interface in sorted(hypothesis.unknown_interfaces & set(candidate_interfaces)):
        candidates.append(ProbePlan(ProbeRequest("has_interface", interface, next_sequence), 2, 96))
    for target in sorted(set(latency_targets) - set(hypothesis.latencies_ms)):
        candidates.append(ProbePlan(ProbeRequest("measure_latency", target, next_sequence), 1, 96))
    if hypothesis.max_payload_bytes is None:
        candidates.append(ProbePlan(ProbeRequest("max_payload_bytes", "", next_sequence), 3, 96))
    if not candidates:
        return None
    return max(candidates, key=lambda item: (item.expected_information / item.estimated_bytes, item.expected_information, item.request.name, item.request.argument))
