"""Small, auditable program-synthesis lane for HERUS.

The synthesizer searches a finite expression grammar from public examples and
is evaluated on hidden examples by the caller. It is intentionally narrow:
this is evidence of bounded synthesis, not general programming intelligence.
"""
from __future__ import annotations
from dataclasses import dataclass
from typing import Callable, Tuple

@dataclass(frozen=True)
class IOExample:
    inputs: Tuple[int, int]
    output: int

@dataclass(frozen=True)
class SynthesisTask:
    task_id: str
    public_examples: Tuple[IOExample, ...]
    hidden_examples: Tuple[IOExample, ...]
    max_depth: int = 2

@dataclass(frozen=True)
class SynthesisResult:
    status: str
    task_id: str
    expression: str
    public_pass: bool
    hidden_pass: bool | None
    candidates_checked: int
    detail: str

OPS: Tuple[tuple[str, Callable[[int, int], int]], ...] = (
    ("x + y", lambda x, y: x + y),
    ("x - y", lambda x, y: x - y),
    ("y - x", lambda x, y: y - x),
    ("x * y", lambda x, y: x * y),
    ("max(x, y)", max),
    ("min(x, y)", min),
    ("abs(x)", lambda x, y: abs(x)),
    ("abs(y)", lambda x, y: abs(y)),
)
CANONICAL_PROBES = ((-2, -1), (-2, 1), (-1, 2), (0, 1), (1, 0), (1, 2), (2, -1), (2, 2))


def synthesize(task: SynthesisTask) -> SynthesisResult:
    if not task.public_examples or len({example.inputs for example in task.public_examples}) < 2 or task.max_depth < 1 or task.max_depth > 3:
        return SynthesisResult("ABSTAIN", task.task_id, "", False, None, 0, "invalid or underspecified task")
    candidates = [(name, fn) for name, fn in OPS]
    if task.max_depth >= 2:
        candidates += _compositions()
    checked = 0
    matches = []
    for expression, fn in candidates:
        checked += 1
        try:
            if all(fn(*example.inputs) == example.output for example in task.public_examples):
                matches.append((expression, fn))
        except (ArithmeticError, TypeError, ValueError):
            continue
    if not matches:
        return SynthesisResult("ABSTAIN", task.task_id, "", False, None, checked, "no candidate matches public examples")
    semantic_groups = {}
    for expression, fn in matches:
        vector = tuple(fn(*inputs) for inputs in CANONICAL_PROBES)
        semantic_groups.setdefault(vector, []).append((expression, fn))
    if len(semantic_groups) != 1:
        detail = "ambiguous public examples"
        return SynthesisResult("ABSTAIN", task.task_id, "", False, None, checked, detail)
    expression, fn = min(next(iter(semantic_groups.values())), key=lambda item: (len(item[0]), item[0]))
    hidden_pass = all(fn(*example.inputs) == example.output for example in task.hidden_examples)
    return SynthesisResult("PROPOSE" if hidden_pass else "REJECT_HIDDEN", task.task_id, expression, True, hidden_pass, checked, "unique public match")


def _compositions():
    result = []
    base = (("x", lambda x, y: x), ("y", lambda x, y: y))
    unary = (("abs", abs),)
    for name, outer in unary:
        for inner_name, inner in base:
            result.append((f"{name}({inner_name})", lambda x, y, o=outer, i=inner: o(i(x, y))))
    binary = (
        ("+", lambda left, right: left + right),
        ("-", lambda left, right: left - right),
        ("*", lambda left, right: left * right),
    )
    atoms = list(base) + list(OPS[:4])
    for operator, combine in binary:
        for left_name, left in atoms:
            for right_name, right in atoms:
                expression = f"({left_name}) {operator} ({right_name})"
                result.append((expression, lambda x, y, l=left, r=right, c=combine: c(l(x, y), r(x, y))))
    return result
