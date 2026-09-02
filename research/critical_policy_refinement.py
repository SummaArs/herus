"""Finite policy refinement checks over explicit state/input/action maps."""
from __future__ import annotations

from dataclasses import dataclass
from enum import Enum

from critical_state_verifier import PolicyRule, StateMachineSpec


class PolicyRefinementVerdict(str, Enum):
    REFINED = "REFINED"
    COUNTEREXAMPLE = "COUNTEREXAMPLE"
    INVALID = "INVALID"


@dataclass(frozen=True)
class PolicyRefinementCertificate:
    verdict: PolicyRefinementVerdict
    reason: str
    concrete_rule: PolicyRule | None = None
    abstract_rule: PolicyRule | None = None


def check_policy_refinement(
    abstract: StateMachineSpec,
    concrete: StateMachineSpec,
    abstract_policy: tuple[PolicyRule, ...],
    concrete_policy: tuple[PolicyRule, ...],
    state_map: dict[str, str],
    input_map: dict[str, str],
    action_map: dict[str, str],
) -> PolicyRefinementCertificate:
    if set(state_map) != set(concrete.states):
        return PolicyRefinementCertificate(PolicyRefinementVerdict.INVALID, "incomplete_state_map")
    if set(input_map) != set(concrete.inputs):
        return PolicyRefinementCertificate(PolicyRefinementVerdict.INVALID, "incomplete_input_map")
    if set(action_map) != set(concrete.actions):
        return PolicyRefinementCertificate(PolicyRefinementVerdict.INVALID, "incomplete_action_map")
    if any(target not in abstract.states for target in state_map.values()):
        return PolicyRefinementCertificate(PolicyRefinementVerdict.INVALID, "mapped_state_outside_abstract")
    if any(target not in abstract.inputs for target in input_map.values()):
        return PolicyRefinementCertificate(PolicyRefinementVerdict.INVALID, "mapped_input_outside_abstract")
    if any(target not in abstract.actions for target in action_map.values()):
        return PolicyRefinementCertificate(PolicyRefinementVerdict.INVALID, "mapped_action_outside_abstract")

    abstract_rules = {(rule.state, rule.input_symbol): rule for rule in abstract_policy}
    concrete_rules = {(rule.state, rule.input_symbol): rule for rule in concrete_policy}
    expected_abstract_keys = {(state, symbol) for state in abstract.states for symbol in abstract.inputs}
    expected_concrete_keys = {(state, symbol) for state in concrete.states for symbol in concrete.inputs}
    if set(abstract_rules) != expected_abstract_keys:
        return PolicyRefinementCertificate(PolicyRefinementVerdict.INVALID, "abstract_policy_not_total")
    if set(concrete_rules) != expected_concrete_keys:
        return PolicyRefinementCertificate(PolicyRefinementVerdict.INVALID, "concrete_policy_not_total")

    for concrete_key in sorted(expected_concrete_keys):
        concrete_rule = concrete_rules[concrete_key]
        abstract_key = (state_map[concrete_rule.state], input_map[concrete_rule.input_symbol])
        abstract_rule = abstract_rules[abstract_key]
        if action_map[concrete_rule.action] != abstract_rule.action:
            return PolicyRefinementCertificate(
                PolicyRefinementVerdict.COUNTEREXAMPLE,
                "policy_action_not_refined",
                concrete_rule,
                abstract_rule,
            )
    return PolicyRefinementCertificate(PolicyRefinementVerdict.REFINED, "all_policy_rules_refine")
