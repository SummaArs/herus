# HERUS Symbiont v2 — Research Architecture

## 1. Research question

Can a host-independent computational identity acquire verified capabilities from interaction with a previously unknown host, preserve those capabilities as abstract contracts, and ground them on another host whose concrete interfaces and action names differ?

This is a falsifiable systems question. It is not an assertion that HERUS is AGI.

## 2. Architecture

```text
                         PERSISTENT HERUS IDENTITY
                  ┌─────────────────────────────────┐
                  │ lineage / verified skills       │
                  │ long-term semantic memory       │
                  │ invariants / provenance         │
                  └────────────────┬────────────────┘
                                   │
                      HOST-INDEPENDENT CONTRACTS
                                   │
        ┌──────────────────────────┼──────────────────────────┐
        │                          │                          │
  perception/IR               reasoning                 capability engine
        │                          │                          │
        └──────────────────────────┼──────────────────────────┘
                                   │
                         disposable HOST CONTEXT
                  ┌────────────────┼───────────────────┐
                  │                │                   │
             Host Model        World Model        evidence ledger
                  │                │                   │
                  └────────────────┼───────────────────┘
                                   │
                              HOST ADAPTER
                                   │
          PC / simulator / wearable / robot / sensor / gateway
```

The key design law is: **capability is not authority**. Discovery and learning may create evidence and proposals; they never grant permission to control an external system.

## 3. Cognitive loop

`observe -> represent -> hypothesize -> probe/act -> observe -> compare -> update -> verify -> promote -> reuse`

The loop is deliberately closed around external feedback. A generated explanation is never treated as evidence merely because it is plausible.

## 4. Three-model separation

### Self Model
What the persistent HERUS identity is, what verified skills it owns, what invariants constrain it, and what provenance supports each capability.

### Host Model
What the current body exposes: resources, actions, state channels, constraints and revision/epoch.

### World Model
What actions empirically do on the current host. Contradictory transitions are quarantined rather than overwritten.

This separation permits a `bind()` operation that changes the body while preserving identity and verified abstract skills while deleting host-local observations and models.

## 5. Skill contract

An active skill is not a code blob. In v2 it is a small, inspectable contract containing:

- abstract goal;
- primitive action sequence used in the source host;
- abstract effect signatures;
- maximum length;
- evidence digests;
- verification status;
- source-host provenance.

The first implementation uses discrete integer state and exact effect signatures because they make the first hypothesis easy to falsify. This must not be mistaken for a solution to continuous robot control.

## 6. Why effect signatures are only a bridge

Matching `delta=(x,+1)` across two hosts demonstrates one narrow form of host independence, but it does not solve semantic grounding. A real robot can have different state coordinate systems, continuous dynamics, delays, hidden variables, stochastic effects and safety constraints.

The next abstraction layer is a typed effect schema:

```text
Effect {
    predicate: MOVE
    args: [BODY, FORWARD]
    preconditions: ...
    expected_change: interval/distribution
    confidence: ...
    evidence: ...
}
```

Grounding then becomes an explicit inference problem rather than string matching.

## 7. Research ladder

### R1 — Verified discovery
Discover action/effect structure on an unknown simulator host under a probe budget.

### R2 — Persistent capability
Promote only independently verified skills into persistent memory.

### R3 — Cross-host transfer
Rename and rearrange primitive interfaces while preserving an abstract task and measure whether the same skill contract can be grounded.

### R4 — Hidden-task generalization
Hold out goal combinations and host variants from development. Do not allow benchmark cases to be synthesized into the test set.

### R5 — Continuous / partially observable hosts
Replace exact discrete transitions with uncertainty intervals or distributions and require abstention when confidence is insufficient.

### R6 — Semantic grounding
Connect the existing HERUS Semantic IR as the proposal/representation layer. The IR must not create authority.

### R7 — Model-based planning
Add a learned world model for counterfactual rollouts, keeping imagined transitions distinct from observed evidence.

### R8 — Open-ended skill growth
Add bounded skill composition and search. A skill is promoted only after held-out and adversarial verification.

### R9 — Physical host
Move from simulator to the LilyGO host only after the existing hardware gates are satisfied. Measure latency, memory, radio, persistence, energy and failure behavior rather than inferring them from software.

## 8. State-of-the-art alignment

The design intentionally borrows mechanisms supported by recent research without collapsing them into one monolithic learned policy:

- ReAct motivates an observation/reasoning/action loop.
- Voyager motivates persistent, reusable skill libraries.
- World-model research motivates explicit prediction of future states for planning.
- Recent continual-learning work motivates modular memory rather than relying on one mutable parameter set.
- Recent robotics skill-discovery systems such as ASPIRE motivate iterative execution, failure diagnosis, reusable skills and transfer across embodiments.
- Recent work on structured skill composition shows that selecting a subset, count and ordering of skills is itself a non-trivial planning problem.

References:

- ReAct: https://arxiv.org/abs/2210.03629
- Voyager: https://arxiv.org/abs/2305.16291
- DreamerV3: https://arxiv.org/abs/2307.22203
- Embodied AI Agents: Modeling the World: https://arxiv.org/abs/2506.22355
- Modular Memory is the Key to Continual Learning Agents: https://arxiv.org/abs/2603.01761
- ASPIRE: Agentic /Skills Discovery for Robotics: https://arxiv.org/abs/2607.00272
- Generative Skill Composition for LLM Agents: https://arxiv.org/abs/2606.32025

These papers establish relevant components and directions; none establishes the full HERUS hypothesis.

## 9. Falsification tests

The project must actively try to break itself.

### Test A — name permutation
Rename every primitive action in the target host. Transfer must not use names.

### Test B — distractor actions
Add actions that produce plausible but task-irrelevant changes. The planner must avoid false promotion.

### Test C — contradictory sensor evidence
Inject inconsistent observations. The world model must quarantine the contradiction and abstain rather than overwrite history.

### Test D — host drift
Change an action's effect after promotion. The old capability must fail verification instead of silently adapting its claim.

### Test E — budget pressure
Reduce probe budget until transfer fails. Report the frontier instead of hiding the failure.

### Test F — unseen composition
Construct test goals by combining skills that were never jointly observed during development.

## 10. Metrics

Report at minimum:

`discovery_success`, `verified_skill_rate`, `false_promotion_rate`, `transfer_success`, `probe_cost`, `steps_to_goal`, `abstention_rate`, `contradiction_detection`, `memory_growth`, `host_rebind_integrity`.

Every benchmark result must include the code revision, benchmark seed/configuration, held-out set identifier and negative results.

## 11. What counts as evidence

A green unit test proves a software contract. It does not prove general intelligence. A simulator transfer proves simulator transfer. It does not prove robot transfer. A robot result proves one physical configuration, not arbitrary hosts.

The research claim should therefore grow only when the evidence grows.
