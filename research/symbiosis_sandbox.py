"""Finite cross-domain scenarios for HERUS symbiosis validation.

All scenarios are local and side-effect free. They produce decisions and
proposals; no device, account, motor, or network service is controlled.
"""
from __future__ import annotations

from dataclasses import dataclass

from defensive_guardian import DefensiveGuardian, DeviceObservation, GuardianDecision
from domain_contract import Domain, EffectLevel, contract_for
from guardian_haptics import haptic_for_guardian


@dataclass(frozen=True)
class SandboxResult:
    domain: Domain
    guardian: GuardianDecision
    effect_permitted: bool
    haptic_state: str | None


def evaluate_robot_legacy() -> SandboxResult:
    guardian = DefensiveGuardian()
    decision = guardian.observe(
        DeviceObservation(
            device_id="legacy-robot",
            channel="UART",
            identity_digest="robot-id-v1",
            capability_digest="sensors-only",
            authorized=True,
        )
    )
    contract = contract_for(Domain.ROBOTICS)
    permitted = contract.permits(EffectLevel.SIMULATE, {"host_digest", "world_digest", "safety_check"})
    proposal = haptic_for_guardian(decision, "robot-evidence")
    return SandboxResult(Domain.ROBOTICS, decision, permitted, proposal.state.value if proposal else None)


def evaluate_bluetooth_change() -> SandboxResult:
    guardian = DefensiveGuardian()
    guardian.observe(
        DeviceObservation(
            device_id="ble-peer",
            channel="BLE",
            identity_digest="peer-id-v1",
            capability_digest="telemetry-only",
            authorized=True,
        )
    )
    decision = guardian.observe(
        DeviceObservation(
            device_id="ble-peer",
            channel="BLE",
            identity_digest="peer-id-v1",
            capability_digest="telemetry-and-control",
            authorized=True,
        )
    )
    proposal = haptic_for_guardian(decision, "ble-drift-evidence")
    return SandboxResult(Domain.PULSE, decision, False, proposal.state.value if proposal else None)


def evaluate_finance_observation() -> SandboxResult:
    guardian = DefensiveGuardian()
    decision = guardian.observe(
        DeviceObservation(
            device_id="ofr-observer",
            channel="WIFI",
            identity_digest="observer-id-v1",
            capability_digest="read-only-regime-data",
            authorized=True,
        )
    )
    contract = contract_for(Domain.FINANCE_SANDBOX)
    permitted = contract.permits(EffectLevel.SIMULATE, {"host_digest", "data_digest", "sandbox_id"})
    proposal = haptic_for_guardian(decision, "finance-evidence")
    return SandboxResult(Domain.FINANCE_SANDBOX, decision, permitted, proposal.state.value if proposal else None)
