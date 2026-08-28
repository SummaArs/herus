"""Deterministic, synthetic-free benchmark for the symbolic laboratory.

The cases are formal fixtures, not a language corpus and not HERUS commands.
"""
from __future__ import annotations

from dataclasses import dataclass

from .core import Budget, Signature, Status, Symbol
from .generator import generate
from .engine import EqualitySaturation, KnowledgeBase, Rule, Var
from .core import Term


@dataclass(frozen=True)
class BenchmarkCase:
    name: str
    expected: Status
    observed: Status
    metric: int

    @property
    def passed(self) -> bool:
        return self.expected is self.observed


def demo_signature() -> Signature:
    return Signature(
        [
            Symbol("alice", "Person"),
            Symbol("bob", "Person"),
            Symbol("parent", "Relation", ("Person", "Person")),
            Symbol("related", "Relation", ("Person", "Person")),
            Symbol("seed", "Token"),
            Symbol("alias", "Token", ("Token",)),
            Symbol("canonical", "Token", ("Token",)),
        ]
    )


def run() -> dict[str, object]:
    signature = demo_signature()
    alice = signature.atom("alice").term
    bob = signature.atom("bob").term
    parent = signature.build("parent", [alice, bob]).term
    related = signature.build("related", [alice, bob]).term

    cases: list[BenchmarkCase] = []
    cases.append(BenchmarkCase("typed_build", Status.OK, signature.validate(parent).status, parent.nodes))
    cases.append(BenchmarkCase("unknown_symbol", Status.UNKNOWN_SYMBOL, signature.atom("unknown").status, 0))
    cases.append(BenchmarkCase("wrong_arity", Status.ARITY_ERROR, signature.build("parent", [alice]).status, 0))

    generated = generate(signature, Budget(max_depth=3, max_terms=32))
    generation_status = generated.status if len(generated.terms) > 3 else Status.TYPE_ERROR
    cases.append(BenchmarkCase("bounded_generation", Status.OK, generation_status, len(generated.terms)))

    rule = Rule(
        "parent-implies-related",
        Term("parent", "Relation", (Var("x", "Person"), Var("y", "Person"))),
        Term("related", "Relation", (Var("x", "Person"), Var("y", "Person"))),
    )
    knowledge = KnowledgeBase(signature=signature)
    knowledge.add("BASE", parent)
    derivation_status = knowledge.derive([rule])
    cases.append(BenchmarkCase("declared_derivation", Status.OK, derivation_status, len(knowledge.derivations())))
    if related not in knowledge.facts("BASE"):
        cases[-1] = BenchmarkCase("declared_derivation", Status.TYPE_ERROR, derivation_status, 0)

    contradiction = KnowledgeBase(signature=signature)
    contradiction.add("HYPOTHESIS", parent)
    contradiction_status = contradiction.add("HYPOTHESIS", parent, positive=False).status
    cases.append(BenchmarkCase("local_conflict", Status.CONFLICT, contradiction_status, 1))

    seed = signature.atom("seed").term
    alias = signature.build("alias", [seed]).term
    canonical = signature.build("canonical", [seed]).term
    rewrites = [
        Rule("alias-to-canonical", Term("alias", "Token", (Var("x", "Token"),)), Term("canonical", "Token", (Var("x", "Token"),))),
        Rule("canonical-to-alias", Term("canonical", "Token", (Var("x", "Token"),)), Term("alias", "Token", (Var("x", "Token"),))),
    ]
    saturation = EqualitySaturation(signature).run([alias], rewrites)
    equivalent = any(alias in group and canonical in group for group in saturation.classes)
    saturation_status = saturation.status if equivalent else Status.TYPE_ERROR
    cases.append(BenchmarkCase("bounded_equivalence", Status.OK, saturation_status, len(saturation.terms)))

    bounded = EqualitySaturation(signature, Budget(max_steps=1, max_terms=8)).run(
        [seed], [Rule("grow", Var("x", "Token"), Term("alias", "Token", (Var("x", "Token"),)))]
    )
    cases.append(BenchmarkCase("step_budget", Status.BUDGET_EXCEEDED, bounded.status, bounded.steps))

    return {
        "schema": "herus.generative_lab.benchmark",
        "version": 1,
        "authority": "none",
        "cases": [
            {"name": case.name, "expected": case.expected.value, "observed": case.observed.value, "metric": case.metric, "passed": case.passed}
            for case in cases
        ],
        "passed": sum(case.passed for case in cases),
        "total": len(cases),
        "all_passed": all(case.passed for case in cases),
    }


if __name__ == "__main__":
    import json
    print(json.dumps(run(), indent=2, sort_keys=True))
