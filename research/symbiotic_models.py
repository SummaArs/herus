"""Finite World/Host/Self models for the HERUS symbiotic runtime.

This module deliberately models *claims* and *proven capabilities*, not open-world
intelligence.  Discovery can produce observations, but it cannot grant authority.
The persistent HERUS identity is independent from the current host binding.
"""
from __future__ import annotations

from dataclasses import dataclass, field, replace
import hashlib
import json
from typing import Iterable, Mapping

from host_profile import HostProfile, validate_profile


ALLOWED_OBSERVATION_STATES = {"OBSERVED", "CONFIRMED", "REJECTED"}
ALLOWED_CONFIDENCE = {"UNPROVEN", "SUPPORTED", "PROVEN"}
ALLOWED_ACTIONS = {"NONE", "PROPOSE", "EXECUTE"}


@dataclass(frozen=True)
class WorldObservation:
    """A bounded, provenance-bearing claim about the external world."""

    subject: str
    predicate: str
    value: str
    source: str
    state: str = "OBSERVED"
    confidence: str = "SUPPORTED"

    def validate(self) -> tuple[str, ...]:
        issues: list[str] = []
        if not self.subject or not self.predicate or not self.value:
            issues.append("world_observation_incomplete")
        if not self.source:
            issues.append("world_observation_source_missing")
        if self.state not in ALLOWED_OBSERVATION_STATES:
            issues.append("world_observation_state_unknown")
        if self.confidence not in ALLOWED_CONFIDENCE:
            issues.append("world_observation_confidence_unknown")
        if self.state == "CONFIRMED" and self.confidence != "PROVEN":
            issues.append("confirmed_requires_proven_confidence")
        return tuple(issues)

    def canonical(self) -> dict[str, str]:
        return {
            "subject": self.subject,
            "predicate": self.predicate,
            "value": self.value,
            "source": self.source,
            "state": self.state,
            "confidence": self.confidence,
        }


@dataclass(frozen=True)
class WorldModel:
    """Append-only finite world claims; invalid claims are never admitted."""

    observations: tuple[WorldObservation, ...] = ()

    def observe(self, observation: WorldObservation) -> "WorldModel":
        if observation.validate():
            raise ValueError("invalid world observation: " + ",".join(observation.validate()))
        return replace(self, observations=self.observations + (observation,))

    def digest(self) -> str:
        payload = [item.canonical() for item in self.observations]
        return hashlib.sha256(json.dumps(payload, sort_keys=True, separators=(",", ":")).encode()).hexdigest()


@dataclass(frozen=True)
class SelfModel:
    """What this HERUS instance may propose, based on proven host facts."""

    herus_id: str
    host_digest: str
    proven_skills: frozenset[str] = frozenset()
    representations: frozenset[str] = frozenset()
    authority: str = "NONE"
    limits: tuple[str, ...] = ()

    def can_propose(self, skill: str) -> bool:
        return skill in self.proven_skills and self.authority in {"NONE", "PROPOSAL_ONLY", "HUMAN_BOUND"}

    def can_execute(self, skill: str) -> bool:
        # Execution is intentionally impossible from discovery alone.  A separate
        # authorization contract must convert a proposal into an executable order.
        return False

    def validate(self) -> tuple[str, ...]:
        issues: list[str] = []
        if not self.herus_id:
            issues.append("self_identity_missing")
        if not self.host_digest:
            issues.append("self_host_binding_missing")
        if self.authority not in {"NONE", "PROPOSAL_ONLY", "HUMAN_BOUND"}:
            issues.append("self_authority_unknown")
        return tuple(issues)


@dataclass(frozen=True)
class PersistentIdentity:
    """Stable HERUS identity with a separately replaceable host binding."""

    herus_id: str
    identity_revision: int = 1
    host_id: str | None = None
    host_digest: str | None = None
    continuity_events: tuple[str, ...] = ()

    def bind(self, profile: HostProfile) -> "PersistentIdentity":
        issues = validate_profile(profile)
        if issues:
            raise ValueError("cannot bind invalid host profile: " + ",".join(issues))
        event = f"bind:{profile.host_id}:{profile.digest()}"
        return replace(self, host_id=profile.host_id, host_digest=profile.digest(), continuity_events=self.continuity_events + (event,))

    def unbind(self) -> "PersistentIdentity":
        if self.host_id is None:
            return self
        event = f"unbind:{self.host_id}:{self.host_digest}"
        return replace(self, host_id=None, host_digest=None, continuity_events=self.continuity_events + (event,))

    def validate(self) -> tuple[str, ...]:
        issues: list[str] = []
        if not self.herus_id:
            issues.append("herus_identity_missing")
        if self.identity_revision < 1:
            issues.append("identity_revision_invalid")
        if (self.host_id is None) != (self.host_digest is None):
            issues.append("partial_host_binding")
        return tuple(issues)


@dataclass(frozen=True)
class SymbioticState:
    """Atomic snapshot joining the three models without merging their authority."""

    identity: PersistentIdentity
    host: HostProfile | None = None
    world: WorldModel = field(default_factory=WorldModel)
    self_model: SelfModel | None = None

    def attach(self, profile: HostProfile, *, skills: Iterable[str] = ()) -> "SymbioticState":
        bound = self.identity.bind(profile)
        self_model = SelfModel(
            herus_id=bound.herus_id,
            host_digest=profile.digest(),
            proven_skills=frozenset(skills),
            representations=profile.representation_set,
            authority="NONE",
            limits=("discovery_does_not_authorize", "execution_requires_external_authorization"),
        )
        return replace(self, identity=bound, host=profile, self_model=self_model)

    def detach(self) -> "SymbioticState":
        detached = self.identity.unbind()
        return replace(self, identity=detached, host=None, self_model=None)

    def validate(self) -> tuple[str, ...]:
        issues = list(self.identity.validate())
        if self.host is None:
            if self.identity.host_id is not None:
                issues.append("identity_bound_without_host")
            if self.self_model is not None:
                issues.append("self_model_without_host")
            return tuple(issues)
        issues.extend(validate_profile(self.host))
        if self.identity.host_id != self.host.host_id or self.identity.host_digest != self.host.digest():
            issues.append("identity_host_mismatch")
        if self.self_model is None:
            issues.append("self_model_missing")
        elif self.self_model.host_digest != self.host.digest():
            issues.append("self_model_host_mismatch")
        return tuple(issues)

    def propose(self, skill: str) -> str:
        if self.self_model is None or not self.self_model.can_propose(skill):
            return "ABSTAIN"
        return "PROPOSE"

    def execute(self, skill: str) -> str:
        if self.self_model is None or not self.self_model.can_execute(skill):
            return "ABSTAIN"
        return "EXECUTE"
