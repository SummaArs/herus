import unittest
from herus_symbiotic.programming_evidence import EvidenceRecord, ProgrammingObligation, build_programming_ledger


class ProgrammingEvidenceTests(unittest.TestCase):
    def test_closes_only_when_all_cases_and_authority_match(self):
        obligation = ProgrammingObligation('parser.v1', ('positive', 'negative', 'malformed'), 'test', ('e0', 'e1', 'e2'))
        evidence = tuple(EvidenceRecord(f'e{i}', kind, 'test', f'digest-{i}') for i, kind in enumerate(('positive', 'negative', 'malformed')))
        result = build_programming_ledger((obligation,), evidence)
        self.assertEqual(result.status, 'closed')
        self.assertEqual(result.open_obligations, ())
        self.assertEqual(len(result.closure_digest), 64)

    def test_missing_case_stays_open_and_unknown(self):
        obligation = ProgrammingObligation('parser.v1', ('positive', 'negative', 'malformed'), 'test', ('e1',))
        evidence = (EvidenceRecord('e1', 'positive', 'test', 'digest'),)
        result = build_programming_ledger((obligation,), evidence)
        self.assertEqual(result.status, 'open')
        self.assertIn('parser.v1', result.open_obligations)
        self.assertIn('parser.v1:missing:malformed', result.unknowns)

    def test_incomparable_authority_never_closes(self):
        obligation = ProgrammingObligation('deploy.v1', ('positive',), 'process', ('e1',))
        evidence = (EvidenceRecord('e1', 'positive', 'unit-test', 'digest'),)
        result = build_programming_ledger((obligation,), evidence)
        self.assertEqual(result.status, 'open')
        self.assertIn('deploy.v1:incomparable:e1', result.unknowns)


if __name__ == '__main__':
    unittest.main()
