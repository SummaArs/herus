import unittest
from fractions import Fraction

from .free_reasoner import (
    V, Mul, normalize, parse_simple, polynomial_string,
    Prover, SearchBudget, NotProved,
    discover_conjectures, DiscoveryBudget,
    synthesize_linear_or_polynomial, SynthesisBudget,
    verify_equation_on_grid,
)


class FreeReasoningTests(unittest.TestCase):
    def test_exact_ring_normalization(self):
        left = parse_simple("(a+b)*(c+d)")
        right = parse_simple("a*c+a*d+b*c+b*d")
        self.assertEqual(normalize(left), normalize(right))
        self.assertEqual(polynomial_string(normalize(left)), "a*c + a*d + b*c + b*d")

    def test_prover_rejects_false_identity(self):
        with self.assertRaises(NotProved):
            Prover().prove(parse_simple("a+b"), parse_simple("a*b"))

    def test_prover_emits_certificate(self):
        proof = Prover(budget=SearchBudget(max_steps=16, max_states=8000)).prove(
            parse_simple("a*(b+c)"), parse_simple("a*b+a*c"), "left distributivity"
        )
        self.assertTrue(proof.verified)
        self.assertEqual(proof.semantic_left, proof.semantic_right)

    def test_parser_and_fraction(self):
        self.assertEqual(normalize(parse_simple("(3/2)*x - 1")).evaluate({"x": 2}), Fraction(2))

    def test_discovery_is_generative(self):
        cands = discover_conjectures(DiscoveryBudget(max_depth=2, max_terms=120, max_pairs=3000))
        self.assertGreater(len(cands), 0)
        self.assertTrue(any(c.left != c.right for c in cands))
        self.assertTrue(any("*" in c.statement() for c in cands))

    def test_synthesis_square(self):
        examples = [({"x": x}, x * x) for x in range(-3, 4)]
        candidate = synthesize_linear_or_polynomial(
            ("x",), examples, SynthesisBudget(max_depth=2, max_terms=1000)
        )
        self.assertEqual(normalize(candidate), normalize(Mul(V("x"), V("x"))))

    def test_counterexample_verification(self):
        ok, ce = verify_equation_on_grid(
            parse_simple("x*x"), parse_simple("x+x"), ("x",), -3, 3
        )
        self.assertFalse(ok)
        self.assertEqual(ce, {"x": -3})

    def test_budget_is_explicit(self):
        with self.assertRaises(Exception):
            discover_conjectures(DiscoveryBudget(max_depth=2, max_terms=2, max_pairs=10))


if __name__ == "__main__":
    unittest.main(verbosity=2)
