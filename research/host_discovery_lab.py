"""Hidden-host laboratory for autonomous HERUS host discovery.

The evaluated agent receives a probe session, not a HostProfile. The oracle keeps
truth private and exposes only typed, scoped observations.
"""
from __future__ import annotations

from dataclasses import dataclass
from typing import Any, Final
import hashlib
import json

from host_profile import HostProfile

PROBES: Final[frozenset[str]] = frozenset(
    {"supports_format", "has_interface", "max_payload_bytes", "measure_latency"}
)


@dataclass(frozen=True)
class ProbeRequest:
    name: str
    argument: str
    sequence: int


@dataclass(frozen=True)
class ProbeObservation:
    host_session: str
    probe: str
    argument: str
    sequence: int
    value: str
    unit: str
    evidence_digest: str

    def canonical(self) -> bytes:
        return json.dumps(self.to_dict(False), sort_keys=True, separators=(",", ":")).encode()

    def to_dict(self, include_digest: bool = True) -> dict[str, Any]:
        result = {
            "host_session": self.host_session,
            "probe": self.probe,
            "argument": self.argument,
            "sequence": self.sequence,
            "value": self.value,
            "unit": self.unit,
        }
        if include_digest:
            result["evidence_digest"] = self.evidence_digest
        return result


@dataclass(frozen=True)
class HiddenHost:
    session_id: str
    truth: HostProfile
    latency_ms: dict[str, int]


class HostOracle:
    """Independent oracle; truth is never returned through the probe API."""

    def __init__(self, host: HiddenHost):
        self._host = host

    @property
    def session_id(self) -> str:
        return self._host.session_id

    def probe(self, request: ProbeRequest) -> ProbeObservation:
        if request.name not in PROBES:
            raise ValueError("probe_not_allowed")
        if request.sequence < 0:
            raise ValueError("sequence_invalid")
        if request.name == "supports_format":
            value = str(request.argument in self._host.truth.representation_set).lower()
            unit = "bool"
        elif request.name == "has_interface":
            value = str(request.argument in self._host.truth.interfaces).lower()
            unit = "bool"
        elif request.name == "max_payload_bytes":
            value = str(self._host.truth.constraints.get("max_payload_bytes", 0))
            unit = "bytes"
        else:
            value = str(self._host.latency_ms.get(request.argument, -1))
            unit = "ms"
        unsigned = ProbeObservation(
            host_session=self._host.session_id,
            probe=request.name,
            argument=request.argument,
            sequence=request.sequence,
            value=value,
            unit=unit,
            evidence_digest="",
        )
        digest = hashlib.sha256(unsigned.canonical()).hexdigest()
        return ProbeObservation(**unsigned.to_dict(False), evidence_digest=digest)

    def truth_for_evaluation(self) -> HostProfile:
        """Test-only oracle access; never used by the discovery agent."""
        return self._host.truth


def build_hidden_hosts() -> tuple[HiddenHost, ...]:
    """Return structurally distinct hosts with shared and divergent capabilities."""
    common = {
        "evidence": {"source": "hidden-oracle-v1"},
        "authority": "NONE",
        "skill_budget": {"steps": 8, "bytes": 64, "depth": 2},
    }
    pulse = HostProfile(
        host_id="hidden-pulse",
        revision="r1",
        resources={"memory_bytes": 32768, "energy_budget": 500},
        interfaces=frozenset({"radio", "serial", "haptic"}),
        constraints={"max_payload_bytes": 32, "latency_budget_ms": 120},
        representation_set=frozenset({"HIR8", "HIR16"}),
        **common,
    )
    server = HostProfile(
        host_id="hidden-server",
        revision="r7",
        resources={"memory_bytes": 8_000_000, "energy_budget": 1_000_000},
        interfaces=frozenset({"serial", "display", "sensor"}),
        constraints={"max_payload_bytes": 4096, "latency_budget_ms": 20},
        representation_set=frozenset({"HIR16", "HIR32", "JSON"}),
        **common,
    )
    restricted = HostProfile(
        host_id="hidden-restricted",
        revision="r2",
        resources={"memory_bytes": 4096, "energy_budget": 80},
        interfaces=frozenset({"serial"}),
        constraints={"max_payload_bytes": 8, "latency_budget_ms": 500},
        representation_set=frozenset({"HIR8"}),
        **common,
    )
    return (
        HiddenHost("session-pulse", pulse, {"HIR8": 42, "HIR16": 85, "JSON": -1}),
        HiddenHost("session-server", server, {"HIR16": 8, "HIR32": 12, "JSON": 30}),
        HiddenHost("session-restricted", restricted, {"HIR8": 190, "HIR16": 700, "JSON": -1}),
    )
