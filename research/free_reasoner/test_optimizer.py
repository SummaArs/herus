from __future__ import annotations

import unittest
from fractions import Fraction

from .free_reasoner import C, Mul, Prover, V
from .optimizer import Example, InvalidExamples, OptimizationBudget, optimize_examples


class OptimizerTests(unittest.TestCase):
    def test_finds_square_but_requires_independent_proof(self) -> None:
        examples = [
            Example({"x": -2}, 4),
            Example({"x": 0}, 0),
            Example({"x": 3}, 9),
        ]
        result = optimize_examples(
            examples,
            budget=OptimizationBudget(episodes=300, seed=7),
        )
        self.assertTrue(result.found_zero_error)
        self.assertIsNotNone(result.candidate)
        proof = Prover().prove(result.candidate, Mul(V("x"), V("x")))
        self.assertTrue(proof.verified)

    def test_same_seed_is_reproducible(self) -> None:
        examples = [Example({"x": 1}, 2), Example({"x": 2}, 3)]
        budget = OptimizationBudget(episodes=40, seed=11)
        first = optimize_examples(examples, budget=budget)
        second = optimize_examples(examples, budget=budget)
        self.assertEqual(first, second)

    def test_inconsistent_examples_do_not_become_proof(self) -> None:
        examples = [Example({"x": 0}, 1), Example({"x": 0}, 2)]
        result = optimize_examples(
            examples,
            budget=OptimizationBudget(episodes=20, seed=2),
        )
        self.assertFalse(result.found_zero_error)
        self.assertNotEqual(result.exact_error, Fraction(0))

    def test_budget_and_parameters_are_explicit(self) -> None:
        with self.assertRaises(ValueError):
            optimize_examples([], budget=OptimizationBudget(episodes=0))
        with self.assertRaises(InvalidExamples):
            optimize_examples([Example({"y": 1}, 1)])
        with self.assertRaises(ValueError):
            optimize_examples(
                [Example({"x": 1}, 1)],
                budget=OptimizationBudget(epsilon=2.0),
            )

    def test_result_is_inert_and_does_not_execute_text(self) -> None:
        result = optimize_examples(
            [Example({"x": 1}, 1)],
            budget=OptimizationBudget(episodes=5, seed=0),
        )
        self.assertIsNotNone(result.candidate)
        self.assertTrue(hasattr(result.candidate, "op"))
        self.assertFalse(hasattr(result.candidate, "__code__"))


if __name__ == "__main__":
    unittest.main()
