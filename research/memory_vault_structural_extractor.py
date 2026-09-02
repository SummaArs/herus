"""Conservative, host-only structural extraction for memory_vault.c.

This module never executes C. It recognizes only a small lexical/structural
subset and returns UNKNOWN for unsupported or ambiguous constructions.
"""
from __future__ import annotations

from dataclasses import dataclass, asdict
from enum import Enum
import hashlib
import json
import re
from pathlib import Path
from typing import Any


class ExtractionVerdict(str, Enum):
    EXTRACTED_MATCH = "EXTRACTED_MATCH"
    DIVERGENCE = "DIVERGENCE"
    UNKNOWN = "UNKNOWN"


@dataclass(frozen=True)
class Observation:
    kind: str
    value: str
    line: int
    function: str


@dataclass(frozen=True)
class ExtractionResult:
    verdict: ExtractionVerdict
    reason: str
    source_digest: str
    observations: tuple[Observation, ...]
    missing: tuple[str, ...]
    unsupported: tuple[str, ...]

    def to_dict(self) -> dict[str, Any]:
        value = asdict(self)
        value["verdict"] = self.verdict.value
        value["observations"] = [asdict(item) for item in self.observations]
        return value


_FUNCTION_RE = re.compile(
    r"(?m)^int\s+(memory_vault_(?:init|seal|open|erase))\s*\([^;]*\)\s*\{"
)
_STATE_ASSIGN_RE = re.compile(r"\bv->state\s*=\s*(MEMORY_VAULT_[A-Z_]+)")
_RETURN_RE = re.compile(r"\breturn\s+(MEMORY_VAULT_E_[A-Z_]+|MEMORY_VAULT_OK)\s*;")
_CALL_RE = re.compile(r"\b([A-Za-z_][A-Za-z0-9_]*)\s*\(")


def _line_number(source: str, offset: int) -> int:
    return source.count("\n", 0, offset) + 1


def _function_bodies(source: str) -> tuple[dict[str, tuple[int, int, str]], tuple[str, ...]]:
    bodies: dict[str, tuple[int, int, str]] = {}
    unsupported: list[str] = []
    for match in _FUNCTION_RE.finditer(source):
        name = match.group(1)
        start = match.end() - 1
        depth = 0
        end = None
        for index in range(start, len(source)):
            char = source[index]
            if char == "{":
                depth += 1
            elif char == "}":
                depth -= 1
                if depth == 0:
                    end = index + 1
                    break
        if end is None:
            unsupported.append(f"unbalanced_body:{name}")
            continue
        nested = _FUNCTION_RE.search(source, start, end)
        if nested is not None:
            unsupported.append(f"unbalanced_body:{name}")
            continue
        bodies[name] = (match.start(), end, source[start:end])
    return bodies, tuple(unsupported)


def _observe_function(source: str, name: str, start: int, body: str) -> list[Observation]:
    observations: list[Observation] = []
    for match in _STATE_ASSIGN_RE.finditer(body):
        observations.append(Observation("state_assignment", match.group(1), _line_number(source, start + match.start()), name))
    for match in _RETURN_RE.finditer(body):
        observations.append(Observation("return", match.group(1), _line_number(source, start + match.start()), name))
    for match in _CALL_RE.finditer(body):
        call = match.group(1)
        if call in {"store_sealed", "commit_generation_floor", "erase_sealed", "load_generation_floor", "load_sealed"}:
            observations.append(Observation("storage_call", call, _line_number(source, start + match.start()), name))
    for token in ("auth_valid", "card_valid"):
        position = body.find(token)
        if position >= 0:
            observations.append(Observation("guard", token, _line_number(source, start + position), name))
    return observations


def extract_source(path: Path) -> tuple[str, tuple[Observation, ...], tuple[str, ...]]:
    source = path.read_text(encoding="utf-8")
    bodies, unsupported = _function_bodies(source)
    observations: list[Observation] = []
    for name, (start, end, body) in bodies.items():
        observations.extend(_observe_function(source, name, start, body))
    allowed_macros = {"VAULT_HEADER_LEN", "VAULT_CARD_LEN", "VAULT_TAG_LEN"}
    for directive in re.finditer(r"(?m)^\s*#define\s+([A-Za-z_][A-Za-z0-9_]*)", source):
        if directive.group(1) not in allowed_macros:
            unsupported = tuple(unsupported) + ("ambiguous_preprocessor",)
            break
    return source, tuple(sorted(observations, key=lambda item: (item.line, item.kind, item.value))), unsupported


def _expected(source_case: dict[str, Any]) -> tuple[set[str], set[str]]:
    concrete = source_case["concrete"]
    expected = {f"state:{item}" for item in concrete["states"]}
    expected |= {f"action:{item}" for item in concrete["actions"]}
    expected |= {f"input:{item}" for item in concrete["inputs"]}
    required = {
        "function:memory_vault_init", "function:memory_vault_seal",
        "function:memory_vault_open", "function:memory_vault_erase",
        "state:MEMORY_VAULT_BLOCKED",
        "storage:store_sealed", "storage:commit_generation_floor",
        "guard:auth_valid", "guard:card_valid",
    }
    return expected, required


def compare_source(source_path: Path, case_path: Path) -> ExtractionResult:
    source, observations, unsupported = extract_source(source_path)
    digest = hashlib.sha256(source.encode("utf-8")).hexdigest()
    case = json.loads(case_path.read_text(encoding="utf-8"))
    if case.get("schema") != "herus.assurance-case.v1":
        return ExtractionResult(ExtractionVerdict.UNKNOWN, "invalid_case_schema", digest, observations, (), ("invalid_case_schema",))
    bodies, body_unsupported = _function_bodies(source)
    unsupported = tuple(sorted(set(unsupported + body_unsupported)))
    if unsupported:
        return ExtractionResult(ExtractionVerdict.UNKNOWN, "unsupported_construct", digest, observations, (), unsupported)
    observed = {f"function:{name}" for name in bodies}
    observed |= {f"state:{item.value}" for item in observations if item.kind == "state_assignment"}
    observed |= {f"action:{item.value}" for item in observations if item.kind == "return"}
    observed |= {f"storage:{item.value}" for item in observations if item.kind == "storage_call"}
    observed |= {f"guard:{item.value}" for item in observations if item.kind == "guard"}
    _, required = _expected(case)
    missing = tuple(sorted(required - observed))
    seal_guards = [item for item in observations if item.function == "memory_vault_seal" and item.kind == "guard"]
    seal_sinks = [item for item in observations if item.function == "memory_vault_seal" and item.kind == "storage_call" and item.value in {"store_sealed", "commit_generation_floor"}]
    if any(guard.line > sink.line for guard in seal_guards for sink in seal_sinks):
        return ExtractionResult(ExtractionVerdict.DIVERGENCE, "guard_after_persistence_sink", digest, observations, missing, unsupported)
    if missing:
        return ExtractionResult(ExtractionVerdict.DIVERGENCE, "required_observation_missing", digest, observations, missing, unsupported)
    return ExtractionResult(ExtractionVerdict.EXTRACTED_MATCH, "declared_obligations_observed", digest, observations, (), ())
