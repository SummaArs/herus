from __future__ import annotations

import json
import time

from .free_reasoner import (
    DiscoveryBudget,
    Prover,
    SearchBudget,
    SynthesisBudget,
    discover_conjectures,
    parse_simple,
    synthesize_linear_or_polynomial,
)


def main() -> None:
    t0 = time.perf_counter()
    proof = Prover(
        budget=SearchBudget(max_steps=30, max_states=25000, max_depth=10)
    ).prove(
        parse_simple("(a+b)*(c+d)"),
        parse_simple("a*c+a*d+b*c+b*d"),
        "held-out distributive expansion",
    )
    candidates = discover_conjectures(
        DiscoveryBudget(max_depth=2, max_terms=120, max_pairs=3000)
    )
    examples = tuple(({"x": x}, x * x) for x in range(-3, 4))
    synthesized = synthesize_linear_or_polynomial(
        ("x",), examples, SynthesisBudget(max_depth=2, max_terms=1000)
    )
    elapsed_ms = (time.perf_counter() - t0) * 1000
    report = {
        "contract": "host-only; exact algebra; bounded; inert",
        "proof_verified": proof.verified,
        "proof_steps": proof.cost,
        "discovered_candidates": len(candidates),
        "synthesized": str(synthesized),
        "runtime_ms": round(elapsed_ms, 3),
    }
    print(json.dumps(report, sort_keys=True))
    if not proof.verified or str(synthesized) != "(x * x)":
        raise SystemExit(1)


if __name__ == "__main__":
    main()
