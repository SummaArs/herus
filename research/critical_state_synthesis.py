"""Bounded policy synthesis driven by the independent critical-state verifier."""
from __future__ import annotations

from dataclasses import dataclass
from itertools import product

from critical_state_verifier import (
    Certificate,
    PolicyRule,
    StateMachineSpec,
    Verdict,
    verify,
)


@dataclass(frozen=True)
class SynthesisResult:
    policy: tuple[PolicyRule, ...] | None
    certificate: Certificate
    candidates: int
    exhausted: bool


def synthesize(spec: StateMachineSpec, *, max_candidates: int) -> SynthesisResult:
    """Enumerate complete policies; return only independently verified policies."""
    if max_candidates <= 0:
        return SynthesisResult(None, Certificate(Verdict.UNKNOWN, 0, (), "invalid_candidate_budget"), 0, False)
    keys = tuple(sorted((state, input_symbol) for state in spec.states for input_symbol in spec.inputs))
    actions = tuple(sorted(spec.actions))
    if not keys or not actions:
        return SynthesisResult(None, Certificate(Verdict.INVALID_SPEC, 0, (), "empty_policy_domain"), 0, True)

    candidates = 0
    for selected_actions in product(actions, repeat=len(keys)):
        if candidates >= max_candidates:
            return SynthesisResult(None, Certificate(Verdict.UNKNOWN, 0, (), "candidate_budget_exhausted"), candidates, True)
        candidates += 1
        policy = tuple(
            PolicyRule(state, input_symbol, action)
            for (state, input_symbol), action in zip(keys, selected_actions)
        )
        certificate = verify(spec, policy)
        if certificate.verdict == Verdict.VERIFIED:
            return SynthesisResult(policy, certificate, candidates, False)
        if certificate.verdict == Verdict.INVALID_SPEC:
            return SynthesisResult(None, certificate, candidates, False)
    return SynthesisResult(None, Certificate(Verdict.UNKNOWN, 0, (), "no_verified_policy"), candidates, True)
