import json
import pathlib
import sys
import unittest

ROOT = pathlib.Path(__file__).resolve().parent
sys.path.insert(0, str(ROOT))

from generative_lab import (  # noqa: E402
    Budget,
    EqualitySaturation,
    KnowledgeBase,
    Rule,
    Signature,
    Status,
    Symbol,
    Term,
    Var,
    generate,
)
from generative_lab.benchmark import run as run_benchmark  # noqa: E402


class GenerativeLabTests(unittest.TestCase):
    def setUp(self):
        self.signature = Signature(
            [
                Symbol("alice", "Person"),
                Symbol("bob", "Person"),
                Symbol("parent", "Relation", ("Person", "Person")),
                Symbol("related", "Relation", ("Person", "Person")),
                Symbol("asserted", "Fact", ("Relation",)),
                Symbol("alias", "Token", ("Token",)),
                Symbol("canonical", "Token", ("Token",)),
                Symbol("seed", "Token"),
            ]
        )

    def test_benchmark_is_deterministic_and_green(self):
        result = run_benchmark()
        self.assertTrue(result["all_passed"])
        self.assertEqual(result["passed"], result["total"])
        self.assertEqual(result["authority"], "none")

    def test_contract_is_host_only_and_closed(self):
        contract = json.loads((ROOT / "generative_lab" / "contract.json").read_text())
        self.assertTrue(contract["host_only"])
        self.assertEqual(contract["authority"], "none")
        self.assertIsNone(contract["authority_bridge"])
        for path in (ROOT / "generative_lab").glob("*.py"):
            text = path.read_text()
            self.assertNotIn("import firmware", text)
            self.assertNotIn("subprocess", text)
            self.assertNotIn("exec(", text)

    def test_signature_rejects_unknown_arity_and_types(self):
        self.assertEqual(self.signature.atom("missing").status, Status.UNKNOWN_SYMBOL)
        alice = self.signature.atom("alice").term
        bob = self.signature.atom("bob").term
        self.assertEqual(self.signature.build("parent", [alice]).status, Status.ARITY_ERROR)
        self.assertEqual(self.signature.build("parent", [alice, self.signature.atom("seed").term]).status, Status.TYPE_ERROR)
        parent = self.signature.build("parent", [alice, bob]).term
        malformed = Term("parent", "Token", (alice, bob))
        self.assertEqual(self.signature.validate(parent).status, Status.OK)
        self.assertEqual(self.signature.validate(malformed).status, Status.TYPE_ERROR)

    def test_budget_rejects_deep_terms(self):
        alice = self.signature.atom("alice").term
        bob = self.signature.atom("bob").term
        result = self.signature.build("parent", [alice, bob], Budget(max_depth=0))
        self.assertEqual(result.status, Status.BUDGET_EXCEEDED)

    def test_composition_derives_only_declared_consequence(self):
        alice = self.signature.atom("alice").term
        bob = self.signature.atom("bob").term
        parent = self.signature.build("parent", [alice, bob]).term
        rule = Rule(
            "parent-implies-related",
            Term("parent", "Relation", (Var("x", "Person"), Var("y", "Person"))),
            Term("related", "Relation", (Var("x", "Person"), Var("y", "Person"))),
        )
        kb = KnowledgeBase(signature=self.signature)
        self.assertEqual(kb.add("BASE", parent).status, Status.OK)
        self.assertEqual(kb.derive([rule]), Status.OK)
        related = self.signature.build("related", [alice, bob]).term
        self.assertIn(related, kb.facts("BASE"))
        self.assertEqual(kb.facts("HYPOTHESIS"), frozenset())
        self.assertEqual(len(kb.derivations()), 1)

    def test_contradiction_is_local_and_non_explosive(self):
        alice = self.signature.atom("alice").term
        bob = self.signature.atom("bob").term
        parent = self.signature.build("parent", [alice, bob]).term
        kb = KnowledgeBase(signature=self.signature)
        self.assertEqual(kb.add("HYPOTHESIS", parent).status, Status.OK)
        self.assertEqual(kb.add("HYPOTHESIS", parent, positive=False).status, Status.CONFLICT)
        self.assertIn(parent, kb.facts("HYPOTHESIS"))
        self.assertIn(parent, kb.negated("HYPOTHESIS"))
        self.assertEqual(kb.facts("BASE"), frozenset())
        self.assertNotIn(self.signature.atom("alice").term, kb.facts("HYPOTHESIS"))

    def test_generation_is_compositional_and_bounded(self):
        result = generate(self.signature, Budget(max_depth=3, max_terms=64))
        self.assertEqual(result.status, Status.OK)
        self.assertGreater(len(result.terms), 3)
        self.assertEqual(sum(result.by_depth), len(result.terms))
        self.assertTrue(all(self.signature.validate(term).ok for term in result.terms))

    def test_saturation_groups_equivalent_terms(self):
        seed = self.signature.atom("seed").term
        alias = self.signature.build("alias", [seed]).term
        canonical = self.signature.build("canonical", [seed]).term
        rules = [
            Rule("alias-to-canonical", Term("alias", "Token", (Var("x", "Token"),)), Term("canonical", "Token", (Var("x", "Token"),))),
            Rule("canonical-to-alias", Term("canonical", "Token", (Var("x", "Token"),)), Term("alias", "Token", (Var("x", "Token"),))),
        ]
        result = EqualitySaturation(self.signature).run([alias], rules)
        self.assertEqual(result.status, Status.OK)
        self.assertIn(canonical, result.terms)
        self.assertTrue(any(alias in group and canonical in group for group in result.classes))

    def test_saturation_is_bounded(self):
        seed = self.signature.atom("seed").term
        rule = Rule("self", Var("x", "Token"), Term("alias", "Token", (Var("x", "Token"),)))
        result = EqualitySaturation(self.signature, Budget(max_steps=2, max_terms=3)).run([seed], [rule])
        self.assertEqual(result.status, Status.BUDGET_EXCEEDED)
        self.assertLessEqual(result.steps, 3)


if __name__ == "__main__":
    unittest.main()
