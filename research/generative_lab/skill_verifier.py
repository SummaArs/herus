"""Independent verifier for the first closed Skill domain.

The verifier interprets a tiny RPN arithmetic DSL. It does not import, compile,
exec or evaluate Python code. A candidate is only promoted to VERIFIED when it
passes visible and hidden examples within its declared step budget.
"""
from __future__ import annotations

from dataclasses import dataclass
from typing import Iterable

from .skills import Skill, SkillState


@dataclass(frozen=True)
class ArithmeticCase:
    input_value: int
    expected: int


@dataclass(frozen=True)
class VerificationResult:
    passed: bool
    state: SkillState
    reason: str
    visible_passed: int
    hidden_passed: int
    steps: int
    counterexamples: tuple[ArithmeticCase, ...]
    evidence_hash: str


def _run_program(program: tuple[str, ...], value: int, max_steps: int) -> tuple[int | None, str, int]:
    stack: list[int] = []
    steps = 0
    for token in program:
        steps += 1
        if steps > max_steps:
            return None, "step_budget_exceeded", steps
        if token == "INPUT":
            stack.append(value)
        elif token.startswith("CONST:"):
            try:
                stack.append(int(token[6:]))
            except ValueError:
                return None, "invalid_constant", steps
        elif token == "ADD":
            if len(stack) < 2:
                return None, "stack_underflow", steps
            stack.append(stack.pop() + stack.pop())
        elif token == "MUL":
            if len(stack) < 2:
                return None, "stack_underflow", steps
            right, left = stack.pop(), stack.pop()
            stack.append(left * right)
        elif token == "NEG":
            if not stack:
                return None, "stack_underflow", steps
            stack.append(-stack.pop())
        else:
            return None, "unknown_opcode", steps
    if len(stack) != 1:
        return None, "non_single_result", steps
    return stack[0], "ok", steps


def verify_arithmetic_skill(skill: Skill, visible: Iterable[ArithmeticCase], hidden: Iterable[ArithmeticCase]) -> VerificationResult:
    if skill.input_type != "Int" or skill.output_type != "Int":
        return VerificationResult(False, SkillState.QUARANTINED, "type_mismatch", 0, 0, 0, (), "")
    if skill.allowed_effects or "EXECUTE_ACTUATOR" in skill.forbidden_effects and skill.allowed_effects:
        return VerificationResult(False, SkillState.QUARANTINED, "effect_policy_violation", 0, 0, 0, (), "")
    budget = skill.resource_budget.get("steps", 0)
    visible_cases, hidden_cases = tuple(visible), tuple(hidden)
    visible_passed = 0
    hidden_passed = 0
    steps = 0
    counterexamples: list[ArithmeticCase] = []
    for case in visible_cases + hidden_cases:
        result, reason, used = _run_program(skill.program, case.input_value, budget)
        steps += used
        if reason == "ok" and result == case.expected:
            if case in visible_cases:
                visible_passed += 1
            else:
                hidden_passed += 1
        else:
            counterexamples.append(case)
    total = len(visible_cases) + len(hidden_cases)
    passed = total > 0 and len(counterexamples) == 0
    evidence = f"{skill.content_hash()}:{visible_passed}/{len(visible_cases)}:{hidden_passed}/{len(hidden_cases)}" if passed else ""
    return VerificationResult(
        passed=passed,
        state=SkillState.VERIFIED if passed else SkillState.TESTED,
        reason="verified" if passed else "counterexample_or_invalid_program",
        visible_passed=visible_passed,
        hidden_passed=hidden_passed,
        steps=steps,
        counterexamples=tuple(counterexamples),
        evidence_hash=evidence,
    )
