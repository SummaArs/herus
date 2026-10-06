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


def synthesize(task: SynthesisTask) -> SynthesisResult:
    if not task.public_examples or task.max_depth < 1 or task.max_depth > 3:
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
    if len(matches) != 1:
        detail = "no candidate matches public examples" if not matches else "ambiguous public examples"
        return SynthesisResult("ABSTAIN", task.task_id, "", False, None, checked, detail)
    expression, fn = matches[0]
    hidden_pass = all(fn(*example.inputs) == example.output for example in task.hidden_examples)
    return SynthesisResult("PROPOSE" if hidden_pass else "REJECT_HIDDEN", task.task_id, expression, True, hidden_pass, checked, "unique public match")


def _compositions():
    result = []
    unary = (("abs", abs),)
    base = (("x", lambda x, y: x), ("y", lambda x, y: y))
    for name, outer in unary:
        for inner_name, inner in base:
            result.append((f"{name}({inner_name})", lambda x, y, o=outer, i=inner: o(i(x, y))))
    return result
