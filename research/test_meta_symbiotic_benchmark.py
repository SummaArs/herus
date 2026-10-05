import unittest
from meta_symbiotic_benchmark import run

class MetaSymbioticBenchmarkTests(unittest.TestCase):
    def test_history_improves_order_without_authority(self):
        result = run()
        self.assertTrue(result['improvement'])
        self.assertEqual(result['candidate_count_before'], result['candidate_count_after'])
        self.assertTrue(result['fresh_evidence_required'])
        self.assertFalse(result['authority_granted'])

if __name__ == '__main__': unittest.main()
