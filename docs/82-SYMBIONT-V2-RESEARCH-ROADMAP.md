# HERUS — Symbiont v2 Research Roadmap

This document is the navigation layer for the next research cycle. Existing documents remain the source of truth for their respective contracts; this page defines their order and the new experimental frontier.

## Architecture layers

```text
00  Sovereign core / assurance
01  Semantic IR + representation
02  Self Model + persistent identity
03  Host Model + discovery
04  World Model + evidence
05  Capability Engine
06  Skill compiler / composition
07  Verification / promotion / rollback
08  Language + multimodal proposal layer
09  Multi-host residence + transfer
10  Physical hosts and embedded constraints
11  Open-ended adaptation and self-improvement
```

## Existing HERUS foundation

- `docs/48-ARQUITETURA-FINITA-E-LINGUAGEM.md`: finite representation and language boundary.
- `docs/59-AUTONOMOUS-HOST-DISCOVERY.md`: bounded discovery.
- `docs/64-CRITERIO-SUPERACAO-GOFAI.md`: falsifiable adaptation criterion.
- `docs/65-MODELO-TRIPLO-E-IDENTIDADE-PERSISTENTE.md`: Self/Host/World separation.
- `docs/67-PROVA-SIMBIOSE-REAL-MULTI-HOST.md`: multi-host evidence.
- `docs/68-FREEZE-PRE-HARDWARE.md`: physical-evidence boundary.
- `docs/69-SIM-NEURO-SIMBIOTICO-V1.md`: local neural + symbolic proposal path.
- `docs/77-LABORATORIO-SIMBIOSE-COMPUTACIONAL.md`: bounded host discovery laboratory.
- `docs/81-SIMBIOSE-CONCORRENTE-MULTI-HOST.md`: concurrent multi-host experiment.

## New v2 research core

- `docs/70-SYMBIONT-V2-RESEARCH-ARCHITECTURE.md`: formal research architecture and falsification plan.
- `research/symbiont_v2/core.py`: dependency-free runtime prototype.
- `research/symbiont_v2/sim_hosts.py`: controlled hosts with permuted primitive names.
- `research/symbiont_v2/test_symbiont.py`: software-level identity, verification, quarantine and transfer gates.
- `research/symbiont_v2/benchmark.py`: reproducible benchmark entry point.

## Implementation order

### Phase A — prove the core hypothesis

1. Run the deterministic simulator.
2. Confirm that discovery is based on observations rather than host metadata.
3. Promote only verified skills.
4. Rebind the same identity to a different host.
5. Transfer a skill through an abstract effect contract.
6. Break the evidence model with contradictions and measure abstention.

### Phase B — make the abstraction non-trivial

Replace exact integer deltas with typed semantic effects, preconditions, postconditions, uncertainty intervals and evidence counts. Introduce partial observability and stochastic transitions.

### Phase C — connect Semantic IR

The existing Semantic IR becomes the language between perception/proposals and the capability engine. LLM/VLM outputs remain untrusted proposals until grounded in evidence. The language component must never mint authority.

### Phase D — model-based planning

Add an explicit learned world model. Imagined rollouts are tagged separately from observed transitions. Planning may use both, but promotion requires external evidence.

### Phase E — continual modular memory

Separate fast adaptation from stable memory. New skills are candidates first; successful skills become versioned persistent modules with provenance, compatibility conditions and rollback. This follows the direction of recent modular-memory continual-learning research rather than mutating one monolithic model.

### Phase F — physical grounding

Only after the existing B1–B10 hardware gates, replace the simulator with the LilyGO/ESP32 host. Measure RAM, latency, persistence, radio behavior, energy and failure modes. Do not infer these properties from desktop execution.

### Phase G — open-ended research

Investigate whether the system can discover its own capability gaps, design bounded experiments, create new skills and improve their implementations while remaining within invariant-preserving sandboxes.

## State-of-the-art positioning

The project should not claim to outperform the field globally. The defensible objective is to combine several strong research ideas under a different systems constraint: **persistent host-independent capability contracts with evidence-backed transfer and an explicit authority boundary**.

Relevant literature:

| Area | Representative work | HERUS use |
|---|---|---|
| Reasoning + acting | ReAct, 2022 | closed-loop proposal/observation cycle |
| Lifelong skill accumulation | Voyager, 2023 | reusable skill library |
| World models | DreamerV3, 2023; 2025 embodied-world-model surveys | predictive internal model |
| Embodied agents | Embodied AI Agents: Modeling the World, 2025 | multimodal world/agent modeling |
| Continual learning memory | Modular Memory, 2026 | persistent modular memory |
| Robot skill discovery | ASPIRE, 2026 | execution/failure/skill compounding |
| Structured skill composition | SkillComposer, 2026 | subset/order/count as planning |

The research frontier is therefore not “invent everything from zero.” It is **find out whether these components can be reorganized around a persistent, auditable symbiont identity without sacrificing generality, transfer or efficiency.**

## Required evidence before stronger claims

A claim of general capability requires:

- held-out tasks unknown during development;
- at least two genuinely different hosts;
- renamed/permuted action interfaces;
- contradictory and drifting observations;
- measured probe and compute budgets;
- negative results, not only successful demonstrations;
- independent benchmark generation where practical;
- physical measurements for physical claims.

A successful simulator experiment is a milestone, not a proof of AGI.
