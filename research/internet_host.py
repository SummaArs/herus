"""Internet as an evidence host, never as an authority or executable model source."""
from __future__ import annotations

from dataclasses import dataclass
from urllib.parse import urlparse

from knowledge_gateway import EvidenceDecision, EvidenceRecord, EvidenceStatus, digest_source, evaluate_evidence


@dataclass(frozen=True)
class InternetHostPolicy:
    allowed_domains: frozenset[str]
    max_content_chars: int = 12000
    max_records: int = 8


@dataclass(frozen=True)
class InternetIngestion:
    accepted: tuple[EvidenceRecord, ...]
    decisions: tuple[EvidenceDecision, ...]
    useful_claims: tuple[str, ...]
    executable: bool


def ingest(records: tuple[EvidenceRecord, ...], question, policy: InternetHostPolicy) -> InternetIngestion:
    accepted: list[EvidenceRecord] = []
    decisions: list[EvidenceDecision] = []
    claims: list[str] = []
    if len(records) > policy.max_records:
        return InternetIngestion((), (EvidenceDecision(EvidenceStatus.REJECTED, "record_budget_exhausted", None),), (), False)
    for record in records:
        domain = (urlparse(record.source_url).hostname or "").lower()
        if domain not in policy.allowed_domains:
            decisions.append(EvidenceDecision(EvidenceStatus.REJECTED, "domain_not_allowlisted", None))
            continue
        if len(record.content) > policy.max_content_chars:
            decisions.append(EvidenceDecision(EvidenceStatus.REJECTED, "content_budget_exhausted", None))
            continue
        decision = evaluate_evidence(question, record)
        decisions.append(decision)
        if decision.status is EvidenceStatus.ACCEPTED:
            accepted.append(record)
            claims.extend(record.claims)
    return InternetIngestion(tuple(accepted), tuple(decisions), tuple(dict.fromkeys(claims)), False)


def make_record(url: str, title: str, content: str, claims: tuple[str, ...]) -> EvidenceRecord:
    base = EvidenceRecord(url, title, content, "herus-internet-host", "2026-09-16T12:00:00Z", "", claims)
    return EvidenceRecord(base.source_url, base.title, base.content, base.retrieved_by, base.retrieved_at, digest_source(base), base.claims)
