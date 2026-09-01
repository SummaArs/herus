"""Finite-state refinement checks for the critical-systems track.

This checks trace inclusion of a concrete transition table in an abstract one
under an explicit state map. It does not prove that the map models reality.
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


def check_refinement(
    abstract: StateMachineSpec,
    concrete: StateMachineSpec,
    state_map: dict[str, str],
) -> RefinementCertificate:
    if not state_map:
        return RefinementCertificate(RefinementVerdict.INVALID, "empty_state_map")
    if set(state_map) != set(concrete.states):
        return RefinementCertificate(RefinementVerdict.INVALID, "incomplete_state_map")
    if any(mapped not in abstract.states for mapped in state_map.values()):
        return RefinementCertificate(RefinementVerdict.INVALID, "mapped_state_outside_abstract")
    if state_map[concrete.initial_state] != abstract.initial_state:
        return RefinementCertificate(RefinementVerdict.INVALID, "initial_state_not_refined")
    if not concrete.inputs.issubset(abstract.inputs):
        return RefinementCertificate(RefinementVerdict.INVALID, "concrete_input_outside_abstract")
    if not concrete.actions.issubset(abstract.actions):
        return RefinementCertificate(RefinementVerdict.INVALID, "concrete_action_outside_abstract")

    abstract_edges = {
        (item.state, item.input_symbol, item.action): item
        for item in abstract.transitions
    }
    for concrete_edge in concrete.transitions:
        key = (
            state_map[concrete_edge.state],
            concrete_edge.input_symbol,
            concrete_edge.action,
        )
        abstract_edge = abstract_edges.get(key)
        if abstract_edge is None:
            return RefinementCertificate(
                RefinementVerdict.COUNTEREXAMPLE,
                "abstract_transition_missing",
                concrete_edge,
                None,
            )
        if state_map[concrete_edge.next_state] != abstract_edge.next_state:
            return RefinementCertificate(
                RefinementVerdict.COUNTEREXAMPLE,
                "next_state_not_refined",
                concrete_edge,
                abstract_edge,
            )
    return RefinementCertificate(RefinementVerdict.REFINED, "all_concrete_edges_refine")
