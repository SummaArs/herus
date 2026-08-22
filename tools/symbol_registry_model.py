"""Host-only collision-aware symbol registry model for HERUS.

This module is deliberately independent from firmware ABI. It models the
properties that must be proved before changing the C reasoner representation:
exact membership for a static factory namespace and explicit interning for a
bounded personal namespace.
"""

from __future__ import annotations

from dataclasses import dataclass
from enum import Enum, IntEnum
from typing import Dict, Iterable, List, Tuple

_HASH_OFFSET = 2166136261
_HASH_PRIME = 16777619


class RegistryStatus(str, Enum):
    OK = "ok"
    UNKNOWN = "unknown"
    AUTH = "auth"
    COLLISION = "collision"
    FULL = "full"
    VERSION_MISMATCH = "version_mismatch"
    INVALID = "invalid"


class Namespace(IntEnum):
    FACTORY = 1
    PERSONAL = 2


@dataclass(frozen=True)
class Handle:
    namespace: Namespace
    version: int
    slot: int

    def encode(self) -> int:
        if not (0 <= self.version <= 0xFF and 0 < self.slot <= 0xFFFF):
            raise ValueError("handle field outside the 32-bit contract")
        return (int(self.namespace) << 24) | (self.version << 16) | self.slot


@dataclass(frozen=True)
class ResolveResult:
    status: RegistryStatus
    handle: Handle | None = None


def canonicalize(text: str) -> str:
    """Canonicalize only ASCII case, matching the C compiler contract."""
    if not isinstance(text, str) or not text:
        return ""
    out = []
    for char in text:
        code = ord(char)
        out.append(chr(code + 32) if 65 <= code <= 90 else char)
    return "".join(out)


def symbol_hash16(text: str) -> int:
    """Replicate the current compiler's folded 16-bit hash for probes."""
    value = _HASH_OFFSET
    for byte in canonicalize(text).encode("utf-8"):
        value ^= byte
        value = (value * _HASH_PRIME) & 0xFFFFFFFF
    folded = ((value >> 16) ^ (value & 0xFFFF)) & 0xFFFF
    return folded or 1


class FactoryLexicon:
    """Static namespace with exact membership and versioned dense handles."""

    def __init__(self, version: int, keys: Iterable[str]):
        if not (0 <= version <= 0xFF):
            raise ValueError("factory version must fit the handle contract")
        canonical_keys = [canonicalize(key) for key in keys]
        if any(not key for key in canonical_keys):
            raise ValueError("empty factory key")
        if len(set(canonical_keys)) != len(canonical_keys):
            raise ValueError("duplicate canonical factory key")
        if len(canonical_keys) > 0xFFFF:
            raise ValueError("factory lexicon exceeds handle slot range")
        self.version = version
        self._slot_by_key: Dict[str, int] = {
            key: slot for slot, key in enumerate(canonical_keys, start=1)
        }
        self._hash_buckets: Dict[int, List[str]] = {}
        for key in canonical_keys:
            self._hash_buckets.setdefault(symbol_hash16(key), []).append(key)

    def resolve(self, text: str) -> ResolveResult:
        key = canonicalize(text)
        slot = self._slot_by_key.get(key)
        if slot is None:
            return ResolveResult(RegistryStatus.UNKNOWN)
        return ResolveResult(
            RegistryStatus.OK, Handle(Namespace.FACTORY, self.version, slot)
        )

    def hash_bucket_size(self, text: str) -> int:
        return len(self._hash_buckets.get(symbol_hash16(text), []))

    def export_manifest(self) -> dict:
        """Export only audit metadata; never export factory key text."""
        return {
            "namespace": int(Namespace.FACTORY),
            "version": self.version,
            "count": len(self._slot_by_key),
        }


class PersonalInterner:
    """Bounded exact interner with private key storage and versioned handles."""

    def __init__(self, version: int, capacity: int):
        if not (0 <= version <= 0xFF):
            raise ValueError("personal version must fit the handle contract")
        if not (0 <= capacity <= 0xFFFF):
            raise ValueError("personal capacity must fit the handle contract")
        self.version = version
        self.capacity = capacity
        self._slot_by_key: Dict[str, int] = {}
        self._key_by_slot: Dict[int, str] = {}
        self._hash_buckets: Dict[int, List[int]] = {}

    def resolve(self, text: str, authorized: bool = False) -> ResolveResult:
        key = canonicalize(text)
        if not key:
            return ResolveResult(RegistryStatus.INVALID)
        old_slot = self._slot_by_key.get(key)
        if old_slot is not None:
            return ResolveResult(
                RegistryStatus.OK, Handle(Namespace.PERSONAL, self.version, old_slot)
            )
        if not authorized:
            return ResolveResult(RegistryStatus.AUTH)
        if len(self._slot_by_key) >= self.capacity:
            return ResolveResult(RegistryStatus.FULL)
        slot = len(self._slot_by_key) + 1
        self._slot_by_key[key] = slot
        self._key_by_slot[slot] = key
        self._hash_buckets.setdefault(symbol_hash16(key), []).append(slot)
        return ResolveResult(
            RegistryStatus.OK, Handle(Namespace.PERSONAL, self.version, slot)
        )

    def accept_handle(self, handle: Handle) -> RegistryStatus:
        if handle.namespace != Namespace.PERSONAL:
            return RegistryStatus.INVALID
        if handle.version != self.version:
            return RegistryStatus.VERSION_MISMATCH
        if handle.slot not in self._key_by_slot:
            return RegistryStatus.UNKNOWN
        return RegistryStatus.OK

    def migrate(self, new_version: int) -> Tuple["PersonalInterner", Dict[int, int]]:
        """Create a new version and an explicit old→new handle migration map."""
        migrated = PersonalInterner(new_version, self.capacity)
        mapping: Dict[int, int] = {}
        for old_slot in sorted(self._key_by_slot):
            key = self._key_by_slot[old_slot]
            result = migrated.resolve(key, authorized=True)
            if result.status is not RegistryStatus.OK or result.handle is None:
                raise RuntimeError("internal migration capacity failure")
            old_handle = Handle(Namespace.PERSONAL, self.version, old_slot)
            mapping[old_handle.encode()] = result.handle.encode()
        return migrated, mapping

    def export_records(self) -> List[int]:
        """Export handles only; private key text never crosses the boundary."""
        return [
            Handle(Namespace.PERSONAL, self.version, slot).encode()
            for slot in sorted(self._key_by_slot)
        ]

    def private_count(self) -> int:
        return len(self._slot_by_key)
