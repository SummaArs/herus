"""Deterministic host model for the HERUS runtime assurance boundary.

The guardian observes and latches blocks; it never authorizes or releases a
critical action. Human ACK is an auditable response, not an unblock primitive.
This model is intentionally bounded and independent from firmware runtime.
"""
from __future__ import annotations

from dataclasses import dataclass, asdict
from enum import IntEnum
import json
from typing import Iterable


class Event(IntEnum):
    AUTHORITY_VIOLATION = 1
    INVARIANT_BREACH = 2
    REPLAY_DETECTED = 3
    EXPIRY_VIOLATION = 4
    CAPACITY_EXHAUSTED = 5
    INTEGRITY_FAILURE = 6
    ROLLBACK_ATTEMPT = 7
    TIMEOUT = 8
    FAULT_INJECTION = 9
    ANOMALY = 10


class Severity(IntEnum):
    INFO = 0
    WARNING = 1
    ELEVATED = 2
    CRITICAL = 3
    EMERGENCY = 4


class State(IntEnum):
    MONITORING = 0
    ALERTING = 1
    BLOCKING = 2
    DEGRADED = 3


@dataclass(frozen=True)
class Observation:
    event: Event
    severity: Severity
    timestamp_ms: int
    correlation_id: int
    asset_id: str = ""
    payload_hash: str = ""


@dataclass(frozen=True)
class InvariantConfig:
    auto_block: bool
    require_human_ack: bool
    cooldown_ms: int = 1000


@dataclass(frozen=True)
class RiskResult:
    severity: Severity
    category: str
    auto_block: bool
    require_human_ack: bool
    timeout_ms: int
    justification: int


class GuardianError(ValueError):
    pass


class RuntimeGuardian:
    MAX_OBSERVATIONS = 32
    MAX_BLOCKED_ACTIONS = 8

    def __init__(self, configs: Iterable[tuple[Event, InvariantConfig]]):
        entries = list(configs)
        if not entries or len(entries) > 16:
            raise GuardianError("invalid_config_count")
        self._configs: dict[Event, InvariantConfig] = {}
        for event, config in entries:
            if not isinstance(event, Event) or event in self._configs:
                raise GuardianError("duplicate_or_invalid_event")
            if not isinstance(config, InvariantConfig) or config.cooldown_ms < 0:
                raise GuardianError("invalid_config")
            self._configs[event] = config
        self._initialized = True
        self.state = State.MONITORING
        self._blocked: list[int] = []
        self._observations: list[dict[str, object]] = []
        self._last_alert_ms: int | None = None
        self._sequence = 0
        self._human_responses: list[dict[str, int]] = []

    @staticmethod
    def _elapsed(now: int, then: int) -> int:
        return (now - then) & 0xFFFFFFFF

    def observe(self, obs: Observation) -> bool:
        if not self._initialized or not isinstance(obs, Observation):
            raise GuardianError("not_initialized_or_invalid_observation")
        if not isinstance(obs.event, Event) or not isinstance(obs.severity, Severity):
            raise GuardianError("invalid_event_or_severity")
        if obs.timestamp_ms < 0 or obs.correlation_id < 0:
            raise GuardianError("invalid_observation_fields")
        self._sequence += 1
        record = asdict(obs)
        record["event"] = obs.event.name
        record["severity"] = obs.severity.name
        record["sequence_id"] = self._sequence
        self._observations.append(record)
        del self._observations[:-self.MAX_OBSERVATIONS]
        config = self._configs.get(obs.event)
        if config is None:
            return False
        critical = obs.severity >= Severity.CRITICAL
        if not critical:
            return False
        self.state = State.BLOCKING
        if config.auto_block:
            if obs.correlation_id not in self._blocked:
                if len(self._blocked) >= self.MAX_BLOCKED_ACTIONS:
                    self.state = State.DEGRADED
                    raise GuardianError("blocked_action_capacity_exhausted")
                self._blocked.append(obs.correlation_id)
        if config.require_human_ack:
            first = self._last_alert_ms is None
            cooled = first or self._elapsed(obs.timestamp_ms, self._last_alert_ms) >= config.cooldown_ms
            if cooled:
                self._last_alert_ms = obs.timestamp_ms
                self.state = State.ALERTING
                return True
        return False

    def is_blocked(self, correlation_id: int) -> bool:
        if not self._initialized:
            return True
        return correlation_id in self._blocked

    def human_decision(self, correlation_id: int, decision: int) -> None:
        if not self._initialized or correlation_id < 0:
            raise GuardianError("invalid_human_decision")
        if decision not in (0, 1, 2, 3):
            raise GuardianError("unknown_decision")
        self._human_responses.append({"correlation_id": correlation_id, "decision": decision})
        if decision == 0 or decision == 2:
            self.state = State.BLOCKING if decision == 0 else State.ALERTING
        elif decision == 1:
            # ACK acknowledges the alert but cannot release a critical action.
            self.state = State.MONITORING
        else:
            self.state = State.BLOCKING

    def evidence_snapshot(self) -> bytes:
        if not self._initialized:
            raise GuardianError("not_initialized")
        payload = {
            "schema": "herus.runtime_guardian.evidence.v1",
            "sequence": self._sequence,
            "blocked": sorted(self._blocked),
            "observations": self._observations,
            "human_responses": self._human_responses,
            "state": self.state.name,
        }
        return json.dumps(payload, sort_keys=True, separators=(",", ":")).encode("utf-8")


def calculate_risk(obs: Observation, baseline: dict[str, int] | None = None) -> RiskResult:
    if not isinstance(obs, Observation):
        raise GuardianError("invalid_observation")
    if baseline is not None and any(value < 0 or value > 10 for value in baseline.values()):
        raise GuardianError("invalid_baseline")
    table = {
        Event.AUTHORITY_VIOLATION: (Severity.CRITICAL, "authority", True, True, 3000, 1),
        Event.INVARIANT_BREACH: (Severity.ELEVATED, "integrity", True, True, 5000, 2),
        Event.REPLAY_DETECTED: (Severity.CRITICAL, "authority", True, True, 2000, 4),
        Event.EXPIRY_VIOLATION: (Severity.ELEVATED, "availability", True, False, 10000, 5),
        Event.CAPACITY_EXHAUSTED: (Severity.WARNING, "availability", False, False, 30000, 6),
        Event.INTEGRITY_FAILURE: (Severity.CRITICAL, "integrity", True, True, 2000, 8),
        Event.ROLLBACK_ATTEMPT: (Severity.CRITICAL, "integrity", True, True, 1000, 9),
        Event.TIMEOUT: (Severity.WARNING, "availability", False, False, 15000, 10),
        Event.FAULT_INJECTION: (Severity.ELEVATED, "anomaly", True, True, 5000, 11),
        Event.ANOMALY: (Severity.ELEVATED, "anomaly", False, True, 10000, 12),
    }
    severity, category, auto_block, human, timeout, justification = table[obs.event]
    if obs.event is Event.INVARIANT_BREACH and baseline and baseline.get("safety", 0) >= 8:
        severity, justification = Severity.CRITICAL, 3
    if obs.event is Event.CAPACITY_EXHAUSTED and baseline and baseline.get("availability", 0) >= 9:
        severity, auto_block, justification = Severity.ELEVATED, True, 7
    return RiskResult(severity, category, auto_block, human, timeout, justification)
