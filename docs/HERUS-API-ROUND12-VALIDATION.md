# HERUS API — Validation of real read-only observation

## Hypothesis

HERUS API v1 can observe an allowlisted public GitHub repository through `GET` requests, produce a provenance-bearing observation, and preserve `authority=PROPOSAL_ONLY` without exposing remote execution.

This hypothesis is bounded to one real public repository, one ref, one local API process and one observation campaign. It does not claim production OAuth, webhooks, multi-tenancy, physical safety, remote rollback, general autonomy or AGI.

## Scope and authority boundary

The implementation is in `api/herus_api/` using FastAPI. The HTTP API uses strict Pydantic models, closed fields, the existing Semantic IR contract, `confidencePct >= runnerUpPct`, and `authority=PROPOSAL_ONLY`.

The GitHub endpoint accepts only the allowlisted repository `SummaArs/herus` and performs two read-only `GET` calls: repository metadata and a branch/ref lookup. No merge, deploy, delete, branch write, workflow, secret, permission, security, radio, actuator or physical path exists in this release. `/api/v1/executions` returns `501 EXECUTION_DISABLED` and `/api/v1/executions/dry-run` returns `501 DRY_RUN_NOT_IMPLEMENTED` until an instrumented transport proves zero mutating calls.

## Protocol

The route `POST /api/v1/connectors/github/observe` requires `repository`, `ref`, and `Idempotency-Key`. It validates the repository allowlist, ref characters, GitHub response shape, repository identity, commit SHA, branch and snapshot digest. The response is an observation, never an instruction.

Proposal creation also requires `Idempotency-Key`. Identical retries return the stored result with `Idempotency-Replayed: true`; reuse of a key for a different payload is rejected. Invalid Semantic IR, unknown fields, wrong authority and confidence lower than the runner-up are rejected fail-closed.

## Provenance and split

The real observation used the local API at `http://127.0.0.1:8080` and the public GitHub API at `https://api.github.com`. The repository was `SummaArs/herus`, ref `main`, with idempotency key `real-github-main-20260921`. The complete structured artifact is `research/evidence/herus_api_github_observation.json`.

The observed commit was `a5ae01ffb976fe3fc1eae002c97136ce2f13bf68`; the snapshot digest was `8e040daf8b187a8c6ff7b18588d71f77b11dbf1d7abec0d5b45c4f70d2ef0665`. No private payload, credential, secret or user data was stored. There is no train/holdout claim in this single observation; it is a local real-system regression sample, not an external holdout.

## Baselines

The disabled-execution baseline returns `501 EXECUTION_DISABLED`. The unimplemented dry-run baseline returns `501 DRY_RUN_NOT_IMPLEMENTED`. A proposal without an idempotency key returns `400 IDEMPOTENCY_KEY_REQUIRED`. An allowlist violation returns `422 VALIDATION_ERROR`. These baselines demonstrate that the first release refuses unsupported authority rather than silently falling back.

## Metrics with denominators

The real observation completed **2/2** read requests. Mutating requests were **0/0**. Proposal-authority violations were **0/1**. Unsafe decisions were **0/1**. Unhandled exceptions were **0/1**. The focused API suite passed **7/7** tests, and the full research suite passed **184/184** tests with one pre-existing skipped test.

The numerator and denominator of each metric are explicit. These denominators describe one bounded campaign, not users, production episodes, a probability of physical safety, or an estimate of general intelligence.

## Adversarial cases

The focused suite tests strict Semantic IR, `confidencePct < runnerUpPct`, missing idempotency keys, idempotency replay, conflicting idempotency payloads, GitHub repository allowlist rejection, disabled remote execution and disabled dry-run. Static checks verify JSON validity, Python compilation, diff hygiene and absence of common credential material.

The broader Round 11 red-team remains separate and passed 15/15 bounded simulator cases, including forged evidence, replay, forged token, expiry extension, mixed epoch, cross-contract identity, unknown state, external effect, attacker verifier, bad types, fake simulator, divergent state and action collision.

## Results

The API successfully answered health in `proposal-only` mode and observed the real public repository. The response contained observation ID `obs_b266d4c6240743488565986ba51262f8`, commit SHA, default branch, visibility, archive/fork flags, source `github-api`, authority `PROPOSAL_ONLY`, and a snapshot digest.

The API performed no remote mutation. Execution endpoints are rejected by construction. The allowlist and strict schema tests pass, and the full HERUS research suite plus the official `prove.sh` gate pass. The official gate now contains **42** gates, including the API suite, and ended with `ALL INVARIANTS HOLD` while preserving the statement that physical gates remain pending.

## Failures and corrections

The first API test attempt exposed a missing validator import and a test module collision with the package-level `app` object. Both were corrected. The first report validation exposed that the ASA validator requires canonical section headings and explicit provenance/metrics terminology; this report was revised without changing the measured results.

The API deliberately does not claim that a read-only GitHub observation proves execution safety. The missing capabilities—OAuth least privilege, signed webhooks, persistent replay ledger, KMS/secret vault, egress controls, multi-tenant storage and remote dry-run instrumentation—remain blocked rather than simulated as complete.

## Limitations

This is one public repository, one ref, one process and two `GET` requests. The idempotency store is process memory and does not yet prove crash recovery or distributed concurrency. OAuth, GitHub App installation tokens, webhook HMAC, tenant isolation, persistent audit, KMS, egress proxy, circuit breakers and provider-side compensation are not implemented.

The public repository response contains no proof that a future mutation would be reversible. Rollback in the existing simulator cannot be transferred to GitHub or any external API. A future write adapter would require separate capabilities, leases, durable idempotency, audit and explicit approval.

Production readiness, physical safety, AGI, general autonomy, open-world generalization and remote execution are **NÃO TESTADO** or **FALSO/REJEITADO**, not implied by these results.

## Claim matrix

| Claim | Status | Evidence | Limitation |
|---|---|---|---|
| API responds in proposal-only mode | PROVADO NO ESCOPO | `/api/v1/health`, 7/7 focused tests | Remote execution disabled |
| Allowlisted repository is observed by GET | PROVADO NO ESCOPO | Real observation, 2/2 reads | One public repository and ref |
| Identical proposal retry is controlled | PROVADO NO ESCOPO | Focused idempotency tests | Store is process memory |
| Semantic IR fields are strict | PROVADO NO ESCOPO | Pydantic models and tests | Not all ASA contracts |
| Remote mutating operations | FALSO/REJEITADO | `/api/v1/executions` returns 501 | Not available in this release |
| Production OAuth/API keys | NÃO TESTADO | Not implemented | Do not expose publicly |
| Signed webhooks | NÃO TESTADO | Not implemented | Future gate |
| Persistent multi-tenant isolation | NÃO TESTADO | Not implemented | Requires durable auth/storage |
| Physical safety, production and AGI | NÃO TESTADO | Outside hypothesis | No general claim |

## Reproduction

```bash
PYTHONPATH=. python3 -m unittest research.test_herus_api
PYTHONPATH=research python3 -m unittest discover -s research -p 'test_*.py'
./prove.sh --quiet
python /home/ubuntu/skills/herus-asa-proof/scripts/validate_asa_report.py docs/HERUS-API-ROUND12-VALIDATION.md
python3 api_server.py
curl http://127.0.0.1:8080/api/v1/health
curl -X POST http://127.0.0.1:8080/api/v1/connectors/github/observe \
  -H 'Content-Type: application/json' \
  -H 'Idempotency-Key: real-github-main-20260921' \
  -d '{"repository":"SummaArs/herus","ref":"main"}'
```
