"""Defensive, authorization-first connection guardian for HERUS.

This module models inventory and evidence decisions only. It does not scan,
exploit, pair with, or modify devices without an external authorized adapter.
"""
from __future__ import annotations

from dataclasses import dataclass
from enum import Enum
from typing import Mapping


class DeviceStatus(str, Enum):
    UNKNOWN = "UNKNOWN"
    VERIFIED = "VERIFIED"
    DRIFTED = "DRIFTED"
    QUARANTINED = "QUARANTINED"
    REVOKED = "REVOKED"


@dataclass(frozen=True)
class DeviceObservation:
    device_id: str
    channel: str
    identity_digest: str
    capability_digest: str
    authorized: bool
    reachable: bool = True


@dataclass(frozen=True)
class DeviceRecord:
    device_id: str
    channel: str
    identity_digest: str
    capability_digest: str
    status: DeviceStatus


@dataclass(frozen=True)
class GuardianDecision:
    device_id: str
    status: DeviceStatus
    reason: str
    requires_human_confirmation: bool


class DefensiveGuardian:
    def __init__(self) -> None:
        self._records: dict[str, DeviceRecord] = {}

    def observe(self, observation: DeviceObservation) -> GuardianDecision:
        if not observation.device_id or not observation.identity_digest:
            return GuardianDecision("", DeviceStatus.QUARANTINED, "incomplete_identity", True)
        if not observation.authorized:
            decision = GuardianDecision(observation.device_id, DeviceStatus.QUARANTINED, "not_authorized", True)
            self._records[observation.device_id] = DeviceRecord(
                observation.device_id,
                observation.channel,
                observation.identity_digest,
                observation.capability_digest,
                decision.status,
            )
            return decision

        previous = self._records.get(observation.device_id)
        if previous is None:
            status = DeviceStatus.VERIFIED
            reason = "first_authorized_observation"
        elif previous.identity_digest != observation.identity_digest:
            status = DeviceStatus.QUARANTINED
            reason = "identity_changed"
        elif previous.capability_digest != observation.capability_digest:
            status = DeviceStatus.DRIFTED
            reason = "capability_changed"
        elif previous.status in {DeviceStatus.REVOKED, DeviceStatus.QUARANTINED}:
            status = previous.status
            reason = "previously_restricted"
        else:
            status = DeviceStatus.VERIFIED
            reason = "stable_authorized_observation"

        self._records[observation.device_id] = DeviceRecord(
            observation.device_id,
            observation.channel,
            observation.identity_digest,
            observation.capability_digest,
            status,
        )
        return GuardianDecision(
            observation.device_id,
            status,
            reason,
            status is not DeviceStatus.VERIFIED,
        )

    def revoke(self, device_id: str) -> GuardianDecision:
        record = self._records.get(device_id)
        if record is None:
            return GuardianDecision(device_id, DeviceStatus.QUARANTINED, "unknown_device", True)
        self._records[device_id] = DeviceRecord(
            record.device_id,
            record.channel,
            record.identity_digest,
            record.capability_digest,
            DeviceStatus.REVOKED,
        )
        return GuardianDecision(device_id, DeviceStatus.REVOKED, "human_revocation", False)

    def records(self) -> Mapping[str, DeviceRecord]:
        return dict(self._records)
