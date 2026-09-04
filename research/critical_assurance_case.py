"""Load declarative finite assurance cases into the independent checkers."""
from __future__ import annotations

import json
from pathlib import Path
from typing import Any

from critical_assurance_certificate import AssuranceCertificate, compose_assurance
from critical_call_path_audit import CallPathResult, audit as audit_call_paths, load_profile
from memory_vault_structural_extractor import compare_source
from critical_sink_inventory import inventory as inventory_sinks, load_profile as load_hcae_profile
from critical_state_verifier import PolicyRule, StateMachineSpec, Transition


def _spec(data: dict[str, Any]) -> StateMachineSpec:
    return StateMachineSpec(
        initial_state=data["initial_state"],
        states=frozenset(data["states"]),
        inputs=frozenset(data["inputs"]),
        actions=frozenset(data["actions"]),
        transitions=tuple(Transition(**item) for item in data["transitions"]),
        forbidden_states=frozenset(data["forbidden_states"]),
        forbidden_actions=frozenset(data["forbidden_actions"]),
        max_steps=data["max_steps"],
    )


def _policy(items: list[dict[str, str]]) -> tuple[PolicyRule, ...]:
    return tuple(PolicyRule(**item) for item in items)


def load_case(path: Path) -> dict[str, Any]:
    value = json.loads(path.read_text(encoding="utf-8"))
    if not isinstance(value, dict) or value.get("schema") != "herus.assurance-case.v1":
        raise ValueError("invalid_assurance_case_schema")
    return value


def run_case(path: Path) -> AssuranceCertificate:
    data = load_case(path)
    abstract = _spec(data["abstract"])
    concrete = _spec(data["concrete"])
    declared_paths = tuple(CallPathResult(**item) for item in data["call_paths"])
    paths = declared_paths
    profile_name = data.get("call_path_profile")
    if profile_name:
        repo_root = Path(__file__).parents[1]
        profile = load_profile(repo_root / profile_name)
        audited_paths = audit_call_paths(profile, repo_root)
        declared_failures = tuple(item for item in declared_paths if item.status != "COVERED")
        paths = audited_paths + declared_failures
    structural_verdict = None
    structural_reason = None
    inventory_verdict = None
    inventory_reason = None
    hcae_profile_name = data.get("hcae_profile")
    if hcae_profile_name:
        repo_root = Path(__file__).parents[1]
        inventory_results = inventory_sinks(load_hcae_profile(repo_root / hcae_profile_name), repo_root)
        inventory_verdict = "PASS" if inventory_results and all(item.status == "PROFILED" for item in inventory_results) else "BLOCKED"
        inventory_reason = "all_known_and_annotated_sinks_profiled" if inventory_verdict == "PASS" else ";".join(item.detail for item in inventory_results if item.status != "PROFILED")
    implementation = data.get("source_contract", {}).get("implementation")
    if implementation:
        source_path = Path(__file__).parents[1] / implementation
        extracted = compare_source(source_path, path)
        structural_verdict = extracted.verdict.value
        structural_reason = extracted.reason
    return compose_assurance(
        abstract,
        concrete,
        _policy(data["abstract_policy"]),
        _policy(data["concrete_policy"]),
        data["state_map"],
        data["input_map"],
        data["action_map"],
        paths,
        structural_verdict,
        structural_reason,
        inventory_verdict,
        inventory_reason,
    )
