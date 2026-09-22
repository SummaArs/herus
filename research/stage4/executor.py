from __future__ import annotations
from dataclasses import dataclass
from .execution_contracts import ExecutionEnvelope, ExecutionReceipt, ExecutionStatus, RecoveryAttestation, _digest

@dataclass
class ExternalExecutorAdapter:
    authority_id: str = 'external-authority'
    clock: int = 0
    revocation_epoch: int = 0
    nonce_floor: int = 0
    calls: int = 0
    effects: int = 0
    status: ExecutionStatus = ExecutionStatus.REJECTED
    receipts: list[ExecutionReceipt] | None = None
    def __post_init__(self):
        self.receipts = [] if self.receipts is None else self.receipts

    def revoke(self): self.revocation_epoch += 1

    def execute(self, envelope: ExecutionEnvelope, *, partial_at: int | None = None, ack: bool = True) -> ExecutionStatus:
        self.calls += 1
        if not envelope.no_automatic_retry or envelope.authority_id != self.authority_id:
            return self._reject()
        if envelope.expires_at <= self.clock or envelope.revocation_epoch != self.revocation_epoch:
            return self._reject()
        try: nonce_value = int(envelope.nonce)
        except ValueError: return self._reject()
        if nonce_value <= self.nonce_floor: return self._reject()
        self.nonce_floor = nonce_value
        if envelope.max_steps < 1 or envelope.max_cost < 1:
            return self._reject()
        for step in range(1, envelope.max_steps + 1):
            self.clock += 1
            committed = True
            self.effects += 1
            receipt = ExecutionReceipt(envelope.operation_id, envelope.attempt_no, step, envelope.nonce, 'commit', committed, 1, self.receipts[-1].digest() if self.receipts else None)
            self.receipts.append(receipt)
            if partial_at == step:
                self.status = ExecutionStatus.FAILED_PARTIAL
                return self.status
            if not ack:
                self.status = ExecutionStatus.UNKNOWN_OUTCOME
                return self.status
        self.status = ExecutionStatus.COMMITTED
        return self.status

    def _reject(self):
        self.status = ExecutionStatus.REJECTED
        return self.status

    def recover(self, attestation: RecoveryAttestation) -> ExecutionStatus:
        if attestation.source_id == self.authority_id:
            self.status = ExecutionStatus.RECOVERY_REQUIRED
            return self.status
        if attestation.conclusion not in {'RESOLVED_COMMITTED','RESOLVED_NOT_COMMITTED','RESOLVED_PARTIAL','RECOVERY_CONFLICT'}:
            self.status = ExecutionStatus.RECOVERY_REQUIRED
            return self.status
        self.status = ExecutionStatus.RECOVERY_RESOLVED if attestation.conclusion != 'RECOVERY_CONFLICT' else ExecutionStatus.RECOVERY_REQUIRED
        return self.status
