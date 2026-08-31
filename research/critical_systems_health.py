"""Finite health-policy synthesis and verification for critical-system research.

This module is deliberately inert: it classifies quantized observations and
returns a decision record. It cannot call hardware, persist state, or actuate.
"""
from __future__ import annotations

from dataclasses import dataclass
from enum import Enum
from itertools import product
from typing import Iterable


class HealthState(str, Enum):
    NOMINAL = "nominal"
    DEGRADING = "degrading"
    MARGIN_LOW = "margin_low"
    SAFE_HOLD = "safe_hold"
    UNKNOWN = "unknown"


@dataclass(frozen=True)
class Observation:
    temperature: int
    voltage: int
    vibration: int
    comm_ok: bool
    human_confirmed: bool

    def valid(self) -> bool:
        return (
            0 <= self.temperature <= 100
            and 0 <= self.voltage <= 100
            and 0 <= self.vibration <= 100
        )


@dataclass(frozen=True)
class Policy:
    temperature_warning: int = 70
    temperature_critical: int = 90
    voltage_warning: int = 30
    voltage_critical: int = 15
    vibration_warning: int = 60
    vibration_critical: int = 85

    def valid(self) -> bool:
        return (
            0 <= self.temperature_warning < self.temperature_critical <= 100
            and 0 <= self.voltage_critical < self.voltage_warning <= 100
            and 0 <= self.vibration_warning < self.vibration_critical <= 100
        )


@dataclass(frozen=True)
class Decision:
    state: HealthState
    can_actuate: bool
    reason: str


def classify(policy: Policy, observation: Observation) -> Decision:
    if not policy.valid():
        return Decision(HealthState.UNKNOWN, False, "invalid_policy")
    if not observation.valid():
        return Decision(HealthState.UNKNOWN, False, "invalid_observation")
    if not observation.comm_ok:
        return Decision(HealthState.SAFE_HOLD, False, "communication_lost")
    if (
        observation.temperature >= policy.temperature_critical
        or observation.voltage <= policy.voltage_critical
        or observation.vibration >= policy.vibration_critical
    ):
        return Decision(HealthState.SAFE_HOLD, False, "critical_limit")
    if (
        observation.temperature >= policy.temperature_warning
        or observation.voltage <= policy.voltage_warning
        or observation.vibration >= policy.vibration_warning
    ):
        return Decision(HealthState.MARGIN_LOW, False, "warning_margin")
    if observation.human_confirmed:
        return Decision(HealthState.NOMINAL, True, "human_confirmation")
    return Decision(HealthState.NOMINAL, False, "confirmation_required")


def finite_observations() -> tuple[Observation, ...]:
    """Return a bounded verification domain, not a claim about all sensors."""
    return tuple(
        Observation(t, v, vibration, comm_ok, confirmed)
        for t, v, vibration, comm_ok, confirmed in product(
            (0, 70, 90, 101), (0, 30, 90), (0, 60, 85), (False, True), (False, True)
        )
    )


def verify_policy(policy: Policy, observations: Iterable[Observation] | None = None) -> tuple[str, ...]:
    """Check fail-closed invariants over a finite declared domain."""
    failures: list[str] = []
    if not policy.valid():
        return ("invalid_policy",)
    if observations is None:
        observations = finite_observations()
    for observation in observations:
        decision = classify(policy, observation)
        if decision.can_actuate and (
            decision.state is not HealthState.NOMINAL or not observation.human_confirmed
        ):
            failures.append("actuation_without_nominal_human_confirmation")
        if not observation.valid():
            if decision.state is not HealthState.UNKNOWN:
                failures.append("invalid_observation_not_unknown")
            continue
        if not observation.comm_ok and decision.state is not HealthState.SAFE_HOLD:
            failures.append("communication_loss_not_safe_hold")
        if (
            observation.valid()
            and observation.comm_ok
            and (
                observation.temperature >= policy.temperature_critical
                or observation.voltage <= policy.voltage_critical
                or observation.vibration >= policy.vibration_critical
            )
            and decision.state is not HealthState.SAFE_HOLD
        ):
            failures.append("critical_limit_not_safe_hold")
    return tuple(sorted(set(failures)))


def synthesize_policy() -> Policy | None:
    """Select the first bounded policy satisfying the declared invariants."""
    candidates = (
        Policy(70, 90, 30, 15, 60, 85),
        Policy(60, 80, 40, 20, 50, 75),
        Policy(75, 95, 25, 10, 65, 90),
    )
    for policy in candidates:
        if not verify_policy(policy):
            return policy
    return None
