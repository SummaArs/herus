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

@dataclass(frozen=True)
class TestFailure:
    test_id: str
    message: str
    expected: str = ""
    observed: str = ""

@dataclass(frozen=True)
class RepairProposal:
    status: str
    hypotheses: Tuple[str, ...]
    plan: Tuple[str, ...]
    requested_evidence: Tuple[str, ...]
    patch: str = ""
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

    def diagnose_failures(self, failures: Tuple[TestFailure, ...]) -> RepairProposal:
        """Turn observed failures into a bounded repair proposal.

        The method never invents a passing result and never emits an
        executable patch without enough evidence. A caller must provide the
        affected code and rerun independent verification outside this module.
        """
        if not failures or any(not item.test_id.strip() or not item.message.strip() for item in failures):
            return RepairProposal("ABSTAIN", (), (), ("Provide a non-empty test id and observed failure message.",))
        hypotheses = tuple(self._failure_hypothesis(item) for item in failures)
        requested = (
            "provide the smallest affected function or interface",
            "reproduce the failure with a fixed input",
            "add a regression case before changing implementation",
            "rerun positive, negative, malformed, and boundary cases",
        )
        return RepairProposal(
            "REPAIR_PROPOSAL",
            hypotheses,
            ("preserve the failing observation", "test the narrowest hypothesis", "propose the smallest reversible change", "rebuild the evidence ledger"),
            requested,
        )

    @staticmethod
    def _failure_hypothesis(failure: TestFailure) -> str:
        message = failure.message.casefold()
        if "timeout" in message or "deadline" in message:
            return f"{failure.test_id}: possible deadline or lifecycle violation"
        if "parse" in message or "syntax" in message:
            return f"{failure.test_id}: possible contract/parser mismatch"
        if failure.expected and failure.observed and failure.expected != failure.observed:
            return f"{failure.test_id}: observed value differs from expected value"
        return f"{failure.test_id}: cause unknown; do not infer implementation fault yet"

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
