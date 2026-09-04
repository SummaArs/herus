"""Composition of finite assurance obligations into one audit certificate.

The composition is a coordinator, not a new authority. It can certify only the
finite claims explicitly supplied to it; physical adequacy and runtime authority
remain outside its scope.
"""
from __future__ import annotations

from dataclasses import dataclass
from enum import Enum
import hashlib
import json
from typing import Any, Iterable

from critical_call_path_audit import CallPathResult
from critical_state_refinement import (
    RefinementCertificate,
    RefinementVerdict,
    check_refinement_with_maps,
)
from critical_policy_refinement import (
    PolicyRefinementCertificate,
    PolicyRefinementVerdict,
    check_policy_refinement,
)
from critical_state_verifier import (
    Certificate,
    PolicyRule,
    StateMachineSpec,
    Verdict,
    verify,
)


class AssuranceVerdict(str, Enum):
    ASSURED = "ASSURED"
    COUNTEREXAMPLE = "COUNTEREXAMPLE"
    INVALID = "INVALID"
    BLOCKED = "BLOCKED"
    UNKNOWN = "UNKNOWN"


@dataclass(frozen=True)
class AssuranceCertificate:
    verdict: AssuranceVerdict
    reason: str
    abstract_verification: Certificate
    concrete_verification: Certificate
    machine_refinement: RefinementCertificate
    policy_refinement: PolicyRefinementCertificate
    call_path_results: tuple[CallPathResult, ...]
    evidence_digest: str
    structural_verdict: str | None = None
    structural_reason: str | None = None
    inventory_verdict: str | None = None
    inventory_reason: str | None = None


def _canonical_digest(value: Any) -> str:
    encoded = json.dumps(value, sort_keys=True, separators=(",", ":"), default=str).encode("utf-8")
    return hashlib.sha256(encoded).hexdigest()


def _spec_data(spec: StateMachineSpec) -> dict[str, Any]:
    return {
        "initial_state": spec.initial_state,
        "states": sorted(spec.states),
        "inputs": sorted(spec.inputs),
        "actions": sorted(spec.actions),
        "transitions": [item.__dict__ for item in spec.transitions],
        "forbidden_states": sorted(spec.forbidden_states),
        "forbidden_actions": sorted(spec.forbidden_actions),
        "max_steps": spec.max_steps,
    }


def _policy_data(policy: Iterable[PolicyRule]) -> list[dict[str, str]]:
    return [item.__dict__ for item in policy]


def compose_assurance(
    abstract: StateMachineSpec,
    concrete: StateMachineSpec,
    abstract_policy: tuple[PolicyRule, ...],
    concrete_policy: tuple[PolicyRule, ...],
    state_map: dict[str, str],
    input_map: dict[str, str],
    action_map: dict[str, str],
    call_path_results: tuple[CallPathResult, ...],
    structural_verdict: str | None = None,
    structural_reason: str | None = None,
    inventory_verdict: str | None = None,
    inventory_reason: str | None = None,
) -> AssuranceCertificate:
    abstract_verification = verify(abstract, abstract_policy)
    concrete_verification = verify(concrete, concrete_policy)
    machine_refinement = check_refinement_with_maps(
        abstract, concrete, state_map, input_map, action_map
    )
    policy_refinement = check_policy_refinement(
        abstract, concrete, abstract_policy, concrete_policy,
        state_map, input_map, action_map,
    )
    evidence_digest = _canonical_digest({
        "abstract": _spec_data(abstract),
        "concrete": _spec_data(concrete),
        "abstract_policy": _policy_data(abstract_policy),
        "concrete_policy": _policy_data(concrete_policy),
        "state_map": state_map,
        "input_map": input_map,
        "action_map": action_map,
        "call_paths": [result.__dict__ for result in call_path_results],
        "structural_verdict": structural_verdict,
        "structural_reason": structural_reason,
        "inventory_verdict": inventory_verdict,
        "inventory_reason": inventory_reason,
    })

    if abstract_verification.verdict == Verdict.COUNTEREXAMPLE or concrete_verification.verdict == Verdict.COUNTEREXAMPLE:
        reason = "concrete_policy_counterexample" if concrete_verification.verdict == Verdict.COUNTEREXAMPLE else "abstract_policy_counterexample"
        return AssuranceCertificate(AssuranceVerdict.COUNTEREXAMPLE, reason, abstract_verification, concrete_verification, machine_refinement, policy_refinement, call_path_results, evidence_digest)
    if abstract_verification.verdict in (Verdict.INVALID_SPEC,) or concrete_verification.verdict in (Verdict.INVALID_SPEC,):
        return AssuranceCertificate(AssuranceVerdict.INVALID, "invalid_machine_or_policy_spec", abstract_verification, concrete_verification, machine_refinement, policy_refinement, call_path_results, evidence_digest)
    if abstract_verification.verdict != Verdict.VERIFIED or concrete_verification.verdict != Verdict.VERIFIED:
        return AssuranceCertificate(AssuranceVerdict.UNKNOWN, "verification_not_complete", abstract_verification, concrete_verification, machine_refinement, policy_refinement, call_path_results, evidence_digest)
    if policy_refinement.verdict == PolicyRefinementVerdict.COUNTEREXAMPLE:
        return AssuranceCertificate(AssuranceVerdict.COUNTEREXAMPLE, "policy_refinement_counterexample", abstract_verification, concrete_verification, machine_refinement, policy_refinement, call_path_results, evidence_digest)
    if machine_refinement.verdict == RefinementVerdict.COUNTEREXAMPLE:
        return AssuranceCertificate(AssuranceVerdict.COUNTEREXAMPLE, "machine_refinement_counterexample", abstract_verification, concrete_verification, machine_refinement, policy_refinement, call_path_results, evidence_digest)
    if machine_refinement.verdict == RefinementVerdict.INVALID or policy_refinement.verdict == PolicyRefinementVerdict.INVALID:
        return AssuranceCertificate(AssuranceVerdict.INVALID, "invalid_refinement_contract", abstract_verification, concrete_verification, machine_refinement, policy_refinement, call_path_results, evidence_digest)
    if machine_refinement.verdict != RefinementVerdict.REFINED or policy_refinement.verdict != PolicyRefinementVerdict.REFINED:
        return AssuranceCertificate(AssuranceVerdict.UNKNOWN, "refinement_not_complete", abstract_verification, concrete_verification, machine_refinement, policy_refinement, call_path_results, evidence_digest)
    if not call_path_results or any(result.status != "COVERED" for result in call_path_results):
        return AssuranceCertificate(AssuranceVerdict.BLOCKED, "critical_call_path_not_covered", abstract_verification, concrete_verification, machine_refinement, policy_refinement, call_path_results, evidence_digest, structural_verdict, structural_reason)
    if structural_verdict is not None and structural_verdict != "EXTRACTED_MATCH":
        return AssuranceCertificate(AssuranceVerdict.BLOCKED, "structural_extraction_not_promoted", abstract_verification, concrete_verification, machine_refinement, policy_refinement, call_path_results, evidence_digest, structural_verdict, structural_reason, inventory_verdict, inventory_reason)
    if inventory_verdict is not None and inventory_verdict != "PASS":
        return AssuranceCertificate(AssuranceVerdict.BLOCKED, "critical_sink_inventory_not_promoted", abstract_verification, concrete_verification, machine_refinement, policy_refinement, call_path_results, evidence_digest, structural_verdict, structural_reason, inventory_verdict, inventory_reason)
    return AssuranceCertificate(AssuranceVerdict.ASSURED, "finite_chain_assured", abstract_verification, concrete_verification, machine_refinement, policy_refinement, call_path_results, evidence_digest, structural_verdict, structural_reason, inventory_verdict, inventory_reason)
