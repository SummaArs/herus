from __future__ import annotations

import unittest
from fractions import Fraction

from .free_reasoner import normalize, polynomial_string
from .structured_terms import (
    Node,
    Program,
    StructuralBudget,
    StructuralBudgetExceeded,
    StructuralError,
    append_node,
    build_reusable_square_plus_square,
    seed_variable,
)


class StructuredTermsTests(unittest.TestCase):
    def test_reuses_one_subterm(self) -> None:
        program = build_reusable_square_plus_square()
        self.assertEqual(str(program), "((x * x) + (x * x))")
        self.assertEqual(program.reachable_nodes, (0, 1, 2))
        self.assertEqual(program.memory_words(), 15)
        self.assertEqual(polynomial_string(normalize(program.materialize())), "2*x^2")

    def test_references_must_point_backward(self) -> None:
        with self.assertRaises(StructuralError):
            Program((Node.apply("add", 0, 0),), 0)

    def test_reference_and_node_budgets_are_fail_closed(self) -> None:
        with self.assertRaises(StructuralBudgetExceeded):
            append_node(seed_variable("x", StructuralBudget(max_nodes=1)), Node.apply("neg", 0))
        with self.assertRaises(StructuralBudgetExceeded):
            Program(
                (Node.variable("x"), Node.apply("add", 0, 0)),
                1,
                StructuralBudget(max_depth=1),
            )

    def test_constants_and_variables_are_exact(self) -> None:
        program = append_node(None, Node.number(Fraction(1, 3)))
        self.assertEqual(program.materialize().op, "#1/3")

    def test_unknown_operator_is_rejected(self) -> None:
        with self.assertRaises(StructuralError):
            Node.apply("execute", 0)


if __name__ == "__main__":
    unittest.main()
