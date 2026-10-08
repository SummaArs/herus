"""Small, deterministic lexical shift detector calibrated on clean data only."""
from __future__ import annotations
import math
import re
from dataclasses import dataclass
from statistics import quantiles

_WORD = re.compile(r"[A-Za-z']+")


def tokens(text: str) -> list[str]:
    return _WORD.findall(text.lower())


@dataclass(frozen=True)
class ShiftFit:
    threshold: float
    calibration_examples: int
    quantile: float


class LexicalShiftDetector:
    def __init__(self, *, quantile: float = 0.99):
        if not 0.5 < quantile < 1.0:
            raise ValueError("quantile_out_of_range")
        self.quantile = quantile
        self.vocabulary: frozenset[str] = frozenset()
        self.mean_length = 0.0
        self.std_length = 1.0
        self.fit_state: ShiftFit | None = None

    def _score(self, text: str) -> float:
        words = tokens(text)
        if not words:
            return 1.0
        unknown = sum(word not in self.vocabulary for word in words) / len(words)
        z_length = max(0.0, (len(words) - self.mean_length) / self.std_length)
        return unknown + 0.05 * min(z_length, 20.0)

    def fit(self, texts: list[str]) -> ShiftFit:
        if not texts:
            raise ValueError("empty_calibration")
        vocab = {word for text in texts for word in tokens(text)}
        lengths = [len(tokens(text)) for text in texts]
        self.vocabulary = frozenset(vocab)
        self.mean_length = sum(lengths) / len(lengths)
        variance = sum((x - self.mean_length) ** 2 for x in lengths) / len(lengths)
        self.std_length = max(math.sqrt(variance), 1.0)
        scores = sorted(self._score(text) for text in texts)
        index = min(len(scores) - 1, max(0, math.ceil(self.quantile * len(scores)) - 1))
        self.fit_state = ShiftFit(scores[index], len(texts), self.quantile)
        return self.fit_state

    def score(self, text: str) -> float:
        if self.fit_state is None:
            raise RuntimeError("detector_not_fitted")
        return self._score(text)

    def accept(self, text: str) -> bool:
        return self.score(text) <= self.fit_state.threshold
