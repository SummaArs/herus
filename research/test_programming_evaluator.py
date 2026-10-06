import unittest
from programming_evaluator import ProgrammingTask, evaluate_python_candidate

RUNNER = '''
import importlib.util
spec = importlib.util.spec_from_file_location("candidate", __CANDIDATE__)
module = importlib.util.module_from_spec(spec)
spec.loader.exec_module(module)
assert module.solve(2, 3) == 5
assert module.solve(-1, 1) == 0
'''


class ProgrammingEvaluatorTests(unittest.TestCase):
    def test_correct_candidate_passes_with_isolated_authority(self):
        result = evaluate_python_candidate(
            ProgrammingTask('add.v1', 'candidate.py', RUNNER),
            'def solve(a, b):\n    return a + b\n',
        )
        self.assertEqual(result.status, 'pass')
        self.assertEqual(result.authority, 'isolated-subprocess')
        self.assertEqual(result.exit_code, 0)

    def test_wrong_candidate_fails(self):
        result = evaluate_python_candidate(
            ProgrammingTask('add.v1', 'candidate.py', RUNNER),
            'def solve(a, b):\n    return a - b\n',
        )
        self.assertEqual(result.status, 'fail')
        self.assertNotEqual(result.exit_code, 0)

    def test_nonterminating_candidate_times_out(self):
        result = evaluate_python_candidate(
            ProgrammingTask('loop.v1', 'candidate.py', 'import importlib.util\nwhile True: pass', 0.1),
            'def solve(a, b):\n    return a + b\n',
        )
        self.assertEqual(result.status, 'timeout')
        self.assertIsNone(result.exit_code)

    def test_malformed_candidate_is_rejected_without_execution(self):
        result = evaluate_python_candidate(
            ProgrammingTask('syntax.v1', 'candidate.py', RUNNER),
            'def solve(:\n    pass\n',
        )
        self.assertEqual(result.status, 'candidate_syntax_error')

    def test_unbounded_timeout_is_rejected(self):
        result = evaluate_python_candidate(
            ProgrammingTask('bad.v1', 'candidate.py', RUNNER, 31),
            'def solve(a, b): return a + b\n',
        )
        self.assertEqual(result.status, 'invalid_request')


if __name__ == '__main__':
    unittest.main()
