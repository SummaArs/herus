import ast
import unittest
from herus_symbiotic.programming import ProgrammingRequest, ProgrammingSkill


class ProgrammingSkillTests(unittest.TestCase):
    def setUp(self):
        self.skill = ProgrammingSkill()

    def test_python_proposal_is_deterministic_and_parseable(self):
        request = ProgrammingRequest('add bounded JSON parsing', 'python', ('stdlib only',))
        first = self.skill.propose(request)
        second = self.skill.propose(request)
        self.assertEqual(first, second)
        self.assertEqual(first.authority, 'none')
        ast.parse(first.source)
        self.assertIn('malformed input fails closed', first.tests)

    def test_external_effect_request_does_not_receive_authority(self):
        result = self.skill.propose(ProgrammingRequest('deploy the service', 'python', ('review required',)))
        self.assertEqual(result.status, 'PROPOSE_WITH_QUESTIONS')
        self.assertTrue(any(q.kind == 'authority' and q.blocking for q in result.questions))
        self.assertEqual(result.authority, 'none')

    def test_unsupported_language_abstains(self):
        result = self.skill.propose(ProgrammingRequest('build it', 'brainfuck'))
        self.assertEqual(result.status, 'ABSTAIN')
        self.assertEqual(result.authority, 'none')

    def test_import_has_no_execution_side_effect(self):
        result = self.skill.propose(ProgrammingRequest('write a parser', 'c11', ('no allocation',)))
        self.assertEqual(result.status, 'PROPOSE')
        self.assertIn('proposal only', result.source)


if __name__ == '__main__':
    unittest.main()
