"""Evidence and obligation ledger for programming proposals.

Adapted conceptually from REA's conservative reconstruction ledgers. A
proposal closes only when every required case has comparable evidence. This
module is pure and never executes the proposed program.
"""
from __future__ import annotations
from dataclasses import dataclass
import hashlib
import json
from typing import Tuple

@dataclass(frozen=True)
class EvidenceRecord:
    evidence_id: str
    case_kind: str
    authority: str
    content_digest: str
    status: str = "authenticated"

@dataclass(frozen=True)
class ProgrammingObligation:
    obligation_id: str
    required_case_kinds: Tuple[str, ...]
    required_authority: str
    evidence_ids: Tuple[str, ...] = ()

@dataclass(frozen=True)
class LedgerResult:
    status: str
    closed_obligations: Tuple[str, ...]
    open_obligations: Tuple[str, ...]
    unknowns: Tuple[str, ...]
    closure_digest: str


def build_programming_ledger(obligations: Tuple[ProgrammingObligation, ...], evidence: Tuple[EvidenceRecord, ...]) -> LedgerResult:
    by_id = {item.evidence_id: item for item in evidence}
    closed, open_ids, unknowns = [], [], []
    for obligation in obligations:
        selected = [by_id[eid] for eid in obligation.evidence_ids if eid in by_id]
        missing = [kind for kind in obligation.required_case_kinds if not any(item.case_kind == kind for item in selected)]
        mismatched = [item.evidence_id for item in selected if item.authority != obligation.required_authority or item.status != "authenticated"]
        if missing:
            unknowns.extend(f"{obligation.obligation_id}:missing:{kind}" for kind in missing)
        if mismatched:
            unknowns.extend(f"{obligation.obligation_id}:incomparable:{eid}" for eid in mismatched)
        if not missing and not mismatched:
            closed.append(obligation.obligation_id)
        else:
            open_ids.append(obligation.obligation_id)
    payload = {
        "closed": closed,
        "open": open_ids,
        "unknowns": unknowns,
    }
    digest = hashlib.sha256(json.dumps(payload, sort_keys=True, separators=(",", ":")).encode()).hexdigest()
    return LedgerResult("closed" if not open_ids else "open", tuple(closed), tuple(open_ids), tuple(unknowns), digest)
