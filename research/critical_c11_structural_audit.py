"""Conservative Clang-AST audit for authority guards dominating critical sinks.

The accepted subset is deliberately small: direct calls, declarations, compound
statements, rejecting ``if`` statements and returns in a top-level function
body. Unsupported control flow, indirect calls and opaque macro expansions do
not promote evidence; they return UNKNOWN.
"""
from __future__ import annotations

from dataclasses import dataclass
import json
from pathlib import Path
import re
import shutil
import subprocess
from typing import Any


@dataclass(frozen=True)
class StructuralSinkResult:
    sink_id: str
    status: str
    source: str
    function: str
    detail: str


def _walk(node: Any):
    if isinstance(node, dict):
        yield node
        for child in node.get("inner", []):
            yield from _walk(child)
    elif isinstance(node, list):
        for child in node:
            yield from _walk(child)


def _call_name(node: dict[str, Any]) -> str | None:
    if node.get("kind") != "CallExpr":
        return None
    for child in _walk(node.get("inner", [])):
        if child.get("kind") == "DeclRefExpr":
            referenced = child.get("referencedDecl")
            if isinstance(referenced, dict) and referenced.get("kind") == "FunctionDecl":
                name = referenced.get("name")
                return name if isinstance(name, str) else None
    for child in _walk(node.get("inner", [])[:1]):
        if child.get("kind") == "MemberExpr" and isinstance(child.get("name"), str):
            return child["name"]
    return None


def _call_kind(node: dict[str, Any]) -> str:
    for child in _walk(node.get("inner", [])[:1]):
        if child.get("kind") == "MemberExpr":
            return "member-function-pointer"
        if child.get("kind") == "DeclRefExpr" and isinstance(child.get("referencedDecl"), dict):
            if child["referencedDecl"].get("kind") == "FunctionDecl":
                return "direct"
    return "indirect"


def _calls(node: dict[str, Any]) -> tuple[tuple[str | None, dict[str, Any]], ...]:
    return tuple((_call_name(item), item) for item in _walk(node) if item.get("kind") == "CallExpr")


def _decl_refs(node: dict[str, Any]) -> set[str]:
    result: set[str] = set()
    for item in _walk(node):
        if item.get("kind") == "DeclRefExpr":
            referenced = item.get("referencedDecl")
            if isinstance(referenced, dict) and referenced.get("kind") in {"VarDecl", "ParmVarDecl"}:
                name = referenced.get("name")
                if isinstance(name, str):
                    result.add(name)
    return result


def _macro_expanded(node: dict[str, Any]) -> bool:
    for item in _walk(node):
        for position in (item.get("loc"), item.get("range", {}).get("begin"), item.get("range", {}).get("end")):
            if isinstance(position, dict) and ("expansionLoc" in position or "spellingLoc" in position):
                return True
    return False


def _source_location(location: dict[str, Any]) -> dict[str, Any]:
    if isinstance(location.get("expansionLoc"), dict):
        return location["expansionLoc"]
    if "offset" in location:
        return location
    if isinstance(location.get("spellingLoc"), dict):
        return location["spellingLoc"]
    return location


def _source_fragment(node: dict[str, Any], source: str) -> str | None:
    begin = _source_location(node.get("range", {}).get("begin", {}))
    end = _source_location(node.get("range", {}).get("end", {}))
    if "offset" not in begin or "offset" not in end:
        return None
    start = begin["offset"]
    stop = end["offset"] + end.get("tokLen", 1)
    encoded = source.encode("utf-8")
    if not isinstance(start, int) or not isinstance(stop, int) or start < 0 or stop > len(encoded):
        return None
    try:
        return encoded[start:stop].decode("utf-8")
    except UnicodeDecodeError:
        return None


def _normalized(text: str) -> str:
    return re.sub(r"\s+", "", text).strip("()")


def _always_returns(node: dict[str, Any]) -> bool:
    kind = node.get("kind")
    if kind == "ReturnStmt":
        return True
    inner = [item for item in node.get("inner", []) if isinstance(item, dict)]
    if kind == "CompoundStmt":
        return bool(inner) and _always_returns(inner[-1])
    if kind == "IfStmt" and len(inner) >= 3:
        return _always_returns(inner[-2]) and _always_returns(inner[-1])
    return False


def _unary_not_target(condition: dict[str, Any], call: str | None, variable: str | None) -> bool:
    for node in _walk(condition):
        if node.get("kind") != "UnaryOperator" or node.get("opcode") != "!":
            continue
        if call is not None and any(name == call for name, _ in _calls(node)):
            return True
        if variable is not None and variable in _decl_refs(node):
            return True
    return False


def _nonzero_target(condition: dict[str, Any], call: str, variable: str | None, source: str) -> bool:
    if variable is None:
        return False
    fragment = _source_fragment(condition, source)
    return fragment is not None and _normalized(fragment) == variable


def _binary_not_equal(condition: dict[str, Any], call: str, expected: str, source: str) -> bool:
    for node in _walk(condition):
        if node.get("kind") != "BinaryOperator" or node.get("opcode") != "!=":
            continue
        if not any(name == call for name, _ in _calls(node)):
            continue
        fragment = _source_fragment(node, source)
        if fragment is not None and expected in fragment:
            return True
    return False


def _condition_rejects(
    condition: dict[str, Any], guard: dict[str, Any], bindings: dict[str, str], source: str
) -> bool:
    mode = guard.get("reject_if")
    call = guard.get("call")
    if mode == "falsy" and isinstance(call, str):
        variable = next((name for name, bound_call in bindings.items() if bound_call == call), None)
        if not _unary_not_target(condition, call, variable):
            return False
        expected = guard.get("expected_argument")
        fragment = _source_fragment(condition, source)
        return expected is None or (isinstance(expected, str) and fragment is not None and expected in fragment)
    if mode == "nonzero" and isinstance(call, str):
        variable = next((name for name, bound_call in bindings.items() if bound_call == call), None)
        return _nonzero_target(condition, call, variable, source)
    if mode == "not_equal" and isinstance(call, str) and isinstance(guard.get("expected"), str):
        return _binary_not_equal(condition, call, guard["expected"], source)
    if mode == "truthy_expression" and isinstance(guard.get("expression"), str):
        expected = _normalized(guard["expression"])
        return any(
            (fragment := _source_fragment(node, source)) is not None and _normalized(fragment) == expected
            for node in _walk(condition)
        )
    return False


def _function_body(ast: dict[str, Any], function: str) -> dict[str, Any] | None:
    for node in _walk(ast):
        if node.get("kind") != "FunctionDecl" or node.get("name") != function:
            continue
        for child in node.get("inner", []):
            if isinstance(child, dict) and child.get("kind") == "CompoundStmt":
                return child
    return None


def _clang_ast(source_path: Path, root: Path, clang: str) -> tuple[dict[str, Any] | None, str | None]:
    executable = shutil.which(clang)
    if executable is None:
        return None, "clang_unavailable"
    include_dirs = sorted({path.parent for path in (root / "firmware").rglob("*.h")})
    command = [executable, "-std=c11", "-Wno-everything"]
    for directory in include_dirs:
        command.extend(["-I", str(directory)])
    command.extend(["-Xclang", "-ast-dump=json", "-fsyntax-only", str(source_path)])
    completed = subprocess.run(command, text=True, capture_output=True, check=False)
    if completed.returncode != 0:
        return None, "clang_parse_failed:" + completed.stderr.splitlines()[0][:160] if completed.stderr else "clang_parse_failed"
    try:
        return json.loads(completed.stdout), None
    except json.JSONDecodeError:
        return None, "clang_ast_invalid_json"


def _valid_structural_guards(value: Any) -> bool:
    if not isinstance(value, list) or not value:
        return False
    for item in value:
        if not isinstance(item, dict) or not isinstance(item.get("id"), str):
            return False
        mode = item.get("reject_if")
        if mode == "falsy" and isinstance(item.get("call"), str) and (
            item.get("expected_argument") is None or isinstance(item.get("expected_argument"), str)
        ):
            continue
        if mode == "nonzero" and isinstance(item.get("call"), str):
            continue
        if mode == "not_equal" and isinstance(item.get("call"), str) and isinstance(item.get("expected"), str):
            continue
        if mode == "truthy_expression" and isinstance(item.get("expression"), str):
            continue
        return False
    return True


def _audit_entry(
    sink_id: str, entry: dict[str, Any], root: Path, clang: str
) -> StructuralSinkResult:
    source = entry.get("source", "")
    function = entry.get("function", "")
    operation = entry.get("operation", "")
    guards = entry.get("structural_guards")
    declared_call_kind = entry.get("sink_call_kind", "direct")
    if declared_call_kind not in {"direct", "member-function-pointer"}:
        return StructuralSinkResult(sink_id, "UNKNOWN", source, function, "invalid_sink_call_kind")
    if not all(isinstance(item, str) and item for item in (source, function, operation)) or not _valid_structural_guards(guards):
        return StructuralSinkResult(sink_id, "UNKNOWN", source, function, "invalid_structural_declaration")
    path = root / source
    if not path.is_file():
        return StructuralSinkResult(sink_id, "UNKNOWN", source, function, "source_missing")
    ast, error = _clang_ast(path, root, clang)
    if ast is None:
        return StructuralSinkResult(sink_id, "UNKNOWN", source, function, error or "clang_ast_unavailable")
    body = _function_body(ast, function)
    if body is None:
        return StructuralSinkResult(sink_id, "UNKNOWN", source, function, "function_ast_missing")
    source_text = path.read_text(encoding="utf-8")
    sink_name = operation[:-1] if operation.endswith("(") else operation
    required = {item["id"]: item for item in guards}
    active: set[str] = set()
    bindings: dict[str, str] = {}
    protected_refs: set[str] = set()
    sink_count = 0
    unsupported = {"ForStmt", "WhileStmt", "DoStmt", "SwitchStmt", "GotoStmt", "IndirectGotoStmt"}

    for statement in body.get("inner", []):
        if not isinstance(statement, dict):
            continue
        if any(node.get("kind") in unsupported for node in _walk(statement)):
            loop_calls = {name for name, _ in _calls(statement) if name}
            loop_refs = _decl_refs(statement)
            loop_has_exit = any(node.get("kind") in {"ReturnStmt", "GotoStmt", "IndirectGotoStmt"} for node in _walk(statement))
            if (
                not entry.get("allow_local_loops_before_sink")
                or loop_calls
                or loop_has_exit
                or loop_refs.intersection(protected_refs)
            ):
                return StructuralSinkResult(sink_id, "UNKNOWN", source, function, "unsupported_control_flow")
            continue
        if statement.get("kind") == "DeclStmt":
            for declaration in statement.get("inner", []):
                if not isinstance(declaration, dict) or declaration.get("kind") != "VarDecl":
                    continue
                names = [name for name, _ in _calls(declaration) if name]
                if len(names) == 1 and isinstance(declaration.get("name"), str):
                    bindings[declaration["name"]] = names[0]
        if statement.get("kind") == "IfStmt":
            parts = [item for item in statement.get("inner", []) if isinstance(item, dict)]
            if len(parts) < 2:
                return StructuralSinkResult(sink_id, "UNKNOWN", source, function, "malformed_if_ast")
            condition, then_branch = parts[0], parts[1]
            else_branch = parts[2] if len(parts) > 2 else None
            condition_sink_calls = [(name, node) for name, node in _calls(condition) if name == sink_name]
            branch_sink_calls = [
                (name, node) for branch in (then_branch, else_branch) if branch is not None
                for name, node in _calls(branch) if name == sink_name
            ]
            if branch_sink_calls:
                return StructuralSinkResult(sink_id, "UNKNOWN", source, function, "sink_in_nested_control_flow")
            for _, call_node in condition_sink_calls:
                actual_call_kind = _call_kind(call_node)
                if actual_call_kind != declared_call_kind:
                    return StructuralSinkResult(sink_id, "UNKNOWN", source, function, "sink_call_kind_mismatch")
                if _macro_expanded(call_node):
                    return StructuralSinkResult(sink_id, "UNKNOWN", source, function, "macro_expanded_sink")
                sink_count += 1
                missing = sorted(set(required) - active)
                if missing:
                    return StructuralSinkResult(sink_id, "UNCOVERED", source, function, "non_dominating_guards:" + ",".join(missing))
            matched = [guard_id for guard_id, guard in required.items() if _condition_rejects(condition, guard, bindings, source_text)]
            relevant_calls = {guard.get("call") for guard in required.values() if isinstance(guard.get("call"), str)}
            condition_calls = {name for name, _ in _calls(condition)}
            relevant_vars = {name for name, call in bindings.items() if call in relevant_calls}
            if matched and _always_returns(then_branch) and else_branch is None:
                active.update(matched)
                protected_refs.update(_decl_refs(condition))
            elif condition_calls.intersection(relevant_calls) or _decl_refs(condition).intersection(relevant_vars):
                return StructuralSinkResult(sink_id, "UNCOVERED", source, function, "guard_rejection_semantics_not_proven")
            continue
        for call_name, call_node in _calls(statement):
            if call_name != sink_name:
                continue
            actual_call_kind = _call_kind(call_node)
            if actual_call_kind != declared_call_kind:
                return StructuralSinkResult(sink_id, "UNKNOWN", source, function, "sink_call_kind_mismatch")
            if _macro_expanded(call_node):
                return StructuralSinkResult(sink_id, "UNKNOWN", source, function, "macro_expanded_sink")
            sink_count += 1
            missing = sorted(set(required) - active)
            if missing:
                return StructuralSinkResult(sink_id, "UNCOVERED", source, function, "non_dominating_guards:" + ",".join(missing))
    if sink_count == 0:
        return StructuralSinkResult(sink_id, "UNKNOWN", source, function, "direct_sink_call_not_found")
    return StructuralSinkResult(sink_id, "STRUCTURALLY_COVERED", source, function, "all_supported_paths_reject_before_sink")


def audit(profile: dict[str, Any], root: Path, clang: str = "clang") -> tuple[StructuralSinkResult, ...]:
    sinks = profile.get("critical_sinks")
    if not isinstance(sinks, dict) or not sinks:
        return (StructuralSinkResult("<profile>", "UNKNOWN", "", "", "missing_critical_sinks"),)
    return tuple(_audit_entry(sink_id, sinks[sink_id], root, clang) for sink_id in sorted(sinks))


def load_profile(path: Path) -> dict[str, Any]:
    value = json.loads(path.read_text(encoding="utf-8"))
    if not isinstance(value, dict) or value.get("schema") != "hcae.profile.v1":
        raise ValueError("invalid_profile_schema")
    return value


def main(argv: list[str] | None = None) -> int:
    import argparse
    parser = argparse.ArgumentParser(prog="critical-c11-structural-audit")
    parser.add_argument("profile", type=Path)
    parser.add_argument("--root", type=Path, default=Path("."))
    parser.add_argument("--clang", default="clang")
    args = parser.parse_args(argv)
    results = audit(load_profile(args.profile), args.root.resolve(), args.clang)
    for result in results:
        print(f"{result.status} {result.sink_id} {result.source}:{result.function} {result.detail}")
    if results and all(item.status == "STRUCTURALLY_COVERED" for item in results):
        print("C11_STRUCTURAL_AUDIT=PASS")
        return 0
    print("C11_STRUCTURAL_AUDIT=BLOCKED")
    return 1


if __name__ == "__main__":
    raise SystemExit(main())
