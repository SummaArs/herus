"""Bounded arithmetic Skill synthesis for the first HERUS experiment."""
from __future__ import annotations

from itertools import product
from typing import Iterable

from .skills import Skill, candidate_skill
from .skill_verifier import ArithmeticCase, verify_arithmetic_skill


TOKENS = ("INPUT", "CONST:-2", "CONST:-1", "CONST:0", "CONST:1", "CONST:2", "CONST:3", "ADD", "MUL", "NEG")


def synthesize_arithmetic_skill(
    *,
    skill_id: str,
    visible: Iterable[ArithmeticCase],
    hidden: Iterable[ArithmeticCase],
    max_program_length: int = 7,
) -> tuple[Skill | None, dict[str, int]]:
    """Enumerate bounded programs and return the first independently verified Skill."""
    visible_cases, hidden_cases = tuple(visible), tuple(hidden)
    metrics = {"candidates": 0, "rejected": 0, "verified": 0, "max_program_length": max_program_length}
    for length in range(1, max_program_length + 1):
        for program in product(TOKENS, repeat=length):
            if program.count("INPUT") != 1:
                continue
            metrics["candidates"] += 1
            candidate = candidate_skill(
                skill_id=skill_id,
                input_type="Int",
                output_type="Int",
                program=program,
                provenance={"kind": "bounded_enumeration", "grammar": "arithmetic-rpn-v1", "length": length},
                resource_budget={"steps": length, "memory": length + 1},
            )
            result = verify_arithmetic_skill(candidate, visible_cases, hidden_cases)
            if result.passed:
                metrics["verified"] += 1
                return candidate, metrics
            metrics["rejected"] += 1
    return None, metrics
