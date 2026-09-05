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
from critical_sink_audit import SinkResult
from critical_c11_structural_audit import StructuralSinkResult
from critical_effect_candidate_audit import CandidateResult
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
    assurance_scope: str = "formal-only"
    sink_audit_results: tuple[SinkResult, ...] = ()
    sink_audit_verdict: str | None = None
    sink_audit_reason: str | None = None
    c11_structural_results: tuple[StructuralSinkResult, ...] = ()
    c11_structural_verdict: str | None = None
    c11_structural_reason: str | None = None
    candidate_results: tuple[CandidateResult, ...] = ()
    candidate_verdict: str | None = None
    candidate_reason: str | None = None


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
    assurance_scope: str = "formal-only",
    sink_audit_results: tuple[SinkResult, ...] = (),
    sink_audit_verdict: str | None = None,
    sink_audit_reason: str | None = None,
    c11_structural_results: tuple[StructuralSinkResult, ...] = (),
    c11_structural_verdict: str | None = None,
    c11_structural_reason: str | None = None,
    candidate_results: tuple[CandidateResult, ...] = (),
    candidate_verdict: str | None = None,
    candidate_reason: str | None = None,
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
        "assurance_scope": assurance_scope,
        "sink_audit_results": [result.__dict__ for result in sink_audit_results],
        "sink_audit_verdict": sink_audit_verdict,
        "sink_audit_reason": sink_audit_reason,
        "c11_structural_results": [result.__dict__ for result in c11_structural_results],
        "c11_structural_verdict": c11_structural_verdict,
        "c11_structural_reason": c11_structural_reason,
        "candidate_results": [result.__dict__ for result in candidate_results],
        "candidate_verdict": candidate_verdict,
        "candidate_reason": candidate_reason,
    })

    def finish(verdict: AssuranceVerdict, reason: str) -> AssuranceCertificate:
        return AssuranceCertificate(
            verdict, reason, abstract_verification, concrete_verification,
            machine_refinement, policy_refinement, call_path_results, evidence_digest,
            structural_verdict, structural_reason, inventory_verdict, inventory_reason,
            assurance_scope, sink_audit_results, sink_audit_verdict, sink_audit_reason,
            c11_structural_results, c11_structural_verdict, c11_structural_reason,
            candidate_results, candidate_verdict, candidate_reason,
        )

    if abstract_verification.verdict == Verdict.COUNTEREXAMPLE or concrete_verification.verdict == Verdict.COUNTEREXAMPLE:
        reason = "concrete_policy_counterexample" if concrete_verification.verdict == Verdict.COUNTEREXAMPLE else "abstract_policy_counterexample"
        return finish(AssuranceVerdict.COUNTEREXAMPLE, reason)
    if abstract_verification.verdict in (Verdict.INVALID_SPEC,) or concrete_verification.verdict in (Verdict.INVALID_SPEC,):
        return finish(AssuranceVerdict.INVALID, "invalid_machine_or_policy_spec")
    if abstract_verification.verdict != Verdict.VERIFIED or concrete_verification.verdict != Verdict.VERIFIED:
        return finish(AssuranceVerdict.UNKNOWN, "verification_not_complete")
    if policy_refinement.verdict == PolicyRefinementVerdict.COUNTEREXAMPLE:
        return finish(AssuranceVerdict.COUNTEREXAMPLE, "policy_refinement_counterexample")
    if machine_refinement.verdict == RefinementVerdict.COUNTEREXAMPLE:
        return finish(AssuranceVerdict.COUNTEREXAMPLE, "machine_refinement_counterexample")
    if machine_refinement.verdict == RefinementVerdict.INVALID or policy_refinement.verdict == PolicyRefinementVerdict.INVALID:
        return finish(AssuranceVerdict.INVALID, "invalid_refinement_contract")
    if machine_refinement.verdict != RefinementVerdict.REFINED or policy_refinement.verdict != PolicyRefinementVerdict.REFINED:
        return finish(AssuranceVerdict.UNKNOWN, "refinement_not_complete")
    if not call_path_results or any(result.status != "COVERED" for result in call_path_results):
        return finish(AssuranceVerdict.BLOCKED, "critical_call_path_not_covered")
    if structural_verdict is not None and structural_verdict != "EXTRACTED_MATCH":
        return finish(AssuranceVerdict.BLOCKED, "structural_extraction_not_promoted")
    if inventory_verdict is not None and inventory_verdict != "PASS":
        return finish(AssuranceVerdict.BLOCKED, "critical_sink_inventory_not_promoted")
    if assurance_scope == "c11-bound" and (not sink_audit_results or sink_audit_verdict != "PASS"):
        return finish(AssuranceVerdict.BLOCKED, "critical_sink_audit_not_promoted")
    if assurance_scope == "c11-bound" and (not c11_structural_results or c11_structural_verdict != "PASS"):
        return finish(AssuranceVerdict.BLOCKED, "c11_structural_audit_not_promoted")
    if assurance_scope == "c11-bound" and (not candidate_results or candidate_verdict != "PASS"):
        return finish(AssuranceVerdict.BLOCKED, "critical_effect_candidates_not_promoted")
    return finish(AssuranceVerdict.ASSURED, "finite_chain_assured")
