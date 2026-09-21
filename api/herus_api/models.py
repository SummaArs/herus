from __future__ import annotations

from datetime import datetime, timezone
from typing import Any, Literal
from uuid import uuid4

from pydantic import BaseModel, ConfigDict, Field, StrictInt, StrictStr, field_validator, model_validator


class StrictModel(BaseModel):
    model_config = ConfigDict(extra="forbid", strict=True)


class Evidence(StrictModel):
    kind: Literal["OBSERVATION", "RULE", "MODEL", "USER"]
    ref: StrictStr = Field(min_length=1, max_length=48, pattern=r"^[A-Za-z0-9_.:-]+$")
    polarity: Literal["POSITIVE", "NEGATIVE"]
    weight: StrictInt = Field(ge=0, le=100)


class Slots(StrictModel):
    minutes: StrictInt | None = Field(default=None, ge=1, le=60)


class SemanticIR(StrictModel):
    schemaVersion: Literal[1]
    eventKind: Literal["ARRIVE", "HELP", "CANCEL"]
    source: Literal["TEXT", "VOICE", "BUTTON", "SENSOR", "VISION", "CODE", "ENVIRONMENT"]
    confidencePct: StrictInt = Field(ge=0, le=100)
    runnerUpPct: StrictInt = Field(ge=0, le=100)
    slots: Slots
    evidence: list[Evidence] = Field(min_length=1, max_length=8)
    hypothesisStatus: Literal["TRUE", "FALSE", "BOTH", "NEITHER"]
    authority: Literal["PROPOSAL_ONLY"]

    @model_validator(mode="after")
    def confidence_not_below_runner_up(self):
        if self.confidencePct < self.runnerUpPct:
            raise ValueError("confidencePct must be >= runnerUpPct")
        return self


class ProposalRequest(StrictModel):
    ir: SemanticIR
    mode: Literal["proposal-only"] = "proposal-only"


class Proposal(StrictModel):
    proposal_id: str
    status: Literal["PROPOSED", "ABSTAINED"]
    authority: Literal["PROPOSAL_ONLY"]
    mode: Literal["proposal-only"]
    ir: SemanticIR
    created_at: str
    request_id: str


class GitHubObservationRequest(StrictModel):
    repository: StrictStr = Field(default="SummaArs/herus", pattern=r"^[A-Za-z0-9_.-]+/[A-Za-z0-9_.-]+$")
    ref: StrictStr = Field(default="main", min_length=1, max_length=255, pattern=r"^[A-Za-z0-9._/-]+$")
    mode: Literal["observation"] = "observation"

    @field_validator("repository")
    @classmethod
    def only_allowlisted_repo(cls, value: str) -> str:
        if value != "SummaArs/herus":
            raise ValueError("repository is not allowlisted")
        return value


class GitHubObservation(StrictModel):
    observation_id: str
    provider: Literal["github"]
    repository: str
    ref: str
    commit_sha: str
    default_branch: str
    visibility: str
    archived: bool
    fork: bool
    observed_at: str
    authority: Literal["PROPOSAL_ONLY"]
    source: Literal["github-api"]
    provenance: dict[str, Any]


class DryRunRequest(StrictModel):
    operation: StrictStr
    mode: Literal["dry-run"] = "dry-run"


def now_iso() -> str:
    return datetime.now(timezone.utc).isoformat()


def new_id(prefix: str) -> str:
    return f"{prefix}_{uuid4().hex}"
