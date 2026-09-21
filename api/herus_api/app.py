from __future__ import annotations

from fastapi import FastAPI, Header, Request, status
from fastapi.exceptions import RequestValidationError
from fastapi.responses import JSONResponse

from .github_observer import GitHubObservationError, GitHubObserver
from .learning import learn_contract, make_record
from .models import (
    DryRunRequest,
    GitHubObservation,
    GitHubObservationRequest,
    Proposal,
    ProposalRequest,
    now_iso,
    new_id,
)

app = FastAPI(
    title="HERUS API",
    version="1.0.0",
    description="Bounded proposal-only and read-only observation API for HERUS.",
    openapi_version="3.1.0",
)


@app.exception_handler(RequestValidationError)
async def validation_exception_handler(request: Request, exc: RequestValidationError):
    return problem("VALIDATION_ERROR", "request failed strict schema validation", request, 422)

_PROPOSALS: dict[str, Proposal] = {}
_OBSERVATIONS: dict[str, GitHubObservation] = {}
_LEARNING_RECORDS = []
_IDEMPOTENCY: dict[str, tuple[str, object]] = {}


def request_id(request: Request) -> str:
    return request.headers.get("x-request-id", new_id("req"))


def problem(code: str, detail: str, request: Request, http_status: int = 422) -> JSONResponse:
    rid = request_id(request)
    return JSONResponse(status_code=http_status, content={
        "type": f"https://herus.local/problems/{code.lower()}",
        "title": code,
        "status": http_status,
        "code": code,
        "detail": detail,
        "instance": str(request.url),
        "requestId": rid,
    }, media_type="application/problem+json", headers={"X-Request-Id": rid})


@app.get("/api/v1/health")
def health(request: Request):
    return {"status": "ok", "service": "herus-api", "version": "1.0.0", "mode": "proposal-only", "requestId": request_id(request)}


@app.post("/api/v1/proposals", response_model=Proposal, status_code=status.HTTP_201_CREATED)
def create_proposal(payload: ProposalRequest, request: Request, idempotency_key: str | None = Header(default=None, alias="Idempotency-Key")):
    if not idempotency_key:
        return problem("IDEMPOTENCY_KEY_REQUIRED", "POST requires Idempotency-Key", request, 400)
    fingerprint = f"proposal:{payload.model_dump_json()}"
    prior = _IDEMPOTENCY.get(idempotency_key)
    if prior and prior[0] != fingerprint:
        return problem("IDEMPOTENCY_CONFLICT", "Idempotency-Key was reused for a different payload", request, 422)
    if prior:
        response = prior[1]
        if isinstance(response, Proposal):
            return JSONResponse(status_code=200, content=response.model_dump(), headers={"Idempotency-Replayed": "true", "X-Request-Id": request_id(request)})
    proposal = Proposal(proposal_id=new_id("proposal"), status="PROPOSED", authority="PROPOSAL_ONLY", mode="proposal-only", ir=payload.ir, created_at=now_iso(), request_id=request_id(request))
    _PROPOSALS[proposal.proposal_id] = proposal
    _IDEMPOTENCY[idempotency_key] = (fingerprint, proposal)
    return proposal


@app.get("/api/v1/proposals/{proposal_id}")
def get_proposal(proposal_id: str, request: Request):
    proposal = _PROPOSALS.get(proposal_id)
    if proposal is None:
        return problem("NOT_FOUND", "proposal not found", request, 404)
    return proposal


@app.post("/api/v1/connectors/github/observe", response_model=GitHubObservation)
def observe_github(payload: GitHubObservationRequest, request: Request, idempotency_key: str | None = Header(default=None, alias="Idempotency-Key")):
    if not idempotency_key:
        return problem("IDEMPOTENCY_KEY_REQUIRED", "observation requires Idempotency-Key", request, 400)
    fingerprint = f"github-observe:{payload.repository}:{payload.ref}"
    prior = _IDEMPOTENCY.get(idempotency_key)
    if prior and prior[0] != fingerprint:
        return problem("IDEMPOTENCY_CONFLICT", "Idempotency-Key was reused for a different observation", request, 422)
    if prior and isinstance(prior[1], GitHubObservation):
        return JSONResponse(status_code=200, content=prior[1].model_dump(), headers={"Idempotency-Replayed": "true", "X-Request-Id": request_id(request)})
    try:
        result = GitHubObserver(repository=payload.repository).observe(payload.ref)
    except GitHubObservationError as error:
        return problem(str(error), "GitHub observation was rejected or unavailable", request, 502 if str(error) == "GITHUB_UNAVAILABLE" else 422)
    snapshot = result["snapshot"]
    observation = GitHubObservation(observation_id=new_id("obs"), provider="github", repository=payload.repository, ref=payload.ref, commit_sha=snapshot["commit_sha"], default_branch=snapshot["default_branch"], visibility=snapshot["visibility"], archived=snapshot["archived"], fork=snapshot["fork"], observed_at=now_iso(), authority="PROPOSAL_ONLY", source="github-api", provenance={"snapshot_digest": result["snapshot_digest"], "endpoint": "https://api.github.com/repos/SummaArs/herus", "method": "GET", "redaction": "none-public-repository"})
    _OBSERVATIONS[observation.observation_id] = observation
    _IDEMPOTENCY[idempotency_key] = (fingerprint, observation)
    return observation


@app.get("/api/v1/observations/{observation_id}")
def get_observation(observation_id: str, request: Request):
    observation = _OBSERVATIONS.get(observation_id)
    if observation is None:
        return problem("NOT_FOUND", "observation not found", request, 404)
    return observation


@app.post("/api/v1/learning/github")
def learn_from_github(payload: GitHubObservationRequest, request: Request, idempotency_key: str | None = Header(default=None, alias="Idempotency-Key")):
    if not idempotency_key:
        return problem("IDEMPOTENCY_KEY_REQUIRED", "learning requires Idempotency-Key", request, 400)
    fingerprint = f"github-learning:{payload.repository}:{payload.ref}"
    prior = _IDEMPOTENCY.get(idempotency_key)
    if prior and prior[0] != fingerprint:
        return problem("IDEMPOTENCY_CONFLICT", "Idempotency-Key was reused for different learning input", request, 422)
    if prior and isinstance(prior[1], dict) and prior[1].get("schema") == "herus-bounded-learning-v1":
        return JSONResponse(status_code=200, content=prior[1], headers={"Idempotency-Replayed": "true", "X-Request-Id": request_id(request)})
    try:
        observed = GitHubObserver(repository=payload.repository).observe(payload.ref)
    except GitHubObservationError as error:
        return problem(str(error), "GitHub learning input was rejected or unavailable", request, 502 if str(error) == "GITHUB_UNAVAILABLE" else 422)
    snapshot = observed["snapshot"]
    record = make_record(source="github", group_id=f"{payload.repository}:{payload.ref}", payload=snapshot, provenance={"provider": "github", "snapshot_digest": observed["snapshot_digest"], "authority": "PROPOSAL_ONLY", "mode": "observation"})
    if all(existing.payload_digest != record.payload_digest for existing in _LEARNING_RECORDS):
        _LEARNING_RECORDS.append(record)
    result = learn_contract(_LEARNING_RECORDS)
    result["request_id"] = request_id(request)
    result["mode"] = "observation"
    result["source"] = "github"
    _IDEMPOTENCY[idempotency_key] = (fingerprint, result)
    return result


@app.post("/api/v1/executions/dry-run")
def dry_run(payload: DryRunRequest, request: Request, idempotency_key: str | None = Header(default=None, alias="Idempotency-Key")):
    return problem("DRY_RUN_NOT_IMPLEMENTED", "No remote execution adapter exists; only observation and proposal are enabled", request, 501)


@app.post("/api/v1/executions")
def executions_disabled(request: Request):
    return problem("EXECUTION_DISABLED", "Remote execution is disabled by construction", request, 501)
