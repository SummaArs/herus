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
    "store_active(",
    "memory_vault_seal(",
    "memory_vault_erase(",
    "erase_sealed(",
    "erase(",
    "commit_record(",
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
    sinks = profile.get("critical_sinks", {})
    declared_ids = set(sinks) if isinstance(sinks, dict) else set()
    results: list[InventoryResult] = []
    registry_name = profile.get("effect_registry")
    if not isinstance(registry_name, str) or not registry_name:
        return (InventoryResult("<registry>", "", "UNPROFILED", "missing_effect_registry"),)
    registry_path = root / registry_name
    try:
        registry = json.loads(registry_path.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError):
        return (InventoryResult("<registry>", registry_name, "UNPROFILED", "invalid_effect_registry"),)
    classes = registry.get("classes") if isinstance(registry, dict) and registry.get("schema") == "herus.critical-effects.v1" else None
    if not isinstance(classes, dict) or not classes:
        return (InventoryResult("<registry>", registry_name, "UNPROFILED", "invalid_effect_registry_schema"),)
    for source in sorted({str(p.relative_to(root)) for p in (root / "firmware").rglob("*.c") if "test_" not in p.name}):
        text = (root / source).read_text(encoding="utf-8")
        for annotation in re.finditer(
            r"HERUS_CRITICAL_SINK:\s*(?P<id>[A-Za-z0-9_.-]+)\s+class=(?P<class>[A-Za-z0-9_.-]+)\s+operation=(?P<operation>[A-Za-z0-9_]+\()",
            text,
        ):
            sink_id = annotation.group("id")
            effect_class = annotation.group("class")
            operation = annotation.group("operation")
            entry = sinks.get(sink_id) if isinstance(sinks, dict) else None
            registry_entry = classes.get(effect_class)
            matches = (
                sink_id in declared_ids and isinstance(entry, dict)
                and entry.get("source") == source and entry.get("operation") == operation
                and entry.get("effect_class") == effect_class
                and isinstance(registry_entry, dict) and registry_entry.get("operation") == operation
            )
            if not matches:
                results.append(InventoryResult(operation, source, "UNPROFILED", "critical_annotation_profile_registry_mismatch"))
            else:
                results.append(InventoryResult(operation, source, "PROFILED", "critical_annotation_matches_profile_and_registry"))
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
