"""Reproducible benchmark for finite critical-state synthesis."""
from __future__ import annotations

from dataclasses import dataclass

from critical_state_synthesis import synthesize
from critical_state_verifier import StateMachineSpec, Transition, Verdict


@dataclass(frozen=True)
class BenchmarkCase:
    name: str
    spec: StateMachineSpec
    expected: Verdict


def cases() -> tuple[BenchmarkCase, ...]:
    valid = StateMachineSpec(
        initial_state="nominal",
        states=frozenset({"nominal", "safe_hold"}),
        inputs=frozenset({"ok", "loss"}),
        actions=frozenset({"hold", "safe_hold"}),
        transitions=(
            Transition("nominal", "ok", "hold", "nominal"),
            Transition("nominal", "loss", "safe_hold", "safe_hold"),
            Transition("safe_hold", "ok", "safe_hold", "safe_hold"),
            Transition("safe_hold", "loss", "safe_hold", "safe_hold"),
        ),
        forbidden_states=frozenset(),
        forbidden_actions=frozenset(),
        max_steps=3,
    )
    incomplete = StateMachineSpec(
        initial_state="nominal",
        states=frozenset({"nominal", "safe_hold"}),
        inputs=frozenset({"ok", "loss"}),
        actions=frozenset({"hold", "safe_hold"}),
        transitions=(Transition("nominal", "ok", "hold", "nominal"),),
        forbidden_states=frozenset(),
        forbidden_actions=frozenset(),
        max_steps=2,
    )
    return (
        BenchmarkCase("valid_finite_policy", valid, Verdict.VERIFIED),
        BenchmarkCase("incomplete_model", incomplete, Verdict.UNKNOWN),
    )


def run(max_candidates: int = 256) -> tuple[dict[str, object], ...]:
    results: list[dict[str, object]] = []
    for case in cases():
        result = synthesize(case.spec, max_candidates=max_candidates)
        results.append(
            {
                "case": case.name,
                "expected": case.expected.value,
                "observed": result.certificate.verdict.value,
                "candidates": result.candidates,
                "verified": result.policy is not None,
                "reason": result.certificate.reason,
            }
        )
    return tuple(results)


if __name__ == "__main__":
    for row in run():
        print(row)
