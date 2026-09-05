"""Bounded, fail-closed inventory for public SyGuS benchmarks.

This is an inventory tool, not a SyGuS solver. It never treats an unsupported
benchmark as solved and never executes generated text.
"""
from __future__ import annotations

from dataclasses import dataclass
from pathlib import Path
import re


@dataclass(frozen=True)
class InventoryRow:
    path: str
    logic: str | None
    synth_functions: int
    constraints: int
    supported: bool
    reasons: tuple[str, ...]


_SUPPORTED_LOGICS = {"LIA"}
_UNSUPPORTED_MARKERS = ("Array", "BitVec", "String", "str.", "ite", "forall", "exists", "define-fun-rec")


def inspect_file(path: Path) -> InventoryRow:
    text = path.read_text(encoding="utf-8", errors="strict")
    logic_match = re.search(r"\(set-logic\s+([^\s\)]+)\)", text)
    logic = logic_match.group(1) if logic_match else None
    synth_functions = len(re.findall(r"\(synth-fun\s+", text))
    constraints = len(re.findall(r"\(constraint\s+", text))
    reasons: list[str] = []
    if logic not in _SUPPORTED_LOGICS:
        reasons.append("unsupported_or_missing_logic")
    if synth_functions != 1:
        reasons.append("requires_single_synth_fun")
    if constraints == 0:
        reasons.append("missing_constraint")
    for marker in _UNSUPPORTED_MARKERS:
        if marker in text:
            reasons.append(f"unsupported_marker:{marker}")
    return InventoryRow(str(path), logic, synth_functions, constraints, not reasons, tuple(sorted(set(reasons))))


def inventory(root: Path, limit: int = 64) -> tuple[InventoryRow, ...]:
    if limit <= 0:
        return ()
    if not root.is_dir():
        return (InventoryRow(str(root), None, 0, 0, False, ("missing_root",)),)
    rows: list[InventoryRow] = []
    for path in sorted(root.rglob("*.sl"))[:limit]:
        rows.append(inspect_file(path))
    return tuple(rows)


def summarize(rows: tuple[InventoryRow, ...]) -> dict[str, int]:
    return {
        "files": len(rows),
        "supported": sum(row.supported for row in rows),
        "rejected": sum(not row.supported for row in rows),
    }


if __name__ == "__main__":
    import argparse

    parser = argparse.ArgumentParser()
    parser.add_argument("root", type=Path)
    parser.add_argument("--limit", type=int, default=64)
    args = parser.parse_args()
    rows = inventory(args.root, args.limit)
    print(summarize(rows))
    for row in rows:
        print(row)
