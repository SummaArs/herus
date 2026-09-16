# HERUS — State-of-the-Art Gap Matrix (2026)

This is a research comparison, not a performance ranking.

| Capability | Frontier direction | HERUS status | Gap to close |
|---|---|---|---|
| Closed-loop reasoning/action | ReAct-style interleaving | bounded proposal/action cycle exists | connect open perception while preserving verifier boundary |
| Persistent skills | Voyager-style reusable skill library | finite verified Skills already exist | host-independent procedural representation + retrieval |
| Structured skill composition | SkillComposer-style subset/order/count selection | bounded composition exists | typed dependency graph + learned composition policy |
| Robot skill discovery | ASPIRE-style iterative exploration and repair | bounded host discovery exists | real execution traces, diagnosis, repair and transfer |
| System identification | ICWM-style adaptation to unseen system configuration | host probing exists | learn continuous dynamics and uncertainty from short interactions |
| World modeling | Dreamer/modern world-model direction | discrete transition model only | predictive latent/structured model + counterfactual separation |
| Self-grounded body model | recent self-grounded action/world models | Host Model is symbolic | morphology, kinematics, sensor/actuator grounding |
| Continual memory | modular-memory direction | persistent verified skill memory | multi-timescale memory, consolidation, forgetting policy |
| Runtime safety | predictive failure monitoring | fail-closed evidence/authority gates | learned risk model and pre-action prediction |
| Open-world adaptation | emerging open-ended agents/robotics | finite open-world tests | generated environments, novel tasks and independent evaluation |
| Physical efficiency | embedded/edge constraints | planned, not yet physically demonstrated | measure ESP32 latency/RAM/energy/radio/thermal behavior |

## The architectural hypothesis beyond the frontier

The project should investigate a combination that is not present as a single established system in the cited literature:

```text
persistent identity
      +
modular verified capabilities
      +
active host/system identification
      +
explicit self/world/host models
      +
world-model planning
      +
independent verification
      +
strict authority separation
      +
transfer across heterogeneous hosts
```

The important research question is not whether each component is individually novel. Most are established directions. The question is whether the **combination around a persistent host-independent computational identity** produces measurable transfer, adaptation and efficiency advantages without weakening safety or auditability.

## Experiments required for a credible SOTA comparison

1. Same task, unseen host, renamed actions.
2. Same skill, altered morphology or API.
3. Hidden task compositions not used for skill synthesis.
4. Contradictory, drifting and delayed observations.
5. Limited probe and compute budgets.
6. Partial observability and stochastic effects.
7. Simulator-to-real transfer with measured physical resources.
8. Baselines that receive comparable observation/action budgets.

## Important limitation

A stronger architecture is not demonstrated by architecture diagrams. The only meaningful claims are those backed by reproducible measurements on held-out cases and, for physical claims, physical measurements.
