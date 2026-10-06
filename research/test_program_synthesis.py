import unittest
from program_synthesis import IOExample, SynthesisTask, synthesize


class ProgramSynthesisTests(unittest.TestCase):
    def test_unique_public_solution_passes_hidden_examples(self):
        task = SynthesisTask(
            'add.v1',
            (IOExample((2, 3), 5), IOExample((-1, 4), 3), IOExample((0, 7), 7)),
            (IOExample((9, -2), 7), IOExample((-5, -6), -11)),
        )
        result = synthesize(task)
        self.assertEqual(result.status, 'PROPOSE')
        self.assertEqual(result.expression, 'x + y')
        self.assertTrue(result.hidden_pass)
        self.assertGreater(result.candidates_checked, 0)

    def test_public_fit_rejected_by_hidden_examples(self):
        task = SynthesisTask(
            'difference.v1',
            (IOExample((2, 1), 1), IOExample((5, 2), 3)),
            (IOExample((1, 5), -5),),
        )
        result = synthesize(task)
        self.assertEqual(result.status, 'REJECT_HIDDEN')
        self.assertFalse(result.hidden_pass)

    def test_ambiguous_public_examples_abstain(self):
        task = SynthesisTask('ambiguous.v1', (IOExample((0, 0), 0), IOExample((1, 1), 0)), ())
        result = synthesize(task)
        self.assertEqual(result.status, 'ABSTAIN')
        self.assertIn('ambiguous', result.detail)

    def test_invalid_depth_abstains(self):
        result = synthesize(SynthesisTask('bad.v1', (IOExample((1, 1), 2),), (), 4))
        self.assertEqual(result.status, 'ABSTAIN')

    def test_composed_expression_passes_hidden_holdout(self):
        task = SynthesisTask(
            'sum-times-difference.v1',
            (IOExample((3, 1), 8), IOExample((4, 2), 12), IOExample((5, 3), 16), IOExample((2, 1), 3)),
            (IOExample((6, 1), 35),),
            2,
        )
        result = synthesize(task)
        self.assertEqual(result.status, 'PROPOSE')
        self.assertTrue(result.hidden_pass)
        self.assertGreater(result.candidates_checked, 10)


if __name__ == '__main__':
    unittest.main()
