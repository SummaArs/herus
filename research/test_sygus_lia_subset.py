from __future__ import annotations

import unittest

from sygus_lia_subset import synthesize_bounded


GOOD = """(set-logic LIA)
(synth-fun inc ((x Int)) Int
  ((I Int))
  ((I Int (x 1 (+ I I) (- I I)))))
(declare-var x Int)
(constraint (= (inc x) (+ x 1)))
(check-synth)
"""

UNSUPPORTED = """(set-logic LIA)
(synth-fun choose ((x Int)) Int
  ((I Int) (B Bool))
  ((I Int (x 0 (ite B I I))) (B Bool true)))
(declare-var x Int)
(constraint (= (choose x) x))
(check-synth)
"""


class SyGuSLiaSubsetTests(unittest.TestCase):
    def test_finds_candidate_but_labels_bound(self) -> None:
        result = synthesize_bounded(GOOD, lower=-2, upper=2, max_depth=2)
        self.assertEqual(result.status, "BOUNDED_VERIFIED")
        self.assertEqual(result.expression, "(+ 1 x)")
        self.assertEqual(result.reason, "finite_domain_only")
        self.assertGreater(result.checked_points, 0)

    def test_unsupported_grammar_is_unknown(self) -> None:
        result = synthesize_bounded(UNSUPPORTED, lower=0, upper=1)
        self.assertEqual(result.status, "UNKNOWN")
        self.assertIsNone(result.expression)

    def test_no_candidate_is_counterexample_to_bounded_search(self) -> None:
        result = synthesize_bounded(GOOD, lower=-1, upper=1, max_depth=0)
        self.assertEqual(result.status, "COUNTEREXAMPLE")
        self.assertEqual(result.reason, "no_candidate_satisfies_finite_domain")

    def test_budget_is_fail_closed(self) -> None:
        result = synthesize_bounded(GOOD, lower=0, upper=2, max_depth=3, max_candidates=1)
        self.assertEqual(result.status, "UNKNOWN")
        self.assertEqual(result.reason, "candidate_budget_exhausted")


if __name__ == "__main__":
    unittest.main()
