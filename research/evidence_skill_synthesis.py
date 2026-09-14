"""Evidence-gated Skill synthesis for the host-only ASA benchmark.

External research is treated as provenance and a hypothesis boundary. It may
select a bounded hypothesis from a finite, reviewed vocabulary, but it cannot
supply executable code, change the verifier, or grant authority. The resulting
Skill must pass the canonical quarantine -> tested -> verified lifecycle.
"""
from __future__ import annotations

from dataclasses import dataclass, replace
from typing import Iterable

from knowledge_gateway import EvidenceRecord, EvidenceStatus, ResearchQuestion, evaluate_evidence
from generative_lab.skill_library import SkillLibrary
from generative_lab.skills import Skill, SkillState, candidate_skill
from generative_lab.skill_verifier import ArithmeticCase, verify_arithmetic_skill


# This is deliberately finite and explicit. A claim selects a reviewed
# hypothesis; it does not become an opcode and it cannot add a new opcode.
CLAIM_HYPOTHESES: dict[str, tuple[str, ...]] = {
    "x_plus_one": ("INPUT", "CONST:1", "ADD"),
    "x_times_two": ("INPUT", "CONST:2", "MUL"),
    "negate_x": ("INPUT", "NEG"),
}


@dataclass(frozen=True)
class SynthesisResult:
    status: EvidenceStatus
    reason: str
    skill: Skill | None
    evidence_digest: str | None
    candidates: int
    rejected: int
    verifier_reason: str | None = None


def _hypothesis_allowed(question: ResearchQuestion, program: tuple[str, ...]) -> bool:
    """Require every non-input token to be in the question's finite vocabulary."""
    allowed = set(question.expected_vocabulary) | {"INPUT"}
    return all(token in allowed for token in program)


def synthesize_from_evidence(
    *,
    question: ResearchQuestion,
    evidence: EvidenceRecord,
    skill_id: str,
    visible: Iterable[ArithmeticCase],
    hidden: Iterable[ArithmeticCase],
) -> SynthesisResult:
    """Gate synthesis on provenance, then verify and attest a finite hypothesis."""
    decision = evaluate_evidence(question, evidence)
    if decision.status is not EvidenceStatus.ACCEPTED:
        return SynthesisResult(
            status=decision.status,
            reason=decision.reason,
            skill=None,
            evidence_digest=decision.evidence_digest,
            candidates=0,
            rejected=0,
        )

    visible_cases, hidden_cases = tuple(visible), tuple(hidden)
    candidates = 0
    rejected = 0
    for claim in evidence.claims:
        program = CLAIM_HYPOTHESES.get(claim)
        if program is None or not _hypothesis_allowed(question, program):
            continue
        candidates += 1
        candidate = candidate_skill(
            skill_id=skill_id,
            input_type="Int",
            output_type="Int",
            program=program,
            provenance={
                "kind": "evidence_selected_finite_hypothesis",
                "claim": claim,
                "research_question": question.question_id,
                "evidence_digest": decision.evidence_digest,
            },
            resource_budget={"steps": len(program), "memory": len(program) + 1},
        )
        verification = verify_arithmetic_skill(candidate, visible_cases, hidden_cases)
        if not verification.passed:
            rejected += 1
            continue

        # Reverification happens after enrichment, because the Skill content
        # hash is part of the attestation evidence.
        enriched = replace(
            candidate,
            provenance={
                **dict(candidate.provenance),
                "source_url": evidence.source_url,
                "retrieved_by": evidence.retrieved_by,
            },
        )
        verification = verify_arithmetic_skill(enriched, visible_cases, hidden_cases)
        if not verification.passed:
            rejected += 1
            continue

        library = SkillLibrary()
        library.add(enriched)
        library.transition(skill_id, 1, SkillState.QUARANTINED)
        library.transition(skill_id, 1, SkillState.TESTED)
        verified = library.attest(
            skill_id,
            1,
            evidence_hash=verification.evidence_hash,
            generalization_score=(verification.hidden_passed / len(hidden_cases)) if hidden_cases else 0.0,
            reliability=verification.visible_passed / (len(visible_cases) + len(hidden_cases)),
        )
        return SynthesisResult(
            status=EvidenceStatus.ACCEPTED,
            reason="verified_proposal_only",
            skill=verified,
            evidence_digest=decision.evidence_digest,
            candidates=candidates,
            rejected=rejected,
            verifier_reason=verification.reason,
        )

    return SynthesisResult(
        status=EvidenceStatus.ACCEPTED,
        reason="no_verified_candidate",
        skill=None,
        evidence_digest=decision.evidence_digest,
        candidates=candidates,
        rejected=rejected,
        verifier_reason="finite_hypothesis_bank_exhausted",
    )
