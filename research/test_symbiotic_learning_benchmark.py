import unittest
from symbiotic_learning_benchmark import run

class SymbioticLearningBenchmarkTests(unittest.TestCase):
    def test_safe_case_proposes_and_negative_cases_abstain(self):
        results = run()['cases']
        by_name = {row['case']: row for row in results}
        self.assertEqual(by_name['rename']['symbiotic'], 'gesture')
        for name in ('ambiguous', 'missing', 'wrong_context', 'stale'):
            self.assertIsNone(by_name[name]['symbiotic'])
            self.assertEqual(by_name[name]['status'], 'ABSTAIN')

if __name__ == '__main__': unittest.main()
