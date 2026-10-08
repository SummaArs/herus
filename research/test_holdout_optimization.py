import unittest
from holdout_optimization import evaluate_fit_holdout
from symbiotic_learning import Feedback, SymbioticLearner, _state


def feedback(example_id, utility, risk=0.0, cost=1.0):
    return Feedback((), 'a', _state({'ok': 1}), _state({'ok': 1}), utility=utility, risk=risk, cost=cost, example_id=example_id)


class HoldoutOptimizationTests(unittest.TestCase):
    def test_fit_and_holdout_are_evaluated_with_disjoint_ids(self):
        result = evaluate_fit_holdout(SymbioticLearner(), [feedback('fit-1', 4), feedback('fit-2', 3)], [feedback('test-1', 2)], grid=(0, 1))
        self.assertEqual(result.status, 'EVALUATED')
        self.assertEqual((result.fit_count, result.holdout_count), (2, 1))
        self.assertAlmostEqual(result.generalization_gap, result.fit_objective - result.holdout_objective)

    def test_overlap_is_blocked_before_optimization(self):
        result = evaluate_fit_holdout(SymbioticLearner(), [feedback('same', 4)], [feedback('same', 2)], grid=(0, 1))
        self.assertEqual(result.status, 'BLOCK')
        self.assertEqual(result.reason, 'fit_holdout_overlap')

    def test_missing_or_duplicate_ids_fail_closed(self):
        with self.assertRaises(ValueError):
            evaluate_fit_holdout(SymbioticLearner(), [feedback('', 4)], [feedback('test', 2)], grid=(0, 1))
        with self.assertRaises(ValueError):
            evaluate_fit_holdout(SymbioticLearner(), [feedback('x', 4), feedback('x', 3)], [feedback('test', 2)], grid=(0, 1))


if __name__ == '__main__':
    unittest.main()
