"""Trace-inclusion checks for finite abstract/concrete state machines.

The checker is finite and explicit. It does not prove that either machine models
reality, and it returns INVALID/UNKNOWN rather than inferring a missing mapping.
"""
from __future__ import annotations

from dataclasses import dataclass
from enum import Enum

from critical_state_verifier import StateMachineSpec, Transition


class RefinementVerdict(str, Enum):
    REFINED = "REFINED"
    COUNTEREXAMPLE = "COUNTEREXAMPLE"
    UNKNOWN = "UNKNOWN"
    INVALID = "INVALID"


@dataclass(frozen=True)
class RefinementCertificate:
    verdict: RefinementVerdict
    reason: str
    concrete_transition: Transition | None = None
    abstract_transition: Transition | None = None


def _total_map(mapping: dict[str, str] | None, domain: frozenset[str], label: str) -> tuple[dict[str, str] | None, str | None]:
    if mapping is None:
        mapping = {item: item for item in domain}
    if set(mapping) != set(domain):
        return None, f"incomplete_{label}_map"
    if any(not isinstance(source, str) or not isinstance(target, str) for source, target in mapping.items()):
        return None, f"invalid_{label}_map"
    return mapping, None


def check_refinement_with_maps(
    abstract: StateMachineSpec,
    concrete: StateMachineSpec,
    state_map: dict[str, str],
    input_map: dict[str, str] | None = None,
    action_map: dict[str, str] | None = None,
) -> RefinementCertificate:
    if not state_map:
        return RefinementCertificate(RefinementVerdict.INVALID, "empty_state_map")
    states, error = _total_map(state_map, concrete.states, "state")
    if error is not None or states is None:
        return RefinementCertificate(RefinementVerdict.INVALID, error or "invalid_state_map")
    inputs, error = _total_map(input_map, concrete.inputs, "input")
    if error is not None or inputs is None:
        return RefinementCertificate(RefinementVerdict.INVALID, error or "invalid_input_map")
    actions, error = _total_map(action_map, concrete.actions, "action")
    if error is not None or actions is None:
        return RefinementCertificate(RefinementVerdict.INVALID, error or "invalid_action_map")
    if any(mapped not in abstract.states for mapped in states.values()):
        return RefinementCertificate(RefinementVerdict.INVALID, "mapped_state_outside_abstract")
    if any(mapped not in abstract.inputs for mapped in inputs.values()):
        return RefinementCertificate(RefinementVerdict.INVALID, "mapped_input_outside_abstract")
    if any(mapped not in abstract.actions for mapped in actions.values()):
        return RefinementCertificate(RefinementVerdict.INVALID, "mapped_action_outside_abstract")
    if states[concrete.initial_state] != abstract.initial_state:
        return RefinementCertificate(RefinementVerdict.INVALID, "initial_state_not_refined")

    abstract_edges = {
        (item.state, item.input_symbol, item.action): item
        for item in abstract.transitions
    }
    for concrete_edge in concrete.transitions:
        key = (
            states[concrete_edge.state],
            inputs[concrete_edge.input_symbol],
            actions[concrete_edge.action],
        )
        abstract_edge = abstract_edges.get(key)
        if abstract_edge is None:
            return RefinementCertificate(
                RefinementVerdict.COUNTEREXAMPLE,
                "abstract_transition_missing",
                concrete_edge,
                None,
            )
        if states[concrete_edge.next_state] != abstract_edge.next_state:
            return RefinementCertificate(
                RefinementVerdict.COUNTEREXAMPLE,
                "next_state_not_refined",
                concrete_edge,
                abstract_edge,
            )
    return RefinementCertificate(RefinementVerdict.REFINED, "all_concrete_edges_refine")


def check_refinement(
    abstract: StateMachineSpec,
    concrete: StateMachineSpec,
    state_map: dict[str, str],
) -> RefinementCertificate:
    """Backward-compatible identity mapping for inputs and actions."""
    return check_refinement_with_maps(abstract, concrete, state_map)
