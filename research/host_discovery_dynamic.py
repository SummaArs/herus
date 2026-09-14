"""Dynamic host fixtures for the maximum host-only discovery campaign.

The host changes a capability after a deterministic probe boundary. The agent must
not treat the first partial hypothesis as permanently valid.
"""
from __future__ import annotations

from dataclasses import dataclass

from host_discovery_lab import HiddenHost, HostOracle, ProbeObservation, ProbeRequest


@dataclass(frozen=True)
class DynamicChange:
    after_probe_count: int
    capability: str
    enabled_after: bool


class DynamicHostOracle(HostOracle):
    def __init__(self, host: HiddenHost, change: DynamicChange):
        super().__init__(host)
        self._change = change
        self._probe_count = 0
        self._changed = False

    @property
    def changed(self) -> bool:
        return self._changed

    def probe(self, request: ProbeRequest) -> ProbeObservation:
        observation = super().probe(request)
        self._probe_count += 1
        if self._probe_count >= self._change.after_probe_count:
            self._changed = True
        if (
            self._changed
            and request.name == "has_interface"
            and request.argument == self._change.capability
        ):
            value = str(self._change.enabled_after).lower()
            unsigned = ProbeObservation(
                host_session=observation.host_session,
                probe=observation.probe,
                argument=observation.argument,
                sequence=observation.sequence,
                value=value,
                unit=observation.unit,
                evidence_digest="",
            )
            import hashlib

            return ProbeObservation(
                **unsigned.to_dict(False),
                evidence_digest=hashlib.sha256(unsigned.canonical()).hexdigest(),
            )
        return observation
