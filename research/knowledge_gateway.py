"""Proposal-only knowledge gateway for ASA external research.

The gateway accepts bounded evidence records from an auxiliary host. It does
not fetch arbitrary URLs, execute downloaded code, or grant authority.
"""
from __future__ import annotations

from dataclasses import dataclass
from enum import Enum
import hashlib
from urllib.parse import urlparse


class EvidenceStatus(str, Enum):
    ACCEPTED = "ACCEPTED"
    REJECTED = "REJECTED"
    NEEDS_REVIEW = "NEEDS_REVIEW"


@dataclass(frozen=True)
class ResearchQuestion:
    question_id: str
    query: str
    expected_vocabulary: frozenset[str]
    max_results: int = 3


@dataclass(frozen=True)
class EvidenceRecord:
    source_url: str
    title: str
    content: str
    retrieved_by: str
    retrieved_at: str
    source_digest: str
    claims: tuple[str, ...]


@dataclass(frozen=True)
class EvidenceDecision:
    status: EvidenceStatus
    reason: str
    evidence_digest: str | None
    executable: bool = False


def digest_source(record: EvidenceRecord) -> str:
    payload = "\x1f".join((record.source_url, record.title, record.content)).encode("utf-8")
    return hashlib.sha256(payload).hexdigest()


def digest_evidence(record: EvidenceRecord) -> str:
    payload = "\x1f".join(
        (
            record.source_url,
            record.title,
            record.content,
            record.retrieved_by,
            record.retrieved_at,
            record.source_digest,
            *record.claims,
        )
    ).encode("utf-8")
    return hashlib.sha256(payload).hexdigest()


def evaluate_evidence(question: ResearchQuestion, record: EvidenceRecord) -> EvidenceDecision:
    parsed = urlparse(record.source_url)
    if parsed.scheme not in {"https"} or not parsed.netloc:
        return EvidenceDecision(EvidenceStatus.REJECTED, "source_must_use_https", None)
    if not record.content.strip() or not record.title.strip():
        return EvidenceDecision(EvidenceStatus.REJECTED, "empty_evidence", None)
    if not record.retrieved_by.strip() or not record.retrieved_at.strip():
        return EvidenceDecision(EvidenceStatus.REJECTED, "missing_provenance", None)
    if not record.claims:
        return EvidenceDecision(EvidenceStatus.NEEDS_REVIEW, "no_explicit_claims", digest_evidence(record))
    if question.max_results < 1:
        return EvidenceDecision(EvidenceStatus.REJECTED, "invalid_budget", None)
    if digest_source(record) != record.source_digest:
        return EvidenceDecision(EvidenceStatus.REJECTED, "digest_mismatch", None)
    return EvidenceDecision(EvidenceStatus.ACCEPTED, "bounded_provenance_verified", digest_evidence(record))
