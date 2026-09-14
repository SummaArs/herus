"""Audit log for host experiments selected by the adaptive core."""
from __future__ import annotations

from dataclasses import dataclass, asdict
import hashlib
import json
from typing import Any


@dataclass(frozen=True)
class ExperimentRecord:
    session_id: str
    sequence: int
    hypothesis: str
    probe: str
    argument: str
    expected_information: int
    estimated_bytes: int
    observed_value: str | None
    outcome: str
    abstention_reason: str | None

    def canonical(self) -> bytes:
        return json.dumps(asdict(self), sort_keys=True, separators=(",", ":")).encode()

    @property
    def digest(self) -> str:
        return hashlib.sha256(self.canonical()).hexdigest()


def make_record(
    *,
    session_id: str,
    sequence: int,
    hypothesis: str,
    probe: str,
    argument: str,
    expected_information: int,
    estimated_bytes: int,
    observed_value: str | None,
    outcome: str,
    abstention_reason: str | None = None,
) -> ExperimentRecord:
    if not session_id or not hypothesis or not probe:
        raise ValueError("experiment_context_required")
    if expected_information < 0 or estimated_bytes <= 0:
        raise ValueError("experiment_budget_invalid")
    if outcome == "ABSTAIN" and not abstention_reason:
        raise ValueError("abstention_reason_required")
    if outcome != "ABSTAIN" and abstention_reason is not None:
        raise ValueError("abstention_reason_unexpected")
    return ExperimentRecord(
        session_id=session_id,
        sequence=sequence,
        hypothesis=hypothesis,
        probe=probe,
        argument=argument,
        expected_information=expected_information,
        estimated_bytes=estimated_bytes,
        observed_value=observed_value,
        outcome=outcome,
        abstention_reason=abstention_reason,
    )
