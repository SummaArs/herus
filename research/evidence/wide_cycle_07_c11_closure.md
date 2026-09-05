# HERUS — Wide Research Cycle 07: C11 Evidence Closure

## Scope

This cycle remained **host-only**. No hardware result was inferred, simulated as physical evidence, or promoted into the certificate. The objective was to make a C11-bound assurance case fail closed when critical effects are unprofiled, structurally ambiguous, lexically misguarded, or omitted from the finite candidate boundary.

## Changes with security effect

The C11 surface now contains **13 annotated HCAE sinks**. The additions include destructive vault erasure, reviewed erasure, trust activation persistence, dual-confirmed trust revocation, fail-closed trust erase retry, collection insert/remove/compact persistence, and the vault monotonic-generation commit.

The public vault erase API no longer accepts only internal state. It requires a separate vault-bound authorization containing a nonzero physical-session identifier and canonical human confirmation. The trust revocation API now requires a separate authorization bound to the active generation, a nonzero physical-session identifier, and both core and nucleus confirmations. The raw ESP32 console `send` command no longer calls `link_send` or queues a frame because it has no assurance snapshot.

## Promotion chain

The C11-bound memory-vault case is promoted only if all of the following are true:

| Evidence layer | Required result |
|---|---|
| Finite abstract and concrete machines | `VERIFIED` |
| State and policy refinement | `REFINED` |
| Call-path audit | `PASS` / covered |
| Structural source extraction | `EXTRACTED_MATCH` |
| HCAE sink inventory | `PASS` |
| Lexical sink audit | `PASS` |
| Clang AST structural audit | `PASS` |
| Finite critical-effect candidate queue | `PASS` |
| Assurance certificate | `ASSURED` |

The candidate scanner is intentionally bounded. It scans a finite sensitive-verb family in `firmware/core` and `firmware/net`; every candidate must be HCAE-profiled or have an exact source/function/operation disposition. It is not a universal C11 effect system.

## Adversarial evidence

The corpus contains contraproves for removed guards, inverted semantic predicates, guards after sinks, macro-expanded sinks, indirect sink calls, unavailable Clang, omitted inventory entries, registry/profile mismatch, unreviewed candidates, divergent extraction, missing HCAE guards, invalid scope, and certificate promotion with missing evidence.

The new destructive-authority tests prove that invalid vault erase authorization is rejected before the backend is called, and that one-sided trust revocation cannot erase storage or reset replay state. The existing backend-failure tests still prove that failed erasure leaves the relevant object blocked or revoked rather than claiming success.

## Reproducible host result

The final local run produced:

| Gate | Result |
|---|---|
| Research suite | `227 passed`, `1 skipped` |
| Firmware and `prove.sh` | `ALL INVARIANTS HOLD` |
| Candidate queue | `CRITICAL_EFFECT_CANDIDATES=PASS` |
| C11 structural audit | `13` sinks, `C11_STRUCTURAL_AUDIT=PASS` |
| HCAE inventory | `SINK_INVENTORY=PASS` |
| Software provenance | Valid in strict mode |
| Real memory-vault certificate | `ASSURED` |

The serialized real case reports `assurance_scope = c11-bound`, `inventory_verdict = PASS`, `sink_audit_verdict = PASS`, `c11_structural_verdict = PASS`, and `candidate_verdict = PASS`.

## What remains unproved

This evidence does not prove secure boot, debug lock, eFuse or root-material protection, flash atomicity under power loss, brownout behavior, physical-session authenticity, energy, latency, endurance, radio behavior in the environment, or sensor correctness. It also does not prove general C11 semantic completeness: the AST auditor deliberately returns `UNKNOWN` outside its supported conservative subset.

The correct conclusion is therefore:

> The HERUS host assurance chain is stronger and more falsifiable, and the C11-bound case is promoted only through the closed finite evidence boundary. The system is ready for later physical falsification, not declared ready for critical deployment.
