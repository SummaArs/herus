"""Typed composition for finite policy Skills."""
from __future__ import annotations

import json

from dataclasses import replace

from .policy_domain import ACTIONS, SIGNALS, STATES, PolicyCase, run_policy
from .skills import Skill, SkillState



def _parse_action_rule(token: str) -> tuple[str, str] | None:
    if not token.startswith("ACTION:") or "=>" not in token:
        return None
    action, decision = token[7:].split("=>", 1)
    if action not in ACTIONS or decision not in ACTIONS:
        return None
    return action, decision


def run_action_policy(program: tuple[str, ...], action: str) -> tuple[str | None, str]:
    if action not in ACTIONS:
        return None, "unknown_action"
    table: dict[str, str] = {}
    for token in program:
        parsed = _parse_action_rule(token)
        if parsed is None:
            return None, "invalid_action_rule"
        source, target = parsed
        if source in table:
            return None, "duplicate_action_rule"
        table[source] = target
    if set(table) != set(ACTIONS):
        return None, "non_total_action_policy"
    return table[action], "ok"


def compose_policy_skills(first: Skill, second: Skill, *, skill_id: str) -> Skill:
    if first.state != SkillState.VERIFIED or second.state != SkillState.VERIFIED:
        raise ValueError("both dependencies must be VERIFIED")
    if first.allowed_effects or second.allowed_effects:
        raise ValueError("effectful dependencies cannot be composed")
    if first.input_type != "Context" or first.output_type != "Action":
        raise TypeError("first Skill must be Context -> Action")
    if second.input_type != "Action" or second.output_type != "Decision":
        raise TypeError("second Skill must be Action -> Decision")
    final_rules: list[str] = []
    for signal in SIGNALS:
        first_result = None
        for state in STATES:
            result, reason = run_policy(first.program, state, signal)
            if reason != "ok":
                raise ValueError(f"first Skill is not total: {reason}")
            if first_result is None:
                first_result = result
            elif first_result != result:
                raise ValueError("first Skill depends on state; composition requires signal-only stage")
        decision, reason = run_action_policy(second.program, first_result or "")
        if reason != "ok" or decision is None:
            raise ValueError(f"second Skill is not total: {reason}")
        final_rules.append(f"SIGNAL:{signal}=>{decision}")
    return Skill(
        skill_id=skill_id,
        version=1,
        input_type="Context",
        output_type="Decision",
        preconditions=first.preconditions + second.preconditions,
        postconditions=first.postconditions + second.postconditions,
        program=tuple(final_rules),
        dependencies=(f"{first.skill_id}@{first.version}", f"{second.skill_id}@{second.version}"),
        resource_budget={"steps": len(final_rules), "memory": len(final_rules)},
        allowed_effects=(),
        forbidden_effects=("EXECUTE_ACTUATOR", "GRANT_AUTHORITY", "ALTER_VERIFIER"),
        determinism="deterministic",
        provenance={"kind": "typed_policy_composition", "parents": [first.content_hash(), second.content_hash()]},
        state=SkillState.CANDIDATE,
        evidence_hash="",
        generalization_score=0.0,
        reliability=0.0,
        usage_count=0,
    )


def run() -> dict[str, object]:
    first = Skill(
        skill_id="signal-policy",
        version=1,
        input_type="Context",
        output_type="Action",
        preconditions=(),
        postconditions=(),
        program=("SIGNAL:OK=>WAIT", "SIGNAL:TIMEOUT=>ALERT", "SIGNAL:CANCEL=>SAFE"),
        dependencies=(),
        resource_budget={"steps": 3, "memory": 3},
        allowed_effects=(),
        forbidden_effects=("EXECUTE_ACTUATOR",),
        determinism="deterministic",
        provenance={"kind": "fixture"},
        state=SkillState.VERIFIED,
        evidence_hash="first-evidence",
        generalization_score=1.0,
        reliability=1.0,
        usage_count=0,
    )
    second = replace(first, skill_id="action-policy", input_type="Action", output_type="Decision", program=("ACTION:WAIT=>WAIT", "ACTION:ALERT=>ALERT", "ACTION:SAFE=>SAFE"), evidence_hash="second-evidence")
    composed = compose_policy_skills(first, second, skill_id="composed-policy")
    cases = (PolicyCase("SAFE", "OK", "WAIT"), PolicyCase("WAIT", "TIMEOUT", "ALERT"), PolicyCase("ALERT", "CANCEL", "SAFE"))
    passed, failures = verify_composed_policy(composed, cases)
    return {"schema": "herus.generative_lab.policy_composition", "version": 1, "authority": "none", "state": composed.state.value, "dependencies": list(composed.dependencies), "passed": passed, "failure_count": len(failures), "allowed_effects": list(composed.allowed_effects)}


def verify_composed_policy(skill: Skill, cases: tuple[PolicyCase, ...]) -> tuple[bool, tuple[PolicyCase, ...]]:
    if skill.input_type != "Context" or skill.output_type != "Decision":
        return False, cases
    failures: list[PolicyCase] = []
    for case in cases:
        result, reason = run_policy(skill.program, case.state, case.signal)
        if reason != "ok" or result != case.expected_action:
            failures.append(case)
    return not failures and bool(cases), tuple(failures)


if __name__ == "__main__":
    print(json.dumps(run(), indent=2, sort_keys=True))
