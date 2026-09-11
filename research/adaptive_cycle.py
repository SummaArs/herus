"""Bounded adaptive discovery cycle with auditable experiment records."""
from __future__ import annotations

from dataclasses import replace

from experiment_log import ExperimentRecord, make_record
from host_discovery import DiscoveryBudget, HostHypothesis, _valid_observation
from host_discovery_lab import HostOracle, ProbeObservation, ProbeRequest
from host_probe_planner import choose_next_probe


def _empty_hypothesis(session_id: str) -> HostHypothesis:
    return HostHypothesis(
        session_id=session_id,
        proven_formats=frozenset(),
        proven_interfaces=frozenset(),
        max_payload_bytes=None,
        latencies_ms={},
        unknown_formats=frozenset(),
        unknown_interfaces=frozenset(),
    )


def run_cycle(
    oracle: HostOracle,
    *,
    candidate_formats: tuple[str, ...],
    candidate_interfaces: tuple[str, ...],
    latency_targets: tuple[str, ...],
    budget: DiscoveryBudget,
) -> tuple[HostHypothesis, tuple[ExperimentRecord, ...]]:
    """Choose one probe at a time; fail closed on invalid observations."""
    hypothesis = replace(
        _empty_hypothesis(oracle.session_id),
        unknown_formats=frozenset(candidate_formats),
        unknown_interfaces=frozenset(candidate_interfaces),
    )
    records: list[ExperimentRecord] = []
    used_bytes = 0
    sequence = 0
    while sequence < budget.max_probes:
        plan = choose_next_probe(
            hypothesis,
            candidate_formats=candidate_formats,
            candidate_interfaces=candidate_interfaces,
            latency_targets=latency_targets,
            next_sequence=sequence,
        )
        if plan is None:
            break
        if used_bytes + plan.estimated_bytes > budget.max_bytes:
            records.append(make_record(
                session_id=oracle.session_id,
                sequence=sequence,
                hypothesis="unresolved capability",
                probe=plan.request.name,
                argument=plan.request.argument,
                expected_information=plan.expected_information,
                estimated_bytes=plan.estimated_bytes,
                observed_value=None,
                outcome="ABSTAIN",
                abstention_reason="byte_budget_exhausted",
            ))
            break
        try:
            observation = oracle.probe(plan.request)
        except ValueError as exc:
            records.append(make_record(
                session_id=oracle.session_id,
                sequence=sequence,
                hypothesis="probe rejected",
                probe=plan.request.name,
                argument=plan.request.argument,
                expected_information=plan.expected_information,
                estimated_bytes=plan.estimated_bytes,
                observed_value=None,
                outcome="ABSTAIN",
                abstention_reason=str(exc),
            ))
            break
        valid = _valid_observation(observation, session_id=oracle.session_id, sequence=sequence)
        if not valid:
            records.append(make_record(
                session_id=oracle.session_id,
                sequence=sequence,
                hypothesis="invalid observation",
                probe=plan.request.name,
                argument=plan.request.argument,
                expected_information=plan.expected_information,
                estimated_bytes=plan.estimated_bytes,
                observed_value=None,
                outcome="ABSTAIN",
                abstention_reason="evidence_invalid",
            ))
            break
        records.append(make_record(
            session_id=oracle.session_id,
            sequence=sequence,
            hypothesis="unresolved capability",
            probe=observation.probe,
            argument=observation.argument,
            expected_information=plan.expected_information,
            estimated_bytes=plan.estimated_bytes,
            observed_value=observation.value,
            outcome="OBSERVED",
        ))
        used_bytes += len(observation.canonical())
        if observation.probe == "supports_format":
            unknown = set(hypothesis.unknown_formats)
            unknown.discard(observation.argument)
            proven = set(hypothesis.proven_formats)
            if observation.value == "true":
                proven.add(observation.argument)
            hypothesis = replace(hypothesis, unknown_formats=frozenset(unknown), proven_formats=frozenset(proven))
        elif observation.probe == "has_interface":
            unknown = set(hypothesis.unknown_interfaces)
            unknown.discard(observation.argument)
            proven = set(hypothesis.proven_interfaces)
            if observation.value == "true":
                proven.add(observation.argument)
            hypothesis = replace(hypothesis, unknown_interfaces=frozenset(unknown), proven_interfaces=frozenset(proven))
        elif observation.probe == "max_payload_bytes":
            try:
                hypothesis = replace(hypothesis, max_payload_bytes=int(observation.value))
            except ValueError:
                pass
        elif observation.probe == "measure_latency":
            try:
                latencies = dict(hypothesis.latencies_ms)
                latencies[observation.argument] = int(observation.value)
                hypothesis = replace(hypothesis, latencies_ms=latencies)
            except ValueError:
                pass
        sequence += 1
    return hypothesis, tuple(records)
