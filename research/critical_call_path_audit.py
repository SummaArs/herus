"""Conservative call-path audit for critical firmware handoffs.

This is not a C parser or a proof of control-flow dominance. It extracts balanced
function bodies from the restricted firmware subset and checks whether direct calls
to protected operations originate only in declared wrappers. Unsupported syntax,
unbalanced bodies and invalid profiles return UNKNOWN.
"""
from __future__ import annotations

from dataclasses import dataclass
import json
from pathlib import Path
import re
from typing import Any


@dataclass(frozen=True)
class CallPathResult:
    rule_id: str
    status: str
    source: str
    caller: str
    detail: str


_FUNCTION_RE = re.compile(
    r"(?m)^[\t ]*(?:static[\t ]+)?(?:const[\t ]+)?[A-Za-z_][\w\t ]*\s+"
    r"([A-Za-z_]\w*)\s*\([^;{}]*\)\s*\{"
)


def _function_bodies(text: str) -> dict[str, str] | None:
    bodies: dict[str, str] = {}
    for match in _FUNCTION_RE.finditer(text):
        opening = text.find("{", match.start(), match.end())
        depth = 0
        closing = None
        for index in range(opening, len(text)):
            if text[index] == "{":
                depth += 1
            elif text[index] == "}":
                depth -= 1
                if depth == 0:
                    closing = index
                    break
        if closing is None:
            return None
        bodies[match.group(1)] = text[opening + 1:closing]
    return bodies


def _valid_rule(rule: Any) -> bool:
    return (
        isinstance(rule, dict)
        and isinstance(rule.get("source"), str)
        and isinstance(rule.get("callee"), str)
        and isinstance(rule.get("allowed_callers"), list)
        and all(isinstance(item, str) and item for item in rule["allowed_callers"])
        and isinstance(rule.get("required_guards", []), list)
        and all(isinstance(item, str) and item for item in rule.get("required_guards", []))
    )


def audit(profile: dict[str, Any], root: Path) -> tuple[CallPathResult, ...]:
    rules = profile.get("protected_calls")
    if not isinstance(rules, dict) or not rules:
        return (CallPathResult("<profile>", "UNKNOWN", "", "", "missing_protected_calls"),)
    results: list[CallPathResult] = []
    for rule_id in sorted(rules):
        rule = rules[rule_id]
        if not _valid_rule(rule):
            results.append(CallPathResult(rule_id, "UNKNOWN", "", "", "invalid_rule"))
            continue
        source = rule["source"]
        path = root / source
        if not path.is_file():
            results.append(CallPathResult(rule_id, "UNKNOWN", source, "", "source_missing"))
            continue
        bodies = _function_bodies(path.read_text(encoding="utf-8"))
        if bodies is None:
            results.append(CallPathResult(rule_id, "UNKNOWN", source, "", "function_unbalanced"))
            continue
        callee = re.compile(r"\b" + re.escape(rule["callee"]) + r"\s*\(")
        calls = [(caller, body) for caller, body in bodies.items() if callee.search(body)]
        forbidden = [caller for caller, _ in calls if caller not in rule["allowed_callers"]]
        missing_allowed = [caller for caller in rule["allowed_callers"] if caller not in bodies]
        if missing_allowed:
            results.append(CallPathResult(rule_id, "UNKNOWN", source, "", "allowed_caller_missing:" + ",".join(missing_allowed)))
        elif not calls:
            results.append(CallPathResult(rule_id, "UNKNOWN", source, "", "no_protected_call_found"))
        elif forbidden:
            results.append(CallPathResult(rule_id, "UNCOVERED", source, forbidden[0], "direct_call_outside_wrapper"))
        else:
            guard_names = rule.get("required_guards", [])
            guard_failures = []
            sink_pattern = re.compile(r"\b" + re.escape(rule["callee"]) + r"\s*\(")
            for caller, body in calls:
                sink = sink_pattern.search(body)
                if sink is None:
                    continue
                for guard in guard_names:
                    guard_match = re.search(r"\b" + re.escape(guard) + r"\s*\(", body)
                    if guard_match is None:
                        guard_failures.append((caller, "guard_missing:" + guard))
                    elif guard_match.start() > sink.start():
                        guard_failures.append((caller, "guard_after_sink:" + guard))
            if guard_failures:
                caller, detail = guard_failures[0]
                results.append(CallPathResult(rule_id, "UNCOVERED", source, caller, detail))
            else:
                results.append(CallPathResult(rule_id, "COVERED", source, ",".join(sorted(caller for caller, _ in calls)), "direct_calls_restricted_to_declared_wrapper_and_guards_before_sink"))
    return tuple(results)


def load_profile(path: Path) -> dict[str, Any]:
    value = json.loads(path.read_text(encoding="utf-8"))
    if not isinstance(value, dict) or value.get("schema") != "hcae.call-path.v1":
        raise ValueError("invalid_call_path_profile_schema")
    return value


def main(argv: list[str] | None = None) -> int:
    import argparse
    parser = argparse.ArgumentParser(prog="critical-call-path-audit")
    parser.add_argument("profile", type=Path)
    parser.add_argument("--root", type=Path, default=Path("."))
    args = parser.parse_args(argv)
    try:
        results = audit(load_profile(args.profile), args.root.resolve())
    except (OSError, ValueError, json.JSONDecodeError) as exc:
        print(f"UNKNOWN profile:{exc}")
        return 2
    for result in results:
        print(f"{result.status} {result.rule_id} {result.source}:{result.caller} {result.detail}")
    if all(result.status == "COVERED" for result in results):
        print("CALL_PATH_AUDIT=PASS")
        return 0
    print("CALL_PATH_AUDIT=BLOCKED")
    return 1


if __name__ == "__main__":
    raise SystemExit(main())
