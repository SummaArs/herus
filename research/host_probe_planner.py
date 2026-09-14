"""Finite active probe planner for host discovery.

The planner chooses only from the finite allowed vocabulary. It never probes
authority or executes effects; it prioritizes unresolved observations by a
stable cost-aware score.
"""
from __future__ import annotations

from dataclasses import dataclass
from functools import lru_cache

from host_discovery import HostHypothesis
from host_discovery_lab import ProbeRequest


@dataclass(frozen=True)
class ProbePlan:
    request: ProbeRequest
    expected_information: int
    estimated_bytes: int


@lru_cache(maxsize=128)
def _vocabulary(values: tuple[str, ...]) -> frozenset[str]:
    """Cache finite candidate vocabularies without changing their semantics."""
    return frozenset(values)


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
    candidate_format_set = _vocabulary(candidate_formats)
    candidate_interface_set = _vocabulary(candidate_interfaces)
    latency_target_set = _vocabulary(latency_targets)
    measured_targets = frozenset(hypothesis.latencies_ms)
    for fmt in sorted(hypothesis.unknown_formats & candidate_format_set):
        candidates.append(ProbePlan(ProbeRequest("supports_format", fmt, next_sequence), 2, 96))
    for interface in sorted(hypothesis.unknown_interfaces & candidate_interface_set):
        candidates.append(ProbePlan(ProbeRequest("has_interface", interface, next_sequence), 2, 96))
    for target in sorted(latency_target_set - measured_targets):
        candidates.append(ProbePlan(ProbeRequest("measure_latency", target, next_sequence), 1, 96))
    if hypothesis.max_payload_bytes is None:
        candidates.append(ProbePlan(ProbeRequest("max_payload_bytes", "", next_sequence), 3, 96))
    if not candidates:
        return None
    return max(candidates, key=lambda item: (item.expected_information / item.estimated_bytes, item.expected_information, item.request.name, item.request.argument))
