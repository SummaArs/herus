"""Active, budgeted discovery of a hidden host.

The discoverer sees only a probe session and finite candidate vocabulary. It never
receives or constructs authority, effects, identity, or an unproven capability.
"""
from __future__ import annotations

from dataclasses import dataclass, field
import hashlib

from host_discovery_lab import HostOracle, ProbeObservation, ProbeRequest


@dataclass(frozen=True)
class DiscoveryBudget:
    max_probes: int
    max_bytes: int


@dataclass(frozen=True)
class HostHypothesis:
    session_id: str
    proven_formats: frozenset[str]
    proven_interfaces: frozenset[str]
    max_payload_bytes: int | None
    latencies_ms: dict[str, int]
    unknown_formats: frozenset[str]
    unknown_interfaces: frozenset[str]
    authority: str = "NONE"
    allowed_effects: frozenset[str] = frozenset()


@dataclass
class DiscoveryResult:
    hypothesis: HostHypothesis
    observations: tuple[ProbeObservation, ...]
    probes_used: int
    bytes_used: int
    blocked_reasons: tuple[str, ...] = ()


def _valid_observation(observation: ProbeObservation, *, session_id: str, sequence: int) -> bool:
    unsigned = ProbeObservation(**observation.to_dict(False), evidence_digest="")
    expected = hashlib.sha256(unsigned.canonical()).hexdigest()
    return (
        observation.host_session == session_id
        and observation.sequence == sequence
        and observation.evidence_digest == expected
    )


def discover_host(
    oracle: HostOracle,
    *,
    candidate_formats: tuple[str, ...],
    candidate_interfaces: tuple[str, ...],
    latency_targets: tuple[str, ...],
    budget: DiscoveryBudget,
) -> DiscoveryResult:
    """Probe finite candidates; fail closed on budget, digest, or sequence errors."""
    observations: list[ProbeObservation] = []
    blocked: list[str] = []
    sequence = 0
    used_bytes = 0

    requests = [
        *(ProbeRequest("supports_format", value, 0) for value in candidate_formats),
        *(ProbeRequest("has_interface", value, 0) for value in candidate_interfaces),
        ProbeRequest("max_payload_bytes", "", 0),
        *(ProbeRequest("measure_latency", value, 0) for value in latency_targets),
    ]
    proven_formats: set[str] = set()
    proven_interfaces: set[str] = set()
    unknown_formats = set(candidate_formats)
    unknown_interfaces = set(candidate_interfaces)
    latencies: dict[str, int] = {}
    max_payload: int | None = None
    seen_values: dict[tuple[str, str], tuple[str, str]] = {}
    conflicted_formats: set[str] = set()
    conflicted_interfaces: set[str] = set()

    for request in requests:
        if len(observations) >= budget.max_probes:
            blocked.append("probe_budget_exhausted")
            break
        try:
            request = ProbeRequest(request.name, request.argument, sequence)
            observation = oracle.probe(request)
        except ValueError as exc:
            blocked.append(str(exc))
            sequence += 1
            continue
        if not _valid_observation(observation, session_id=oracle.session_id, sequence=sequence):
            blocked.append("evidence_digest_invalid")
            sequence += 1
            continue
        encoded_size = len(observation.canonical())
        if used_bytes + encoded_size > budget.max_bytes:
            blocked.append("byte_budget_exhausted")
            break
        observation_key = (observation.probe, observation.argument)
        previous = seen_values.get(observation_key)
        current = (observation.value, observation.unit)
        if previous is not None and previous != current:
            blocked.append("observation_conflict")
            if observation.probe == "supports_format":
                conflicted_formats.add(observation.argument)
                proven_formats.discard(observation.argument)
            elif observation.probe == "has_interface":
                conflicted_interfaces.add(observation.argument)
                proven_interfaces.discard(observation.argument)
            continue
        seen_values[observation_key] = current
        observations.append(observation)
        used_bytes += encoded_size
        sequence += 1
        if observation.probe == "supports_format":
            unknown_formats.discard(observation.argument)
            if observation.value == "true":
                proven_formats.add(observation.argument)
        elif observation.probe == "has_interface":
            unknown_interfaces.discard(observation.argument)
            if observation.value == "true":
                proven_interfaces.add(observation.argument)
        elif observation.probe == "max_payload_bytes":
            try:
                max_payload = int(observation.value)
            except ValueError:
                blocked.append("payload_observation_invalid")
        elif observation.probe == "measure_latency":
            try:
                if int(observation.value) >= 0:
                    latencies[observation.argument] = int(observation.value)
            except ValueError:
                blocked.append("latency_observation_invalid")

    hypothesis = HostHypothesis(
        session_id=oracle.session_id,
        proven_formats=frozenset(proven_formats - conflicted_formats),
        proven_interfaces=frozenset(proven_interfaces - conflicted_interfaces),
        max_payload_bytes=max_payload,
        latencies_ms=dict(sorted(latencies.items())),
        unknown_formats=frozenset(unknown_formats | conflicted_formats),
        unknown_interfaces=frozenset(unknown_interfaces | conflicted_interfaces),
    )
    return DiscoveryResult(
        hypothesis=hypothesis,
        observations=tuple(observations),
        probes_used=len(observations),
        bytes_used=used_bytes,
        blocked_reasons=tuple(dict.fromkeys(blocked)),
    )
