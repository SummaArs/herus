from __future__ import annotations

import unittest
from fractions import Fraction

from .free_reasoner import parse_simple, normalize
from .optimizer import Example
from .structured_search import structural_search
from .structured_terms import StructuralBudget


class StructuralSearchTests(unittest.TestCase):
    def test_reusable_subterms_reach_nested_composition(self) -> None:
        target = parse_simple("(x*x-x)*(x+x)")
        examples = tuple(
            Example({"x": x}, normalize(target).evaluate({"x": Fraction(x)}))
            for x in (-2, -1, 0, 1, 2)
        )
        result = structural_search(
            examples,
            StructuralBudget(max_nodes=8, max_depth=8),
            max_evaluations=40000,
        )
        self.assertIsNotNone(result.candidate)
        self.assertEqual(result.train_error, 0)
        self.assertEqual(
            normalize(result.candidate.materialize()),
            normalize(target),
        )

    def test_budget_is_fail_closed(self) -> None:
        result = structural_search(
            (Example({"x": 0}, 1),),
            StructuralBudget(max_nodes=4, max_depth=4),
            max_evaluations=2,
        )
        self.assertEqual(result.evaluations, 2)
        self.assertTrue(result.exhausted)


if __name__ == "__main__":
    unittest.main()
