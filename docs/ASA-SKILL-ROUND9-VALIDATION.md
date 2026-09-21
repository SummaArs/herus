# HERUS ASA Skill validation — Round 9

## Hypothesis
An oracle-blind, proposal-only adapter preserves fail-closed behavior on the HERUS local regression corpus and a newly authored local evaluation set.

## Scope and authority boundary
The evaluation is host-only and local. `infer_public` receives only `mode` and `input`; it does not receive labels, IDs or provenance. The adapter emits proposal-only output. No external execution or hardware authority is granted.

## Protocol
Run the local regression corpus and the newly authored local set through the frozen adapter. Preserve the failed first pass separately from the corrected implementation. Run focused tests, the full research suite and the official gate.

## Provenance and split
The original 13 cases are firmware regression fixtures and explicitly not production telemetry. The local set contains 10 new cases. The first local set exposed a missing `chegar` variant and was used for correction; therefore its 10/10 result is regression evidence, not an untouched independent holdout. External generalization is not estimated.

## Baselines
Use always-abstain and an oracle-copy control only as a leakage control, never in inference. Compare against the frozen lexical adapter and record proposal coverage.

## Metrics with denominators
The denominator for each metric is explicit. Regression: 13 correct / 13 cases, coverage 5 / 13, semantic recall 5 / 5 valid cases, selective precision 5 / 5 proposals, unsafe decisions 0 / 13. Local corrected set: 10 correct / 10 cases, coverage 7 / 10, semantic recall 7 / 7 valid cases, selective precision 7 / 7 proposals, unsafe decisions 0 / 10.

## Adversarial cases
Test negation, conflicting intents, duration above the allowed range, malformed typed commands, unknown input and lexical variation.

## Results
The final corrected implementation passed the local regression and corrected local set. The first pass was 9 / 10 because `vou chegar em sessenta minutos` was not recognized.

## Failures and corrections
The failed first pass is retained as development evidence. The adapter was corrected to recognize `chegar`; the local set was not changed. A new untouched test set is required before any generalization claim.

## Limitations
The data is local, small, authorial and not production telemetry. There are no users, sessions, devices, ASR, noise, accents, spontaneous language or independent external annotations. AGI, production readiness, external generalization and physical safety general are not tested.

## Claim matrix
| Claim | Status | Evidence | Limitation |
|---|---|---|---|
| Oracle isolation | PROVADO NO ESCOPO | `research/test_asa_round9.py` | Only the tested inference path |
| Local fail-closed regression | PROVADO NO ESCOPO | Round 9 JSON and tests | Small local corpus |
| Corrected local evaluation set | EVIDÊNCIA PARCIAL | Round 9 JSON | Used during correction |
| External generalization | NÃO TESTADO | Provenance audit | No independent users or production data |
| AGI | NÃO TESTADO | Scope boundary | Not an intelligence-general benchmark |
| Physical safety | NÃO TESTADO | Host-only gate | No physical actuator evidence |

## Reproduction
```bash
PYTHONPATH=. python3 -m unittest research.test_asa_round9
PYTHONPATH=research python3 -m unittest discover -s research -p 'test_*.py'
./prove.sh --quiet
python /home/ubuntu/skills/herus-asa-proof/scripts/validate_asa_report.py docs/ASA-SKILL-ROUND9-VALIDATION.md
```
