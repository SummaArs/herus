"""Bounded action lease for multi-host coordination.

A lease serializes one proposal scope. Its monotonically increasing fencing
 token prevents an expired or migrated instance from acting after reacquisition.
It grants no authority by itself; it only prevents stale coordination.
"""
from __future__ import annotations

from dataclasses import dataclass
from typing import Callable


@dataclass(frozen=True)
class ActionLease:
    action_id: str
    owner: str
    fencing_token: int
    expires_at: float


class ActionLeaseBook:
    def __init__(self, ttl: float = 30.0, clock: Callable[[], float] | None = None):
        if ttl <= 0:
            raise ValueError("lease_ttl_invalid")
        import time
        self.ttl = ttl
        self.clock = clock or time.monotonic
        self._leases: dict[str, ActionLease] = {}
        self._next_token = 0

    def acquire(self, action_id: str, owner: str) -> ActionLease | None:
        if not action_id or not owner:
            raise ValueError("lease_identity_invalid")
        current = self._leases.get(action_id)
        now = self.clock()
        if current is not None and current.expires_at > now:
            return None
        self._next_token += 1
        lease = ActionLease(action_id, owner, self._next_token, now + self.ttl)
        self._leases[action_id] = lease
        return lease

    def valid(self, lease: ActionLease) -> bool:
        current = self._leases.get(lease.action_id)
        return bool(current and current == lease and current.expires_at > self.clock())

    def renew(self, lease: ActionLease) -> ActionLease | None:
        if not self.valid(lease):
            return None
        renewed = ActionLease(lease.action_id, lease.owner, lease.fencing_token, self.clock() + self.ttl)
        self._leases[lease.action_id] = renewed
        return renewed

    def release(self, lease: ActionLease) -> bool:
        if not self.valid(lease):
            return False
        del self._leases[lease.action_id]
        return True

    def fencing_token(self, action_id: str) -> int:
        lease = self._leases.get(action_id)
        return lease.fencing_token if lease and lease.expires_at > self.clock() else 0
