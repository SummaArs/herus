"""Closed finite policy domain for HERUS Skill synthesis.

The domain models symbolic transitions only. Actions are labels, not actuator
calls. The verifier rejects unknown states, signals, duplicate rules and
non-total policies.
"""
from __future__ import annotations

from dataclasses import dataclass
from itertools import permutations
from typing import Iterable

from .skills import Skill, candidate_skill

STATES = ("SAFE", "WAIT", "ALERT")
SIGNALS = ("OK", "TIMEOUT", "CANCEL")
ACTIONS = ("WAIT", "ALERT", "SAFE")
RULE_TOKENS = tuple(f"SIGNAL:{signal}=>{action}" for signal, action in (
    ("OK", "WAIT"),
    ("OK", "ALERT"),
    ("OK", "SAFE"),
    ("TIMEOUT", "WAIT"),
    ("TIMEOUT", "ALERT"),
    ("TIMEOUT", "SAFE"),
    ("CANCEL", "WAIT"),
    ("CANCEL", "ALERT"),
    ("CANCEL", "SAFE"),
))


@dataclass(frozen=True)
class PolicyCase:
    state: str
    signal: str
    expected_action: str


def _decode_rule(token: str) -> tuple[str, str] | None:
    if not token.startswith("SIGNAL:") or "=>" not in token:
        return None
    signal, action = token[7:].split("=>", 1)
    if signal not in SIGNALS or action not in ACTIONS:
        return None
    return signal, action


def run_policy(program: tuple[str, ...], state: str, signal: str) -> tuple[str | None, str]:
    if state not in STATES or signal not in SIGNALS:
        return None, "unknown_context"
    seen: set[str] = set()
    rules: dict[str, str] = {}
    for token in program:
        decoded = _decode_rule(token)
        if decoded is None:
            return None, "invalid_rule"
        current_signal, action = decoded
        if current_signal in seen:
            return None, "duplicate_rule"
        seen.add(current_signal)
        rules[current_signal] = action
    if set(rules) != set(SIGNALS):
        return None, "non_total_policy"
    return rules[signal], "ok"


def verify_policy_skill(skill: Skill, visible: Iterable[PolicyCase], hidden: Iterable[PolicyCase]) -> tuple[bool, tuple[PolicyCase, ...], int]:
    if skill.input_type != "Context" or skill.output_type != "Action" or skill.allowed_effects:
        return False, (), 0
    cases = tuple(visible) + tuple(hidden)
    failures: list[PolicyCase] = []
    for case in cases:
        result, reason = run_policy(skill.program, case.state, case.signal)
        if reason != "ok" or result != case.expected_action:
            failures.append(case)
    return not failures and bool(cases), tuple(failures), len(cases)


def synthesize_policy_skill(*, skill_id: str, visible: tuple[PolicyCase, ...], hidden: tuple[PolicyCase, ...]) -> tuple[Skill | None, dict[str, int]]:
    metrics = {"candidates": 0, "rejected": 0, "verified": 0}
    for program in permutations(RULE_TOKENS, 3):
        metrics["candidates"] += 1
        skill = candidate_skill(
            skill_id=skill_id,
            input_type="Context",
            output_type="Action",
            program=program,
            provenance={"kind": "bounded_policy_enumeration", "grammar": "finite-policy-v1"},
            resource_budget={"steps": 3, "memory": 3},
        )
        passed, _, _ = verify_policy_skill(skill, visible, hidden)
        if passed:
            metrics["verified"] += 1
            return skill, metrics
        metrics["rejected"] += 1
    return None, metrics
