"""Fail-closed lexical audit of declared critical sinks.

This is a conservative inventory gate, not a proof of control-flow dominance.
It reports UNKNOWN when a source/function/operation cannot be located. A sink is
COVERED only when every declared guard token occurs before the declared sink
operation inside the extracted function body. AST/interprocedural proof remains
future work.
"""
from __future__ import annotations

from dataclasses import dataclass
import json
from pathlib import Path
import re
from typing import Any


@dataclass(frozen=True)
class SinkResult:
    sink_id: str
    status: str
    source: str
    function: str
    detail: str


def _function_body(text: str, function: str) -> str | None:
    match = re.search(r"\b" + re.escape(function) + r"\s*\([^;{}]*\)\s*\{", text)
    if not match:
        return None
    opening = text.find("{", match.start(), match.end())
    depth = 0
    for index in range(opening, len(text)):
        if text[index] == "{":
            depth += 1
        elif text[index] == "}":
            depth -= 1
            if depth == 0:
                return text[opening + 1:index]
    return None


def _valid_entry(entry: Any) -> bool:
    semantic_guards = entry.get("semantic_guards", []) if isinstance(entry, dict) else []
    valid_semantic = (
        isinstance(semantic_guards, list)
        and all(
            isinstance(item, dict)
            and item.get("kind") == "rejects_mismatch"
            and isinstance(item.get("expression"), str)
            and bool(item["expression"])
            for item in semantic_guards
        )
    )
    return (
        isinstance(entry, dict)
        and isinstance(entry.get("source"), str)
        and isinstance(entry.get("function"), str)
        and isinstance(entry.get("operation"), str)
        and isinstance(entry.get("guards"), list)
        and bool(entry["guards"])
        and all(isinstance(item, str) and item for item in entry["guards"])
        and valid_semantic
    )


def audit(profile: dict[str, Any], root: Path) -> tuple[SinkResult, ...]:
    sinks = profile.get("critical_sinks")
    if not isinstance(sinks, dict) or not sinks:
        return (SinkResult("<profile>", "UNKNOWN", "", "", "missing_critical_sinks"),)
    results: list[SinkResult] = []
    for sink_id in sorted(sinks):
        entry = sinks[sink_id]
        if not _valid_entry(entry):
            results.append(SinkResult(sink_id, "UNKNOWN", "", "", "invalid_sink_declaration"))
            continue
        source = entry["source"]
        path = root / source
        if not path.is_file():
            results.append(SinkResult(sink_id, "UNKNOWN", source, entry["function"], "source_missing"))
            continue
        body = _function_body(path.read_text(encoding="utf-8"), entry["function"])
        if body is None:
            results.append(SinkResult(sink_id, "UNKNOWN", source, entry["function"], "function_missing_or_unbalanced"))
            continue
        operation_position = body.find(entry["operation"])
        if operation_position < 0:
            results.append(SinkResult(sink_id, "UNKNOWN", source, entry["function"], "operation_missing"))
            continue
        missing = [guard for guard in entry["guards"] if body.find(guard) < 0]
        late = [guard for guard in entry["guards"] if 0 <= body.find(guard) > operation_position]
        semantic_missing = [
            item["expression"] for item in entry.get("semantic_guards", [])
            if body.find(item["expression"]) < 0
        ]
        semantic_late = [
            item["expression"] for item in entry.get("semantic_guards", [])
            if 0 <= body.find(item["expression"]) > operation_position
        ]
        if missing or late or semantic_missing or semantic_late:
            if missing:
                detail = "missing_guard:" + ",".join(missing)
            elif late:
                detail = "guard_after_operation:" + ",".join(late)
            elif semantic_missing:
                detail = "missing_semantic_guard:" + ",".join(semantic_missing)
            else:
                detail = "semantic_guard_after_operation:" + ",".join(semantic_late)
            results.append(SinkResult(sink_id, "UNCOVERED", source, entry["function"], detail))
            continue
        results.append(SinkResult(sink_id, "COVERED", source, entry["function"], "lexical_guard_before_operation"))
    return tuple(results)


def load_profile(path: Path) -> dict[str, Any]:
    value = json.loads(path.read_text(encoding="utf-8"))
    if not isinstance(value, dict) or value.get("schema") != "hcae.profile.v1":
        raise ValueError("invalid_profile_schema")
    return value


def main(argv: list[str] | None = None) -> int:
    import argparse

    parser = argparse.ArgumentParser(prog="critical-sink-audit")
    parser.add_argument("profile", type=Path)
    parser.add_argument("--root", type=Path, default=Path("."))
    args = parser.parse_args(argv)
    try:
        results = audit(load_profile(args.profile), args.root.resolve())
    except (OSError, ValueError, json.JSONDecodeError) as exc:
        print(f"UNKNOWN profile:{exc}")
        return 2
    for result in results:
        print(f"{result.status} {result.sink_id} {result.source}:{result.function} {result.detail}")
    if all(result.status == "COVERED" for result in results):
        print("SINK_AUDIT=PASS")
        return 0
    print("SINK_AUDIT=BLOCKED")
    return 1


if __name__ == "__main__":
    raise SystemExit(main())
