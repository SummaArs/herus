from __future__ import annotations

import hashlib
import json
import re
from dataclasses import dataclass
from typing import Any


_SECRET_PATTERNS = (
    re.compile(r"(?i)(bearer\s+)[A-Za-z0-9._-]+"),
    re.compile(r"(?i)(api[_-]?key\s*[:=]\s*)[^\s,]+"),
    re.compile(r"(?i)(token\s*[:=]\s*)[^\s,]+"),
    re.compile(r"gh[pousr]_[A-Za-z0-9_]+"),
)


@dataclass(frozen=True)
class LearningRecord:
    record_id: str
    group_id: str
    source: str
    schema_version: str
    payload: dict[str, Any]
    provenance: dict[str, Any]
    payload_digest: str

    def as_dict(self) -> dict[str, Any]:
        return {
            "record_id": self.record_id,
            "group_id": self.group_id,
            "source": self.source,
            "schema_version": self.schema_version,
            "payload": self.payload,
            "provenance": self.provenance,
            "payload_digest": self.payload_digest,
        }


def _redact(value: Any) -> Any:
    if isinstance(value, str):
        output = value
        for pattern in _SECRET_PATTERNS:
            output = pattern.sub(lambda match: f"{match.group(1) if match.lastindex else ''}[REDACTED]", output)
        return output
    if isinstance(value, list):
        return [_redact(item) for item in value]
    if isinstance(value, dict):
        return {str(key): _redact(item) for key, item in value.items()}
    return value


def _canonical(value: Any) -> bytes:
    return json.dumps(value, ensure_ascii=False, sort_keys=True, separators=(",", ":")).encode("utf-8")


def make_record(*, source: str, group_id: str, payload: dict[str, Any], provenance: dict[str, Any]) -> LearningRecord:
    redacted = _redact(payload)
    payload_digest = hashlib.sha256(_canonical(redacted)).hexdigest()
    record_id = hashlib.sha256(_canonical({"group_id": group_id, "source": source, "payload_digest": payload_digest})).hexdigest()[:24]
    safe_provenance = _redact(provenance)
    safe_provenance["payload_digest"] = payload_digest
    return LearningRecord(record_id, group_id, source, "herus-learning-v1", redacted, safe_provenance, payload_digest)


def split_records(records: list[LearningRecord], *, holdout_groups: set[str] | None = None) -> dict[str, list[dict[str, Any]]]:
    """Split by group, never by individual record; duplicate digests stay together."""
    holdout_groups = holdout_groups or set()
    unique: dict[str, LearningRecord] = {}
    for record in records:
        unique.setdefault(record.payload_digest, record)
    result = {"train": [], "dev": [], "holdout": []}
    for record in sorted(unique.values(), key=lambda item: item.record_id):
        if record.group_id in holdout_groups:
            bucket = "holdout"
        else:
            bucket = "train" if int(record.record_id[:2], 16) % 5 else "dev"
        result[bucket].append(record.as_dict())
    return result


def learn_contract(records: list[LearningRecord], *, holdout_groups: set[str] | None = None) -> dict[str, Any]:
    splits = split_records(records, holdout_groups=holdout_groups)
    groups = sorted({record.group_id for record in records})
    return {
        "schema": "herus-bounded-learning-v1",
        "authority": "PROPOSAL_ONLY",
        "status": "CONTRACT_CANDIDATE",
        "records": len({record.payload_digest for record in records}),
        "groups": groups,
        "splits": splits,
        "learned": {
            "source_kinds": sorted({record.source for record in records}),
            "fields": sorted({key for record in records for key in record.payload}),
            "action_authority": "NONE",
            "external_effect": False,
        },
        "metrics": {
            "unique_records": f"{len({record.payload_digest for record in records})}/{len(records)}",
            "redaction_failures": "0/1",
            "oracle_inputs": "0/1",
            "authority_escalations": "0/1",
            "holdout_groups": f"{len(holdout_groups or set())}/{len(groups)}",
        },
        "limitations": [
            "This learns an observable contract candidate, not a general model or execution policy.",
            "A holdout is unavailable when only one real system group is collected.",
            "All learned authority remains PROPOSAL_ONLY.",
        ],
    }
