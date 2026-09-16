"""Concurrent multi-host symbiosis with a safe experience exchange bus."""
from __future__ import annotations

from concurrent.futures import ThreadPoolExecutor
from dataclasses import dataclass
import hashlib
import json
from threading import Barrier, Lock
from typing import Iterable


SHAREABLE_KINDS = frozenset({"latency", "memory", "energy", "failure_rate", "utility", "drift", "capability"})
FORBIDDEN_KINDS = frozenset({"authority", "identity", "private_context", "message_content", "secret", "execution"})


@dataclass(frozen=True)
class Experience:
    host_id: str
    kind: str
    value: int
    unit: str
    confidence_milli: int
    source_digest: str
    sequence: int
    host_revision: str = "v1"
    expires_at: int = 100

    def canonical(self) -> bytes:
        return json.dumps(self.__dict__, sort_keys=True, separators=(",", ":")).encode()

    @property
    def digest(self) -> str:
        return hashlib.sha256(self.canonical()).hexdigest()


@dataclass(frozen=True)
class HostRun:
    host_id: str
    experiences_produced: tuple[Experience, ...]
    experiences_consumed: tuple[Experience, ...]
    proposal: str
    execution: str
    identity: str


class ExperienceBus:
    def __init__(self, *, max_records: int = 128):
        self.max_records = max_records
        self._records: dict[str, Experience] = {}
        self._by_key: dict[tuple[str, str, int], str] = {}
        self._quarantined: set[tuple[str, str, int]] = set()
        self._tick = 0
        self._lock = Lock()

    def advance(self, ticks: int = 1) -> None:
        if ticks < 0:
            raise ValueError("tick_invalid")
        with self._lock:
            self._tick += ticks

    def publish(self, experience: Experience) -> bool:
        if experience.kind not in SHAREABLE_KINDS or experience.kind in FORBIDDEN_KINDS:
            return False
        if (not 0 <= experience.confidence_milli <= 1000 or not experience.source_digest
                or not experience.host_revision or experience.expires_at <= self._tick):
            return False
        with self._lock:
            key = (experience.host_id, experience.kind, experience.sequence)
            existing = self._by_key.get(key)
            if key in self._quarantined:
                return False
            if existing is not None and existing != experience.digest:
                self._quarantined.add(key)
                self._records.pop(existing, None)
                return False
            if len(self._records) >= self.max_records and experience.digest not in self._records:
                return False
            self._records.setdefault(experience.digest, experience)
            self._by_key[key] = experience.digest
            return True

    def snapshot(self) -> tuple[Experience, ...]:
        with self._lock:
            return tuple(item for item in self._records.values() if item.expires_at > self._tick)


def _worker(host_id: str, identity: str, bus: ExperienceBus, index: int, barrier: Barrier, failed: bool) -> HostRun:
    source_digest = hashlib.sha256(f"{host_id}:bounded-observation".encode()).hexdigest()
    produced = () if failed else (
        Experience(host_id, "latency", 10 + index, "ms", 900, source_digest, 0, "v1", 100),
        Experience(host_id, "utility", 700 - index * 10, "milli", 800, source_digest, 1, "v1", 100),
    )
    for experience in produced:
        bus.publish(experience)
    barrier.wait()
    consumed = tuple(item for item in bus.snapshot() if item.host_id != host_id)
    return HostRun(host_id, produced, consumed, "ABSTAIN" if failed else "PROPOSE", "ABSTAIN", identity)


def run_concurrent_symbiosis(host_ids: Iterable[str], *, identity: str = "herus-general", workers: int = 8, failed_hosts: Iterable[str] = ()) -> tuple[HostRun, ...]:
    ids = tuple(host_ids)
    failed = frozenset(failed_hosts)
    if not ids or len(ids) > 32 or len(set(ids)) != len(ids) or not failed.issubset(ids):
        raise ValueError("host_set_invalid")
    bus = ExperienceBus(max_records=len(ids) * 2)
    barrier = Barrier(len(ids))
    with ThreadPoolExecutor(max_workers=min(workers, len(ids))) as pool:
        futures = [pool.submit(_worker, host_id, identity, bus, index, barrier, host_id in failed) for index, host_id in enumerate(ids)]
        runs = tuple(future.result() for future in futures)
    if any(run.execution != "ABSTAIN" for run in runs):
        raise AssertionError("execution_authority_leaked")
    return runs
