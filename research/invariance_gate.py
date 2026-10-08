"""Decision-invariance gate: abstain when benign views disagree."""
from __future__ import annotations
from dataclasses import dataclass
import re


def views(text: str) -> list[str]:
    compact = re.sub(r"[^A-Za-z0-9' ]+", " ", text).lower()
    prefix_removed = re.sub(r"^[^:,.!?]{0,80}[:,.!?]\s*", "", text, count=1).strip()
    return [text, compact, prefix_removed]


@dataclass(frozen=True)
class InvarianceFit:
    max_disagreements: int
    calibration_examples: int


class DecisionInvarianceGate:
    def __init__(self, predict, *, max_calibration_disagreements: int = 0):
        self.predict = predict
        self.max_calibration_disagreements = max_calibration_disagreements
        self.fit_state: InvarianceFit | None = None

    def _score(self, text: str) -> int:
        values = [self.predict(item) for item in views(text)]
        return sum(value != values[0] for value in values[1:])

    def fit(self, texts: list[str]) -> InvarianceFit:
        if not texts:
            raise ValueError("empty_calibration")
        scores = [self._score(text) for text in texts]
        allowed = max(scores) if self.max_calibration_disagreements else 0
        self.fit_state = InvarianceFit(allowed, len(texts))
        return self.fit_state

    def score(self, text: str) -> int:
        if self.fit_state is None:
            raise RuntimeError("gate_not_fitted")
        return self._score(text)

    def accept(self, text: str) -> bool:
        return self.score(text) <= self.fit_state.max_disagreements
