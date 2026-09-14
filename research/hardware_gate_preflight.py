"""Host-only preflight for the first HERUS physical host."""
from __future__ import annotations

from dataclasses import dataclass
from typing import Mapping


REQUIRED = ("board_revision", "radio_variant", "schematic_reference", "selftest_result")


@dataclass(frozen=True)
class Preflight:
    status: str
    reason: str
    missing: tuple[str, ...]


def evaluate(record: Mapping[str, object]) -> Preflight:
    missing = tuple(key for key in REQUIRED if not record.get(key) or record.get(key) == "pending")
    if missing:
        return Preflight("BLOCKED", "physical_identity_incomplete", missing)
    if record.get("selftest_result") != "pass":
        return Preflight("BLOCKED", "selftest_not_passed", ())
    if record.get("radio_variant") not in {"SX1262-915"}:
        return Preflight("BLOCKED", "radio_variant_not_frozen", ())
    return Preflight("READY_FOR_BENCH", "identity_and_selftest_present", ())
