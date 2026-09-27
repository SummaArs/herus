# HERUS Free Symbolic Reasoning Lab

This host-only research track adds a symbolic generator beyond finite retrieval.

It combines exact polynomial normalization, bounded rewrite search, conjecture
discovery by semantic equivalence classes, and enumerative program synthesis.
No generated expression is executed and no generated result gains HERUS authority.

## Run

```bash
python3 -m unittest -v free_reasoner.test_free_reasoner
python3 -m free_reasoner.free_reasoner
```

The engine is deterministic and bounded. `BudgetExceeded`, `NotProved` and
`NoSynthesis` are explicit outcomes; the system never turns a search timeout into
a positive result.

## Research claim

The interesting property is not that the algebra can simplify expressions.
That is old. The experiment is that **new expressions can be generated from a
finite grammar, collapsed by exact semantics, and promoted only after exact
verification**. This creates a symbolic generate -> test -> prove loop without a
language model.

The implementation is intentionally not called universal reasoning. Grounding,
open-world semantics, goals/value, induction over arbitrary structures and
physical-world interaction remain separate research problems.
