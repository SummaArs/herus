"""H0 as the first real HERUS host: a computer residence with governed exit."""
from __future__ import annotations

from dataclasses import asdict, dataclass
import hashlib
import json
from typing import Iterable

from host_profile import HostProfile
from symbiotic_models import PersistentIdentity, SymbioticState, WorldObservation


@dataclass(frozen=True)
class ExitBundle:
    schema: str
    herus_id: str
    identity_revision: int
    source_host_id: str
    source_host_digest: str
    model_revision: str
    transferable_skills: tuple[str, ...]
    authority: str
    world_context_transferred: bool
    secrets_transferred: bool
    execution_authority_transferred: bool
    source_state: str

    def canonical(self) -> dict[str, object]:
        return asdict(self)

    def digest(self) -> str:
        return hashlib.sha256(json.dumps(self.canonical(), sort_keys=True, separators=(",", ":")).encode()).hexdigest()


class H0ComputerResidence:
    """A bounded H0 host. The computer is a residence, not the HERUS identity."""

    def __init__(self, herus_id: str = "herus-h0"):
        self.identity = PersistentIdentity(herus_id)
        self.state: SymbioticState | None = None
        self.mode = "UNBOUND"
        self._exited = False

    def enter(self, profile: HostProfile, skills: Iterable[str] = ()) -> SymbioticState:
        if self.state is not None and self.state.host is not None:
            raise RuntimeError("h0_already_bound")
        self.state = SymbioticState(identity=self.identity).attach(profile, skills=skills)
        self.identity = self.state.identity
        self.mode = "RESIDENT"
        self._exited = False
        return self.state

    def observe(self, observation: WorldObservation) -> None:
        if self.state is None or self.state.host is None:
            raise RuntimeError("h0_not_resident")
        self.state = SymbioticState(self.state.identity, self.state.host, self.state.world.observe(observation), self.state.self_model)

    def propose(self, skill: str) -> str:
        if self.state is None:
            return "ABSTAIN"
        return self.state.propose(skill)

    def prepare_exit(self) -> ExitBundle:
        if self.state is None or self.state.host is None or self.state.self_model is None:
            raise RuntimeError("h0_not_resident")
        host = self.state.host
        bundle = ExitBundle(
            schema="herus.exit-bundle.v1",
            herus_id=self.state.identity.herus_id,
            identity_revision=self.state.identity.identity_revision,
            source_host_id=host.host_id,
            source_host_digest=host.digest(),
            model_revision="h0-reference-v1",
            transferable_skills=tuple(sorted(self.state.self_model.proven_skills)),
            authority="NONE",
            world_context_transferred=False,
            secrets_transferred=False,
            execution_authority_transferred=False,
            source_state="RESIDENT",
        )
        self.state = self.state.detach()
        self.identity = self.state.identity
        self.mode = "DETACHED"
        self._exited = True
        return bundle

    def validate_exit(self, bundle: ExitBundle) -> tuple[str, ...]:
        issues: list[str] = []
        if bundle.schema != "herus.exit-bundle.v1": issues.append("exit_schema_invalid")
        if bundle.herus_id != self.identity.herus_id: issues.append("exit_identity_mismatch")
        if bundle.authority != "NONE": issues.append("exit_authority_escalation")
        if bundle.world_context_transferred or bundle.secrets_transferred or bundle.execution_authority_transferred:
            issues.append("exit_forbidden_state_transferred")
        return tuple(issues)

    @property
    def resident(self) -> bool:
        return self.mode == "RESIDENT" and self.state is not None and self.state.host is not None
