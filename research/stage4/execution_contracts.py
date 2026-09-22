from __future__ import annotations
from dataclasses import dataclass, asdict
import hashlib, json
from enum import Enum

class ExecutionStatus(str, Enum):
    REJECTED='REJECTED'; AUTHORIZED='AUTHORIZED'; IN_FLIGHT='IN_FLIGHT'; COMMITTED='COMMITTED'
    FAILED_PARTIAL='FAILED_PARTIAL'; UNKNOWN_OUTCOME='UNKNOWN_OUTCOME'; RECOVERY_REQUIRED='RECOVERY_REQUIRED'
    RECOVERY_RESOLVED='RECOVERY_RESOLVED'

@dataclass(frozen=True)
class ExecutionEnvelope:
    proposal_digest: str; action_sequence_digest: str; host_context_digest: str
    operation_id: str; attempt_no: int; nonce: str; expires_at: int
    max_steps: int; max_cost: int; authority_id: str; revocation_epoch: int
    no_automatic_retry: bool = True
    def canonical(self): return asdict(self)
    def digest(self): return _digest(self.canonical())

@dataclass(frozen=True)
class ExecutionReceipt:
    operation_id: str; attempt_no: int; step: int; nonce: str; event: str
    committed: bool; cost: int; parent_digest: str | None
    def digest(self): return _digest(asdict(self))

@dataclass(frozen=True)
class RecoveryAttestation:
    operation_id: str; attempt_no: int; conclusion: str
    source_id: str; ledger_digest: str; committed_steps: int | None
    def digest(self): return _digest(asdict(self))

def _digest(value):
    return hashlib.sha256(json.dumps(value, sort_keys=True, separators=(',', ':')).encode()).hexdigest()
