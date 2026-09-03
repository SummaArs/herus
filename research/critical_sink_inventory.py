"""Fail-closed inventory check for known critical operations in firmware C11."""
from __future__ import annotations

from dataclasses import dataclass
import json
from pathlib import Path
import re
from typing import Any


@dataclass(frozen=True)
class InventoryResult:
    operation: str
    source: str
    status: str
    detail: str


# This list is intentionally explicit: it is an inventory boundary, not a guesser.
KNOWN_OPERATIONS = (
    "interaction_take_send(",
    "store_sealed(",
    "memory_vault_seal(",
    "core_link_seal_nucleus_intent(",
)


def _profile_operations(profile: dict[str, Any]) -> set[tuple[str, str]]:
    sinks = profile.get("critical_sinks")
    if not isinstance(sinks, dict):
        return set()
    return {
        (entry.get("source", ""), entry.get("operation", ""))
        for entry in sinks.values()
        if isinstance(entry, dict)
    }


def inventory(profile: dict[str, Any], root: Path) -> tuple[InventoryResult, ...]:
    declared = _profile_operations(profile)
    results: list[InventoryResult] = []
    for source in sorted({str(p.relative_to(root)) for p in (root / "firmware").rglob("*.c") if "test_" not in p.name}):
        text = (root / source).read_text(encoding="utf-8")
        for operation in KNOWN_OPERATIONS:
            # Ignore the function definition itself; calls are the inventory target.
            matches = []
            for match in re.finditer(r"(?<![A-Za-z0-9_])" + re.escape(operation), text):
                line_start = text.rfind("\n", 0, match.start()) + 1
                line_end = text.find("\n", match.start())
                line = text[line_start:line_end if line_end >= 0 else len(text)]
                # A declaration such as `int sink(...)` is not an invocation.
                if re.match(r"\s*(?:static\s+)?[A-Za-z_][A-Za-z0-9_\s*]*" + re.escape(operation), line):
                    continue
                matches.append(match.start())
            if not matches:
                continue
            if (source, operation) not in declared:
                results.append(InventoryResult(operation, source, "UNPROFILED", "known_operation_missing_from_hcae_profile"))
            else:
                results.append(InventoryResult(operation, source, "PROFILED", "known_operation_declared"))
    return tuple(results)


def load_profile(path: Path) -> dict[str, Any]:
    value = json.loads(path.read_text(encoding="utf-8"))
    if not isinstance(value, dict) or value.get("schema") != "hcae.profile.v1":
        raise ValueError("invalid_profile_schema")
    return value


def main(argv: list[str] | None = None) -> int:
    import argparse
    parser = argparse.ArgumentParser(prog="critical-sink-inventory")
    parser.add_argument("profile", type=Path)
    parser.add_argument("--root", type=Path, default=Path("."))
    args = parser.parse_args(argv)
    results = inventory(load_profile(args.profile), args.root.resolve())
    for result in results:
        print(f"{result.status} {result.source} {result.operation} {result.detail}")
    if not results or any(result.status != "PROFILED" for result in results):
        print("SINK_INVENTORY=BLOCKED")
        return 1
    print("SINK_INVENTORY=PASS")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
