from __future__ import annotations

import unittest

from .compare_search import error, prove_candidate
from .free_reasoner import parse_simple, normalize
from .optimizer import Example
from .search_methods import beam_search, mcts_search


class SearchMethodsTests(unittest.TestCase):
    def setUp(self) -> None:
        target = parse_simple("x*x+x")
        self.target = target
        self.examples = tuple(
            Example({"x": x}, normalize(target).evaluate({"x": x}))
            for x in (-2, -1, 0, 1, 2)
        )

    def test_beam_finds_and_does_not_prove_by_itself(self) -> None:
        result = beam_search(self.examples, beam_width=24, max_evaluations=768)
        self.assertIsNotNone(result.candidate)
        self.assertEqual(result.train_error, 0)
        self.assertTrue(prove_candidate(result.candidate, self.target))

    def test_mcts_is_bounded_and_reproducible(self) -> None:
        first = mcts_search(self.examples, simulations=128, seed=3)
        second = mcts_search(self.examples, simulations=128, seed=3)
        self.assertEqual(first, second)
        self.assertLessEqual(first.evaluations, 128 * 6)

    def test_sparse_data_is_not_generalization(self) -> None:
        sparse = (Example({"x": 0}, 0),)
        result = beam_search(sparse, max_depth=1, beam_width=1, max_evaluations=3)
        self.assertEqual(result.train_error, 0)
        self.assertIsNotNone(result.candidate)
        # The hidden target for this test is 2*x, not the candidate's target.
        self.assertNotEqual(error(result.candidate, (Example({"x": 3}, 6),)), 0)


if __name__ == "__main__":
    unittest.main()
