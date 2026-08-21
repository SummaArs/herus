# Academic findings round 3 — misattribution, implicit authority and memory governance

## Sources read

1. Ahad et al., **The Misattribution Gap: When Memory Poisoning Looks Like Model Failure in Agentic AI Systems**, arXiv:2605.22842, submitted 12 May 2026. URL: https://arxiv.org/abs/2605.22842
2. Lin et al., **A Survey on Long-Term Memory Security in LLM Agents: Attacks, Defenses, and Governance Across the Memory Lifecycle**, arXiv:2604.16548v2, revised 11 June 2026. URL: https://arxiv.org/abs/2604.16548

## Findings from Ahad et al.

The paper defines a **Misattribution Gap**: memory-layer attacks can generate behavior that looks like model failure, causing defenders to apply the wrong remediation. It names **Semantic Norm Drift (SND)** as a path distinct from emergent misalignment and collusion. The described mechanism is a policy-formatted document entering a shared vector store through normal upload, later returning as trusted system context after provenance is lost through a **Trust Laundering Chain**.

The abstract reports 64 documented failures, zero detections by four safety classifiers across 510 checkpoints, explicit citation of the injected document as normative authority in 59 of 65 valid cases, no trigger/model access/repeated interaction requirement, full effect within five sessions, and indefinite persistence. It proposes **Counterfactual Composition Testing** to identify the causal entry and **Memory-Persistent Information-Flow Control** at the cross-session boundary. These are claims from an arXiv preprint currently marked as under review; they are not treated as independently reproduced facts.

HERUS consequence: every semantic offer must carry a non-launderable source identity, and the system must distinguish evidence, preference, policy and authority. A memory record citing a policy-like phrase must not become an action permission merely because retrieval promoted it to context. The redteam should remove provenance, replace source role, or route a memory through a derived record and demand detection.

## Findings from Lin et al.

The survey describes persistent memory as different from ordinary input security because it is **persistent, stateful and propagating**. Its lifecycle has six phases: Write, Store, Retrieve, Execute, Share & Propagate, and Forget & Rollback. It groups objectives into Integrity, Confidentiality, Availability and Governance.

It proposes Verifiable Memory Governance and argues that robust long-term-memory security cannot be retrofitted only at retrieval or execution. It must be anchored in storage-time provenance, versioning and policy-aware retention. This aligns with AGSC/AGSC-D but exposes a remaining requirement: provenance must survive derivation, sharing and rollback, not merely be checked at the initial write.

HERUS consequence: add tests for **lineage conservation**, **role conservation**, **cross-principal propagation**, **rollback completeness** and **post-revocation reintroduction**. A successful deletion must remove not only the original record but every derived capsule that could recreate its authority.

## Hypotheses to test next

| ID | Hypothesis | Observable failure |
|---|---|---|
| ATTR-01 | A derived memory cannot become more authoritative than its least-authoritative ancestor | Authority mask increases after derivation |
| ATTR-02 | A source-role substitution is detected | Observation is relabeled as policy or action |
| ATTR-03 | Retrieval ranking cannot launder provenance | High relevance does not remove lineage |
| ATTR-04 | Revocation propagates through all descendants | Revoked source remains actionable through a derived record |
| ATTR-05 | Cross-principal sharing preserves authority boundaries | Core-fed knowledge gains local action authority |
| ATTR-06 | Counterfactual removal identifies the causal memory | Removing a causal record does not change the offer or is not flagged |

## Limits

These sources concern LLM-agent systems and do not prove behavior of the HERUS C11 host simulator or future hardware. Their reported percentages and corpus claims remain literature claims requiring replication before being used as HERUS results.

## Findings from Zhang et al.

The paper argues that semantic similarity is an inadequate trust boundary for personal-agent memory. A related memory may still be contextually inappropriate and create cross-domain leakage, sycophancy, tool-call drift or memory-induced jailbreaks. It frames memory as a durable control channel that can reshape task interpretation and tool execution. Its proposed MemGate is a learned query-conditioned admission layer; the HERUS will not import that neural component, but the architectural lesson is directly useful: **retrieval admission must be policy- and context-conditioned, not similarity-only**.

HERUS consequence: implement a deterministic admission predicate over authority scope, purpose, epoch, freshness, conflict, source role and physical-action boundary. Similarity may rank candidates, but it cannot raise trust, change role or grant action.

## Findings from Wang et al.

The survey defines execution provenance as a typed graph of an agent execution and evidence tracing as a projection onto evidence-support relations. It connects retrieval grounding, claim support, tool-use safety, memory lineage, observability, debugging, audit and recovery. The key design implication for HERUS is that a flat provenance ID is not enough for causal diagnosis: the system should retain typed edges such as `derived-from`, `supports`, `contradicts`, `retrieved-for` and `authorized-by` while keeping action authority separate.

HERUS consequence: the next guard should test graph conservation. A derived capsule may support an offer but cannot inherit `authorized-by` unless a local physical confirmation creates a new edge. Removing a causal ancestor should alter or invalidate the offer, and a provenance mismatch should cause abstention rather than silent fallback.

## Revised hypotheses

| ID | Hypothesis | Observable failure |
|---|---|---|
| ATTR-01 | A derived memory cannot become more authoritative than its least-authoritative ancestor | Authority mask increases after derivation |
| ATTR-02 | A source-role substitution is detected | Observation is relabeled as policy or action |
| ATTR-03 | Retrieval ranking cannot launder provenance | High relevance does not remove lineage |
| ATTR-04 | Revocation propagates through all descendants | Revoked source remains actionable through a derived record |
| ATTR-05 | Cross-principal sharing preserves authority boundaries | Core-fed knowledge gains local action authority |
| ATTR-06 | Counterfactual removal identifies the causal memory | Removing a causal record does not change the offer or is not flagged |
| ATTR-07 | Similarity-only selection cannot bypass the admission predicate | A highly similar stale/conflicting record is admitted |
| ATTR-08 | Provenance graph edges are typed and conserved | `supports` silently becomes `authorized-by` |
