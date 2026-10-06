"""Proposal-only programming skill for HERUS.

This module plans code changes and emits reviewable artifacts. It never writes
files, executes code, opens a shell, or grants authority.
"""
from __future__ import annotations
from dataclasses import dataclass
from typing import Tuple
import ast
import re

SUPPORTED = {"python", "c11", "javascript"}

@dataclass(frozen=True)
class ProgrammingRequest:
    goal: str
    language: str = "python"
    constraints: Tuple[str, ...] = ()
    files: Tuple[str, ...] = ()

@dataclass(frozen=True)
class ProgrammingQuestion:
    kind: str
    prompt: str
    blocking: bool

@dataclass(frozen=True)
class ProgrammingProposal:
    status: str
    language: str
    plan: Tuple[str, ...]
    questions: Tuple[ProgrammingQuestion, ...]
    source: str
    tests: Tuple[str, ...]
    assumptions: Tuple[str, ...]
    authority: str = "none"

class ProgrammingSkill:
    """Finite, deterministic planner for small programming proposals."""

    def propose(self, request: ProgrammingRequest) -> ProgrammingProposal:
        language = request.language.lower()
        if language not in SUPPORTED:
            return ProgrammingProposal(
                status="ABSTAIN",
                language=language,
                plan=(),
                questions=(ProgrammingQuestion("language", f"Unsupported language: {language}", True),),
                source="",
                tests=(),
                assumptions=(),
            )
        goal = request.goal.strip()
        if not goal:
            return ProgrammingProposal(
                status="ABSTAIN",
                language=language,
                plan=(),
                questions=(ProgrammingQuestion("scope", "What behavior must be changed?", True),),
                source="",
                tests=(),
                assumptions=(),
            )
        questions = self._questions(goal, request.constraints)
        plan = (
            "extract the observable contract",
            "identify affected files and interfaces",
            "write adversarial tests before implementation",
            "produce a minimal implementation proposal",
            "review syntax, authority boundaries, and rollback conditions",
        )
        source = self._skeleton(goal, language)
        tests = self._tests(goal, language)
        assumptions = (
            "the proposal is not an execution authorization",
            "existing project conventions must be inspected before applying it",
            "generated code requires an independent test and human review",
        )
        status = "PROPOSE" if not any(q.blocking for q in questions) else "PROPOSE_WITH_QUESTIONS"
        return ProgrammingProposal(status, language, plan, questions, source, tests, assumptions)

    @staticmethod
    def _questions(goal: str, constraints: Tuple[str, ...]) -> Tuple[ProgrammingQuestion, ...]:
        qs = []
        if not constraints:
            qs.append(ProgrammingQuestion("constraint", "Which runtime, dependency, and resource constraints apply?", False))
        if re.search(r"\b(delete|remove|migrate|publish|deploy|execute|run)\b", goal, re.I):
            qs.append(ProgrammingQuestion("authority", "This request may have external effects; specify an explicit review gate.", True))
        return tuple(qs)

    @staticmethod
    def _skeleton(goal: str, language: str) -> str:
        if language == "python":
            return f'''"""Proposal for: {goal}"""\n\ndef implement(request):\n    """Return a value; do not perform external effects."""\n    raise NotImplementedError("proposal-only skeleton")\n'''
        if language == "c11":
            return f'''/* Proposal for: {goal} */\n#include <stddef.h>\n\nint herus_proposal(const void *request, size_t length) {{\n    (void)request; (void)length;\n    return -1; /* proposal only; no external effect */\n}}\n'''
        return f'''// Proposal for: {goal}\nexport function implement(request) {{\n  void request;\n  throw new Error("proposal-only skeleton");\n}}\n'''

    @staticmethod
    def _tests(goal: str, language: str) -> Tuple[str, ...]:
        base = ("empty input abstains", "malformed input fails closed", "proposal has no authority", "repeated proposal is deterministic")
        if language == "python":
            source = ProgrammingSkill._skeleton(goal, language)
            try:
                ast.parse(source)
            except SyntaxError:
                return base + ("generated Python must parse",)
        return base + (f"generated {language} syntax must be checked independently",)
