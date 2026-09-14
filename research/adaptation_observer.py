"""Finite observer for real OFR time-series artifacts.

This module extracts bounded metadata and regime observations only. It never
produces trades, transfers, forecasts-as-authority, or personalized advice.
"""
from __future__ import annotations

from dataclasses import dataclass
import gzip
import hashlib
import json
from pathlib import Path
from typing import Any


@dataclass(frozen=True)
class SeriesObservation:
    series_id: str
    points: int
    first_date: str | None
    last_date: str | None
    first_value: float | None
    last_value: float | None
    delta: float | None
    regime: str
    source_digest: str


def _values(node: Any) -> list[tuple[str, float]]:
    if not isinstance(node, dict):
        return []
    nested = node.get("timeseries")
    if not isinstance(nested, dict):
        return []
    aggregation = nested.get("aggregation")
    if not isinstance(aggregation, list):
        return []
    result: list[tuple[str, float]] = []
    for item in aggregation:
        if isinstance(item, list) and len(item) == 2 and isinstance(item[0], str) and isinstance(item[1], (int, float)):
            result.append((item[0], float(item[1])))
    return result


def _regime(delta: float | None) -> str:
    if delta is None:
        return "UNKNOWN"
    if delta > 0:
        return "RISING"
    if delta < 0:
        return "FALLING"
    return "FLAT"


def observe_artifact(path: str | Path, limit: int = 32) -> tuple[SeriesObservation, ...]:
    raw = Path(path).read_bytes()
    digest = hashlib.sha256(raw).hexdigest()
    decoded = gzip.decompress(raw) if raw[:2] == b"\x1f\x8b" else raw
    payload = json.loads(decoded)
    observations: list[SeriesObservation] = []
    if not isinstance(payload, dict):
        return ()
    for series_id, entry in payload.get("timeseries", {}).items():
        points = _values(entry)
        if not points:
            continue
        first_date, first_value = points[0]
        last_date, last_value = points[-1]
        delta = last_value - first_value
        observations.append(SeriesObservation(
            series_id=str(series_id), points=len(points), first_date=first_date,
            last_date=last_date, first_value=first_value, last_value=last_value,
            delta=delta, regime=_regime(delta), source_digest=digest,
        ))
        if len(observations) >= limit:
            break
    return tuple(observations)
