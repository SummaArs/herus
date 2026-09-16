"""Bounded multi-host migration for the HERUS symbiote.

This proves contract continuity across declared host profiles. It does not prove
physical operation, sensor access, or arbitrary open-world generality.
"""
from __future__ import annotations

from dataclasses import dataclass
from typing import Iterable

from h0_host_runtime import ExitBundle
from host_profile import HostProfile, validate_profile
from symbiotic_models import PersistentIdentity, SymbioticState


ALLOWED_HOST_KINDS = {"computer", "embedded", "wrist", "robot", "vehicle"}


@dataclass(frozen=True)
class MigrationReceipt:
    herus_id: str
    source_host_id: str
    target_host_id: str
    target_host_digest: str
    identity_revision: int
    authority: str
    context_transferred: bool
    execution_authority_transferred: bool
    target_state: str

    def canonical(self) -> dict[str, object]:
        return self.__dict__.copy()


class HostMigration:
    """One-way governed bind from an exited host to a declared target host."""

    def __init__(self, bundle: ExitBundle):
        self.bundle = bundle
        self.state: SymbioticState | None = None
        self.receipt: MigrationReceipt | None = None

    def bind(self, target: HostProfile, *, host_kind: str, skills: Iterable[str] = ()) -> SymbioticState:
        issues = list(validate_profile(target))
        if host_kind not in ALLOWED_HOST_KINDS:
            issues.append("host_kind_unknown")
        if target.host_id == self.bundle.source_host_id:
            issues.append("migration_target_is_source")
        if self.bundle.authority != "NONE":
            issues.append("migration_authority_escalation")
        if self.bundle.world_context_transferred or self.bundle.secrets_transferred:
            issues.append("migration_context_transfer_forbidden")
        if self.bundle.execution_authority_transferred:
            issues.append("migration_execution_transfer_forbidden")
        if issues:
            raise ValueError("migration_rejected: " + ",".join(issues))
        identity = PersistentIdentity(self.bundle.herus_id, identity_revision=self.bundle.identity_revision)
        self.state = SymbioticState(identity=identity).attach(target, skills=skills)
        self.receipt = MigrationReceipt(
            herus_id=self.state.identity.herus_id,
            source_host_id=self.bundle.source_host_id,
            target_host_id=target.host_id,
            target_host_digest=target.digest(),
            identity_revision=self.state.identity.identity_revision,
            authority="NONE",
            context_transferred=False,
            execution_authority_transferred=False,
            target_state="BOUND_CLEAN_CONTEXT",
        )
        return self.state

    def validate(self) -> tuple[str, ...]:
        issues: list[str] = []
        if self.state is None or self.receipt is None:
            return ("migration_not_bound",)
        issues.extend(self.state.validate())
        if self.state.world.observations:
            issues.append("target_world_not_clean")
        if self.state.self_model is not None and self.state.self_model.authority != "NONE":
            issues.append("target_authority_not_none")
        return tuple(issues)
