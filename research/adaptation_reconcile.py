"""Fail-closed reconciliation for distributed host observations."""
from __future__ import annotations

from dataclasses import dataclass
from typing import Iterable


@dataclass(frozen=True)
class Observation:
    source_id: str
    sequence: int
    host_digest: str
    key: str
    value: float
    evidence_digest: str


@dataclass(frozen=True)
class Reconciliation:
    key: str
    status: str
    value: float | None
    accepted_sources: tuple[str, ...]
    reason: str


def reconcile(observations: Iterable[Observation], minimum_sources: int = 2) -> Reconciliation:
    rows = list(observations)
    if not rows:
        return Reconciliation("", "BLOCKED", None, (), "no_observations")
    keys = {row.key for row in rows}
    hosts = {row.host_digest for row in rows}
    if len(keys) != 1 or len(hosts) != 1:
        return Reconciliation(rows[0].key, "BLOCKED", None, (), "scope_conflict")
    unique: dict[str, Observation] = {}
    for row in rows:
        previous = unique.get(row.source_id)
        if previous is not None and row.sequence <= previous.sequence:
            return Reconciliation(row.key, "BLOCKED", None, (), "replay_or_nonmonotonic_source")
        unique[row.source_id] = row
    values = {row.value for row in unique.values()}
    if len(values) != 1:
        return Reconciliation(rows[0].key, "BLOCKED", None, (), "value_conflict")
    if len(unique) < minimum_sources:
        return Reconciliation(rows[0].key, "BLOCKED", None, tuple(sorted(unique)), "quorum_missing")
    digests = {row.evidence_digest for row in unique.values()}
    if len(digests) != 1:
        return Reconciliation(rows[0].key, "BLOCKED", None, (), "evidence_conflict")
    return Reconciliation(rows[0].key, "ACCEPTED", rows[0].value, tuple(sorted(unique)), "quorum_consistent")
