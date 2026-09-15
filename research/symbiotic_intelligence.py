"""Symbiotic Intelligence Model v1: tiny neural-symbolic local controller.

The neural part classifies a bounded feature vector. The symbolic part maps only
finite labels to typed events. The symbiotic part selects a representation that
fits the current host profile. No part grants execution authority.
"""
from __future__ import annotations

from dataclasses import dataclass
from typing import Iterable

from host_profile import HostProfile

LABELS = ("ARRIVE", "HELP", "CANCEL", "UNKNOWN")


@dataclass(frozen=True)
class Pattern:
    features: tuple[int, ...]
    source: str
    digest: str


@dataclass(frozen=True)
class NeuralScores:
    scores: tuple[int, ...]
    label: str
    confidence_milli: int


@dataclass(frozen=True)
class SIMDecision:
    label: str
    representation: str
    neural_confidence_milli: int
    proposal: str
    execution: str
    reason: str


class MicroMLP:
    """A deterministic fixed-point 4->3->4 network for local experiments."""

    # Quantized weights. This is an inspectable model, not a hidden service.
    hidden_weights = ((3, 2, -2, -1), (-2, -1, 3, 2), (1, -3, -1, 3))
    output_weights = ((3, -2, 1), (-1, 3, -2), (-2, 1, 3), (0, 0, 0))

    def infer(self, pattern: Pattern) -> NeuralScores:
        if len(pattern.features) != 4:
            raise ValueError("feature_shape_invalid")
        hidden = tuple(max(0, sum(a * b for a, b in zip(row, pattern.features))) for row in self.hidden_weights)
        scores = tuple(sum(a * b for a, b in zip(row, hidden)) for row in self.output_weights)
        winner = max(range(len(scores)), key=scores.__getitem__)
        ordered = sorted(scores, reverse=True)
        margin = ordered[0] - ordered[1]
        confidence = max(0, min(1000, 500 + margin * 25))
        return NeuralScores(scores, LABELS[winner], confidence)


def _representation(profile: HostProfile, required_bytes: int, required_steps: int) -> str | None:
    candidates = (
        ("SIM-INT8", 4096, 12),
        ("SIM-HDC8", 2048, 8),
        ("SIM-RULES", 512, 4),
    )
    available = profile.skill_budget.get("bytes", 0)
    available_steps = profile.skill_budget.get("steps", 0)
    for name, bytes_needed, steps_needed in candidates:
        if name in profile.representation_set and bytes_needed <= available and steps_needed <= available_steps and bytes_needed <= required_bytes and steps_needed <= required_steps:
            return name
    return None


def decide(profile: HostProfile, pattern: Pattern, *, required_bytes: int = 4096, required_steps: int = 12, min_confidence_milli: int = 700) -> SIMDecision:
    """Produce a bounded proposal; execution is structurally impossible here."""
    if profile.authority != "NONE":
        return SIMDecision("UNKNOWN", "", 0, "ABSTAIN", "ABSTAIN", "host_authority_not_discoverable")
    representation = _representation(profile, required_bytes, required_steps)
    if representation is None:
        return SIMDecision("UNKNOWN", "", 0, "ABSTAIN", "ABSTAIN", "no_representation_fits_budget")
    result = MicroMLP().infer(pattern)
    if result.confidence_milli < min_confidence_milli or result.label == "UNKNOWN":
        return SIMDecision(result.label, representation, result.confidence_milli, "ABSTAIN", "ABSTAIN", "neural_confidence_below_symbolic_gate")
    if not pattern.source or not pattern.digest:
        return SIMDecision("UNKNOWN", representation, result.confidence_milli, "ABSTAIN", "ABSTAIN", "pattern_provenance_missing")
    return SIMDecision(result.label, representation, result.confidence_milli, "PROPOSE", "ABSTAIN", "neural_pattern_passed_finite_symbolic_gate")


def train_local_delta(patterns: Iterable[tuple[Pattern, str]]) -> dict[str, int]:
    """Return bounded observation statistics; it never mutates the model."""
    counts = {label: 0 for label in LABELS}
    for pattern, label in patterns:
        if label not in counts or len(pattern.features) != 4:
            raise ValueError("training_label_or_shape_invalid")
        counts[label] += 1
    return counts
