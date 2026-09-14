"""Finite channel contracts for HERUS symbiosis.

A channel transports evidence and proposals. It never grants authority by itself.
"""
from __future__ import annotations

from dataclasses import dataclass
from enum import Enum


class Channel(str, Enum):
    USB = "USB"
    BLE = "BLE"
    LORA = "LORA"
    WIFI = "WIFI"
    UART = "UART"
    I2C = "I2C"
    SPI = "SPI"
    CAN = "CAN"
    RS485 = "RS485"


class ChannelRole(str, Enum):
    OBSERVE = "OBSERVE"
    PROPOSE = "PROPOSE"
    CONTROL = "CONTROL"


@dataclass(frozen=True)
class ChannelContract:
    channel: Channel
    role: ChannelRole
    authenticated: bool
    encrypted: bool
    max_payload: int
    effects: frozenset[str]

    def can_transport(self, effect: str, authenticated: bool) -> bool:
        """Transport eligibility is necessary, never sufficient, for execution."""
        return (
            authenticated
            and self.authenticated
            and effect in self.effects
            and self.role is not ChannelRole.OBSERVE
        )


DEFAULT_CHANNEL_CONTRACTS = {
    Channel.USB: ChannelContract(Channel.USB, ChannelRole.PROPOSE, True, True, 4096, frozenset({"READ_TELEMETRY", "PROPOSE_ACTION"})),
    Channel.BLE: ChannelContract(Channel.BLE, ChannelRole.PROPOSE, True, True, 512, frozenset({"READ_TELEMETRY", "HAPTIC_FEEDBACK", "PROPOSE_ACTION", "ISOLATE_DEVICE"})),
    Channel.LORA: ChannelContract(Channel.LORA, ChannelRole.PROPOSE, True, True, 222, frozenset({"READ_TELEMETRY", "PROPOSE_ACTION", "ALERT"})),
    Channel.WIFI: ChannelContract(Channel.WIFI, ChannelRole.PROPOSE, True, True, 4096, frozenset({"READ_TELEMETRY", "PROPOSE_ACTION", "ALERT"})),
    Channel.UART: ChannelContract(Channel.UART, ChannelRole.CONTROL, False, False, 1024, frozenset({"READ_TELEMETRY", "PROPOSE_ACTION"})),
    Channel.I2C: ChannelContract(Channel.I2C, ChannelRole.CONTROL, False, False, 256, frozenset({"READ_TELEMETRY", "HAPTIC_FEEDBACK"})),
    Channel.SPI: ChannelContract(Channel.SPI, ChannelRole.CONTROL, False, False, 4096, frozenset({"READ_TELEMETRY"})),
    Channel.CAN: ChannelContract(Channel.CAN, ChannelRole.PROPOSE, True, True, 64, frozenset({"READ_TELEMETRY", "PROPOSE_ACTION", "ALERT"})),
    Channel.RS485: ChannelContract(Channel.RS485, ChannelRole.PROPOSE, True, True, 1024, frozenset({"READ_TELEMETRY", "PROPOSE_ACTION"})),
}


def channel_for(channel: Channel) -> ChannelContract:
    return DEFAULT_CHANNEL_CONTRACTS[channel]
