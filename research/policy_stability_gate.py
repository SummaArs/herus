"""Fail-closed stability gate for universal policy promotion."""
from __future__ import annotations

from dataclasses import dataclass
from typing import Iterable, Mapping


@dataclass(frozen=True)
class StabilityDecision:
    status: str
    policy: str
    reason: str
    hosts: int
    passing_hosts: int
    minimum_margin: float
    deltas: tuple[tuple[str, float], ...]


def evaluate(records: Iterable[Mapping[str, object]], *, alternative: str = "score_calibrated", default: str = "universal_default", margin: float = 0.01, min_hosts: int = 3) -> StabilityDecision:
    if margin < 0 or min_hosts < 1:
        raise ValueError("invalid_stability_contract")
    rows = list(records)
    if len(rows) < min_hosts:
        return StabilityDecision("BLOCKED", default, "insufficient_independent_hosts", len(rows), 0, margin, ())
    deltas: list[tuple[str, float]] = []
    passing = 0
    for row in rows:
        host = str(row.get("dataset", row.get("host", "unknown")))
        metrics = row.get("validation_accuracy")
        if not isinstance(metrics, Mapping):
            selection = row.get("selection")
            metrics = selection.get("validation_accuracy") if isinstance(selection, Mapping) else None
        if not isinstance(metrics, Mapping) or alternative not in metrics or default not in metrics:
            return StabilityDecision("BLOCKED", default, "missing_disjoint_validation_metrics", len(rows), passing, margin, tuple(deltas))
        delta = float(metrics[alternative]) - float(metrics[default])
        deltas.append((host, round(delta, 6)))
        if delta >= margin:
            passing += 1
    if passing == len(rows):
        return StabilityDecision("PROMOTE", alternative, "all_hosts_exceed_practical_margin", len(rows), passing, margin, tuple(deltas))
    return StabilityDecision("ABSTAIN", default, "not_all_hosts_exceed_practical_margin", len(rows), passing, margin, tuple(deltas))
