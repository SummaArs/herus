"""Jev-inspired typed decision layer for HERUS.

This is a local contract and harness, not a reimplementation of TypeSafe's
closed Jev model. It returns typed probabilities and abstains conservatively.
"""
from __future__ import annotations
from concurrent.futures import ThreadPoolExecutor
from dataclasses import dataclass
from enum import Enum
from typing import Any, Callable, Mapping, Sequence

class QuestionKind(str, Enum):
    NOUL = "noul"
    CHOICE = "choice"
    SCORE = "score"

@dataclass(frozen=True)
class Question:
    name: str
    kind: QuestionKind
    instructions: str
    options: tuple[str, ...] = ()
    minimum: int = 0
    maximum: int = 100
    def __post_init__(self) -> None:
        if not self.name or not self.instructions: raise ValueError("question_metadata_missing")
        if self.kind is QuestionKind.CHOICE and len(self.options) < 2: raise ValueError("choice_options_invalid")
        if self.kind is QuestionKind.NOUL and self.options: raise ValueError("noul_options_forbidden")
        if self.kind is QuestionKind.SCORE and self.minimum >= self.maximum: raise ValueError("score_range_invalid")

@dataclass(frozen=True)
class TypedAnswer:
    name: str
    kind: QuestionKind
    probabilities: tuple[tuple[str, int], ...]
    value: str | int
    confidence_milli: int
    abstained: bool
    reason: str

@dataclass(frozen=True)
class DecisionBatch:
    answers: tuple[TypedAnswer, ...]
    authority: str = "NONE"
    @property
    def executable(self) -> bool: return False

Oracle = Callable[[Mapping[str, Any], Question], Mapping[str, float] | float | bool]

def _normalise(values: Mapping[str, float]) -> tuple[tuple[str, int], ...]:
    if not values or any(v < 0 for v in values.values()): raise ValueError("probability_domain_invalid")
    total = sum(float(v) for v in values.values())
    if total <= 0: raise ValueError("probability_mass_invalid")
    scaled = [(k, int(round(float(v) / total * 1000))) for k, v in values.items()]
    key, _ = max(scaled, key=lambda x: x[1])
    delta = 1000 - sum(v for _, v in scaled)
    return tuple(sorted((k, v + delta if k == key else v) for k, v in scaled))

def _answer(state: Mapping[str, Any], q: Question, oracle: Oracle, threshold: int) -> TypedAnswer:
    raw = oracle(state, q)
    if raw is None:
        return TypedAnswer(q.name, q.kind, (("unknown", 1000),), "unknown", 0, True, "evidence_missing")
    if q.kind is QuestionKind.NOUL:
        p = float(raw) if isinstance(raw, (bool, int, float)) else float(raw.get("true", 0.0))
        p = max(0.0, min(1.0, p)); probs = _normalise({"false": 1-p, "true": p}); value = "true" if p >= .5 else "false"
    elif q.kind is QuestionKind.CHOICE:
        if not isinstance(raw, Mapping): raise ValueError("choice_oracle_shape_invalid")
        probs = _normalise({o: float(raw.get(o, 0.0)) for o in q.options}); value = max(probs, key=lambda x: x[1])[0]
    else:
        score = float(raw.get("score", 0.0)) if isinstance(raw, Mapping) else float(raw)
        score = max(q.minimum, min(q.maximum, score)); pos = (score-q.minimum)/(q.maximum-q.minimum)
        probs = _normalise({"low": 1-pos, "high": pos}); value = int(round(score))
    peak = max(v for _, v in probs); confidence = int(peak * .9) if peak >= threshold else int(peak * .75)
    abstain = confidence < threshold
    return TypedAnswer(q.name, q.kind, probs, value, confidence, abstain, "confidence_below_gate" if abstain else "typed_decision_ready")

class SystemOneEngine:
    """Parallel typed-decision harness for local or future model adapters."""
    def __init__(self, oracle: Oracle, *, max_workers: int = 8, min_confidence_milli: int = 700):
        if not 1 <= max_workers <= 16 or not 0 <= min_confidence_milli <= 1000: raise ValueError("system_one_policy_invalid")
        self.oracle, self.max_workers, self.min_confidence_milli = oracle, max_workers, min_confidence_milli
    def decide(self, state: Mapping[str, Any], questions: Sequence[Question]) -> DecisionBatch:
        if not isinstance(state, Mapping) or not questions or len(questions) > 64: raise ValueError("decision_request_invalid")
        if len({q.name for q in questions}) != len(questions): raise ValueError("question_names_not_unique")
        with ThreadPoolExecutor(max_workers=min(self.max_workers, len(questions))) as pool:
            answers = tuple(pool.submit(_answer, state, q, self.oracle, self.min_confidence_milli).result() for q in questions)
        return DecisionBatch(answers)

class FiniteStateOracle:
    """Inspectable H0/H1 oracle: consumes only finite evidence from state."""
    def __call__(self, state: Mapping[str, Any], q: Question) -> Mapping[str, float] | float:
        evidence = state.get("evidence", {})
        if not isinstance(evidence, Mapping): raise ValueError("evidence_invalid")
        value = evidence.get(q.name)
        if value is None: return None
        if q.kind is QuestionKind.CHOICE:
            if not isinstance(value, Mapping): raise ValueError("choice_evidence_invalid")
            return {o: float(value.get(o, 0.0)) for o in q.options}
        return value
