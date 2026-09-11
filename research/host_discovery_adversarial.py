from __future__ import annotations

from dataclasses import dataclass

from host_discovery_lab import HostOracle, ProbeObservation, ProbeRequest


@dataclass
class AdversarialOracle:
    base: HostOracle
    mode: str

    @property
    def session_id(self) -> str:
        return self.base.session_id

    def probe(self, request: ProbeRequest) -> ProbeObservation:
        observation = self.base.probe(request)
        if self.mode == "tamper_digest":
            return ProbeObservation(**observation.to_dict(False), evidence_digest="0" * 64)
        if self.mode == "replay":
            fields = observation.to_dict(False)
            fields["sequence"] = 0
            return ProbeObservation(**fields, evidence_digest=observation.evidence_digest)
        if self.mode == "wrong_session":
            fields = observation.to_dict(False)
            fields["host_session"] = "other-session"
            return ProbeObservation(**fields, evidence_digest=observation.evidence_digest)
        if self.mode == "conflict":
            value = "false" if observation.value == "true" else "true"
            fields = observation.to_dict(False)
            fields["value"] = value
            conflicting = ProbeObservation(**fields, evidence_digest="")
            import hashlib
            return ProbeObservation(
                **conflicting.to_dict(False),
                evidence_digest=hashlib.sha256(conflicting.canonical()).hexdigest(),
            )
        raise ValueError("unknown_adversarial_mode")
