#!/usr/bin/env python3
"""Bounded candidate discovery for critical effects in host C11 sources.

This is not universal effect inference. It scans a finite verb family inside
function bodies under firmware/core and firmware/net. Every candidate must be
HCAE-profiled or explicitly reviewed; otherwise the result is REVIEW_REQUIRED.
"""
from __future__ import annotations

from dataclasses import dataclass
import argparse
import json
from pathlib import Path
import re
from typing import Any

SENSITIVE_VERBS = ("send", "store", "commit", "erase", "seal", "publish", "transmit")
CONTROL_WORDS = {"if", "for", "while", "switch", "return", "sizeof"}
FUNCTION_RE = re.compile(
    r"(?m)^[ \t]*(?:static[ \t]+)?[A-Za-z_][A-Za-z0-9_ \t*]*?\b"
    r"(?P<name>[A-Za-z_][A-Za-z0-9_]*)[ \t]*\([^;{}]*\)[ \t\r\n]*\{"
)
CALL_RE = re.compile(r"(?<![A-Za-z0-9_])(?P<name>[A-Za-z_][A-Za-z0-9_]*)[ \t]*\(")


@dataclass(frozen=True)
class CandidateResult:
    source: str
    function: str
    operation: str
    status: str
    detail: str


def load_json(path: Path) -> dict[str, Any]:
    value = json.loads(path.read_text(encoding="utf-8"))
    if not isinstance(value, dict):
        raise ValueError(f"object required: {path}")
    return value


def _function_bodies(source: str) -> tuple[tuple[str, str], ...]:
    results: list[tuple[str, str]] = []
    for match in FUNCTION_RE.finditer(source):
        name = match.group("name")
        if name in CONTROL_WORDS:
            continue
        start = match.end() - 1
        depth = 0
        end = None
        for index in range(start, len(source)):
            if source[index] == "{":
                depth += 1
            elif source[index] == "}":
                depth -= 1
                if depth == 0:
                    end = index + 1
                    break
        if end is not None:
            results.append((name, source[start:end]))
    return tuple(results)


def _profiled(profile: dict[str, Any]) -> set[tuple[str, str, str]]:
    results: set[tuple[str, str, str]] = set()
    sinks = profile.get("critical_sinks", {})
    if not isinstance(sinks, dict):
        return results
    for entry in sinks.values():
        if not isinstance(entry, dict):
            continue
        source, function, operation = entry.get("source"), entry.get("function"), entry.get("operation")
        if all(isinstance(value, str) for value in (source, function, operation)):
            results.add((source, function, operation[:-1] if operation.endswith("(") else operation))
    return results


def _reviewed(dispositions: dict[str, Any]) -> dict[tuple[str, str, str], str]:
    results: dict[tuple[str, str, str], str] = {}
    if dispositions.get("schema") != "herus.critical-effect-dispositions.v1":
        return results
    for item in dispositions.get("reviewed_internal", []):
        if not isinstance(item, dict):
            continue
        key = (item.get("source"), item.get("function"), item.get("operation"))
        if all(isinstance(value, str) and value for value in key) and isinstance(item.get("reason"), str):
            results[key] = item["reason"]
    return results


def audit(profile: dict[str, Any], dispositions: dict[str, Any], root: Path) -> tuple[CandidateResult, ...]:
    profiled = _profiled(profile)
    reviewed = _reviewed(dispositions)
    results: list[CandidateResult] = []
    for directory in (root / "firmware/core", root / "firmware/net"):
        for path in sorted(directory.glob("*.c")):
            if path.name.startswith("test_"):
                continue
            relative = str(path.relative_to(root))
            source = path.read_text(encoding="utf-8")
            for function, body in _function_bodies(source):
                seen: set[str] = set()
                for match in CALL_RE.finditer(body):
                    operation = match.group("name")
                    if operation in CONTROL_WORDS or operation in seen:
                        continue
                    if not any(verb in operation.lower() for verb in SENSITIVE_VERBS):
                        continue
                    seen.add(operation)
                    key = (relative, function, operation)
                    if key in profiled:
                        results.append(CandidateResult(relative, function, operation, "PROFILED", "hcae_critical_sink"))
                    elif key in reviewed:
                        results.append(CandidateResult(relative, function, operation, "REVIEWED_INTERNAL", reviewed[key]))
                    else:
                        results.append(CandidateResult(relative, function, operation, "REVIEW_REQUIRED", "sensitive_verb_call_without_disposition"))
    return tuple(sorted(results, key=lambda item: (item.source, item.function, item.operation)))


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("profile", type=Path)
    parser.add_argument("dispositions", type=Path)
    parser.add_argument("--root", type=Path, default=Path(__file__).parents[1])
    args = parser.parse_args()
    results = audit(load_json(args.profile), load_json(args.dispositions), args.root)
    for item in results:
        print(item.status, item.source, item.function, item.operation, item.detail)
    blocked = not results or any(item.status == "REVIEW_REQUIRED" for item in results)
    print("CRITICAL_EFFECT_CANDIDATES=" + ("BLOCKED" if blocked else "PASS"))
    return 1 if blocked else 0


if __name__ == "__main__":
    raise SystemExit(main())
