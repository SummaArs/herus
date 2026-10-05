import unittest

import herus_symbiotic as herus


class PublicApiTests(unittest.TestCase):
    def test_public_import_is_proposal_only(self):
        learner = herus.SymbioticLearner()
        episode = herus.Episode.from_maps({'mode': 0}, 'tap', {'mode': 1})
        self.assertTrue(learner.observe(episode))
        proposal = learner.propose(episode.effect, (episode,))
        self.assertEqual((proposal.status, proposal.action), ('PROPOSE', 'tap'))

    def test_public_meta_api_requires_fresh_evidence(self):
        meta = herus.MetaSymbioticLearner()
        problem = herus.Problem.from_maps({'mode': 1})
        proposal = meta.adapt(problem, ())
        self.assertEqual(proposal.status, 'ABSTAIN')
        self.assertFalse(proposal.fresh_evidence)

    def test_public_symbols_are_present(self):
        for name in ('Episode', 'SymbioticLearner', 'MetaSymbioticLearner', 'Problem'):
            self.assertTrue(hasattr(herus, name), name)


if __name__ == '__main__':
    unittest.main()
