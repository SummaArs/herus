"""Bounded HTTPS retrieval for public evidence; never executes retrieved content."""
from __future__ import annotations
from dataclasses import dataclass
from urllib.parse import urlparse
from urllib.request import Request, urlopen

from internet_host import InternetHostPolicy, make_record
from knowledge_gateway import EvidenceRecord


@dataclass(frozen=True)
class FetchPolicy:
    allowed_domains: frozenset[str]
    max_urls: int = 4
    max_bytes_per_url: int = 200_000
    timeout_seconds: int = 8


def fetch_public(url: str, policy: FetchPolicy) -> EvidenceRecord:
    parsed = urlparse(url)
    if parsed.scheme != "https" or (parsed.hostname or "").lower() not in policy.allowed_domains:
        raise ValueError("url_not_allowlisted")
    request = Request(url, headers={"User-Agent": "HERUS-evidence-host/1.0"})
    with urlopen(request, timeout=policy.timeout_seconds) as response:
        content_type = response.headers.get("Content-Type", "")
        if not any(kind in content_type for kind in ("text/", "application/pdf")):
            raise ValueError("content_type_not_allowed")
        data = response.read(policy.max_bytes_per_url + 1)
    if len(data) > policy.max_bytes_per_url:
        raise ValueError("response_budget_exhausted")
    content = data.decode("utf-8", errors="replace") if "text/" in content_type else "PDF_BYTES_OMITTED_FROM_CLAIM_EXTRACTION"
    title = parsed.path.rstrip("/").split("/")[-1] or parsed.hostname or "untitled"
    return make_record(url, title, content, ("external_source_retrieved",))


def fetch_many(urls: tuple[str, ...], policy: FetchPolicy) -> tuple[EvidenceRecord, ...]:
    if len(urls) > policy.max_urls:
        raise ValueError("url_budget_exhausted")
    records = []
    for url in urls:
        records.append(fetch_public(url, policy))
    return tuple(records)
