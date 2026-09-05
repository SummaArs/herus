"""Load declarative finite assurance cases into the independent checkers."""
from __future__ import annotations

import json
from pathlib import Path
from typing import Any

from critical_assurance_certificate import AssuranceCertificate, compose_assurance
from critical_call_path_audit import CallPathResult, audit as audit_call_paths, load_profile
from memory_vault_structural_extractor import compare_source
from critical_sink_inventory import inventory as inventory_sinks, load_profile as load_hcae_profile
from critical_sink_audit import audit as audit_sinks
from critical_c11_structural_audit import audit as audit_c11_structure
from critical_effect_candidate_audit import audit as audit_effect_candidates, load_json as load_effect_json
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
    scope = value.get("assurance_scope")
    if scope not in {"formal-only", "c11-bound"}:
        raise ValueError("invalid_or_missing_assurance_scope")
    if scope == "formal-only":
        if any(key in value for key in ("source_contract", "hcae_profile", "call_path_profile", "effect_dispositions")):
            raise ValueError("formal_only_case_declares_c11_evidence")
        return value
    source_contract = value.get("source_contract")
    if not isinstance(source_contract, dict) or not isinstance(source_contract.get("implementation"), str) or not source_contract["implementation"]:
        raise ValueError("c11_bound_case_missing_source_contract")
    for key in ("hcae_profile", "call_path_profile", "effect_dispositions"):
        if not isinstance(value.get(key), str) or not value[key]:
            raise ValueError(f"c11_bound_case_missing_{key}")
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
    sink_audit_results = ()
    sink_audit_verdict = None
    sink_audit_reason = None
    c11_structural_results = ()
    c11_structural_verdict = None
    c11_structural_reason = None
    candidate_results = ()
    candidate_verdict = None
    candidate_reason = None
    hcae_profile_name = data.get("hcae_profile")
    if hcae_profile_name:
        repo_root = Path(__file__).parents[1]
        hcae_profile = load_hcae_profile(repo_root / hcae_profile_name)
        inventory_results = inventory_sinks(hcae_profile, repo_root)
        inventory_verdict = "PASS" if inventory_results and all(item.status == "PROFILED" for item in inventory_results) else "BLOCKED"
        inventory_reason = "all_known_and_annotated_sinks_profiled" if inventory_verdict == "PASS" else ";".join(item.detail for item in inventory_results if item.status != "PROFILED")
        sink_audit_results = audit_sinks(hcae_profile, repo_root)
        sink_audit_verdict = "PASS" if sink_audit_results and all(item.status == "COVERED" for item in sink_audit_results) else "BLOCKED"
        sink_audit_reason = "all_declared_sinks_covered" if sink_audit_verdict == "PASS" else ";".join(item.detail for item in sink_audit_results if item.status != "COVERED")
        c11_structural_results = audit_c11_structure(hcae_profile, repo_root)
        c11_structural_verdict = "PASS" if c11_structural_results and all(item.status == "STRUCTURALLY_COVERED" for item in c11_structural_results) else "BLOCKED"
        c11_structural_reason = "all_supported_paths_reject_before_sink" if c11_structural_verdict == "PASS" else ";".join(item.detail for item in c11_structural_results if item.status != "STRUCTURALLY_COVERED")
        dispositions = load_effect_json(repo_root / data["effect_dispositions"])
        candidate_results = audit_effect_candidates(hcae_profile, dispositions, repo_root)
        candidate_verdict = "PASS" if candidate_results and all(item.status != "REVIEW_REQUIRED" for item in candidate_results) else "BLOCKED"
        candidate_reason = "finite_candidate_queue_closed" if candidate_verdict == "PASS" else ";".join(item.detail for item in candidate_results if item.status == "REVIEW_REQUIRED")
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
        data["assurance_scope"],
        sink_audit_results,
        sink_audit_verdict,
        sink_audit_reason,
        c11_structural_results,
        c11_structural_verdict,
        c11_structural_reason,
        candidate_results,
        candidate_verdict,
        candidate_reason,
    )
