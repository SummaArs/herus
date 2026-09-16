"""Computational symbiosis laboratory: discover, compile, operate, detach."""
from __future__ import annotations

from dataclasses import dataclass
from typing import Iterable

from active_discovery import ActiveDiscoveryConfig, run_active_discovery
from host_discovery import DiscoveryBudget, DiscoveryResult
from host_discovery_lab import HostOracle, HiddenHost
from host_profile import HostProfile
from symbiotic_models import PersistentIdentity, SymbioticState


@dataclass(frozen=True)
class AdaptationPlan:
    host_session: str
    representations: tuple[str, ...]
    interfaces: tuple[str, ...]
    skills: tuple[str, ...]
    max_payload_bytes: int | None
    latencies_ms: tuple[tuple[str, int], ...]
    status: str


@dataclass(frozen=True)
class SymbiosisRun:
    host_session: str
    discovery: DiscoveryResult
    plan: AdaptationPlan
    state: SymbioticState
    proposal: str
    execution: str


def compile_adaptation(discovery: DiscoveryResult) -> AdaptationPlan:
    hypothesis = discovery.hypothesis
    interfaces = tuple(sorted(hypothesis.proven_interfaces))
    representations = tuple(sorted(hypothesis.proven_formats))
    skills = ["observe"]
    if "haptic" in hypothesis.proven_interfaces:
        skills.append("haptic_feedback")
    if "radio" in hypothesis.proven_interfaces:
        skills.append("bounded_radio")
    if "sensor" in hypothesis.proven_interfaces:
        skills.append("sense")
    status = "ADAPTED" if (
        representations
        and not discovery.blocked_reasons
        and not hypothesis.unknown_formats
        and not hypothesis.unknown_interfaces
    ) else "ADAPTED_WITH_LIMITS"
    return AdaptationPlan(
        host_session=hypothesis.session_id,
        representations=representations,
        interfaces=interfaces,
        skills=tuple(skills),
        max_payload_bytes=hypothesis.max_payload_bytes,
        latencies_ms=tuple(sorted(hypothesis.latencies_ms.items())),
        status=status,
    )


def compile_profile(discovery: DiscoveryResult, plan: AdaptationPlan) -> HostProfile:
    """Compile only from probe evidence; hidden oracle truth never enters the plan."""
    return HostProfile(
        host_id="discovered:" + discovery.hypothesis.session_id,
        revision="probe-v1",
        resources={"memory_bytes": 0},
        interfaces=frozenset(plan.interfaces),
        constraints={"max_payload_bytes": plan.max_payload_bytes or 0, **dict(plan.latencies_ms)},
        representation_set=frozenset(plan.representations),
        skill_budget={"bytes": plan.max_payload_bytes or 0, "steps": discovery.probes_used},
        evidence={"discovery_session": discovery.hypothesis.session_id, "discovery": "verified"},
        authority="NONE",
    )


def run_symbiosis(host: HiddenHost, identity: PersistentIdentity, *, max_probes: int = 32) -> SymbiosisRun:
    oracle = HostOracle(host)
    discovery = run_active_discovery(oracle, ActiveDiscoveryConfig(
        candidate_formats=("HIR8", "HIR16", "HIR32", "SIM-INT8", "SIM-HDC8", "SIM-RULES"),
        candidate_interfaces=("button", "haptic", "radio", "serial", "display", "sensor", "actuator"),
        latency_targets=("latency_budget_ms", "control", "inference"),
        budget=DiscoveryBudget(max_probes=max_probes, max_bytes=10000),
    ))
    plan = compile_adaptation(discovery)
    profile = compile_profile(discovery, plan)
    state = SymbioticState(identity=identity).attach(profile, skills=plan.skills)
    proposal = state.propose("observe")
    return SymbiosisRun(host.session_id, discovery, plan, state, proposal, state.execute("observe"))
