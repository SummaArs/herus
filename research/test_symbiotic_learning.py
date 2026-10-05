import unittest
from symbiotic_learning import Episode, SymbioticLearner, _state

class SymbioticLearningTests(unittest.TestCase):
    def setUp(self):
        self.source = [
            Episode.from_maps({'mode': 0}, 'button_a', {'mode': 1}),
            Episode.from_maps({'volume': 0}, 'button_b', {'volume': 1}),
        ]

    def test_induces_observable_effect_without_authority(self):
        learner = SymbioticLearner()
        for episode in self.source:
            self.assertTrue(learner.observe(episode))
        skills = learner.induce()
        self.assertEqual(len(skills), 2)
        self.assertTrue(all(skill.status == 'CANDIDATE' for skill in skills))
        self.assertFalse(hasattr(learner, 'execute'))

    def test_transfers_by_effect_not_action_name(self):
        learner = SymbioticLearner()
        learner.observe(self.source[0])
        target = [Episode.from_maps({'mode': 0}, 'gesture_double', {'mode': 1})]
        proposal = learner.propose(self.source[0].effect, target)
        self.assertEqual((proposal.status, proposal.action), ('PROPOSE', 'gesture_double'))

    def test_ambiguous_effect_abstains(self):
        learner = SymbioticLearner()
        target = [
            Episode.from_maps({'mode': 0}, 'a', {'mode': 1}),
            Episode.from_maps({'mode': 0}, 'b', {'mode': 1}),
        ]
        self.assertEqual(learner.propose(target[0].effect, target).status, 'ABSTAIN')

    def test_budget_and_risk_fail_closed(self):
        learner = SymbioticLearner(max_observations=1, max_cost=1, max_risk=0)
        self.assertTrue(learner.observe(self.source[0]))
        self.assertFalse(learner.observe(Episode.from_maps({}, 'unsafe', {'x': 1}, cost=1, risk=1)))
        self.assertFalse(learner.observe(self.source[1]))
        proposal = learner.propose(self.source[0].effect, self.source, cost_budget=0)
        self.assertEqual(proposal.reason, 'budget_exhausted')

    def test_rollback_restores_learning_state(self):
        learner = SymbioticLearner()
        snapshot = learner.snapshot()
        learner.observe(self.source[0])
        learner.rollback(snapshot)
        self.assertEqual(learner.version, 0)
        self.assertEqual(learner.induce(), ())

    def test_missing_effect_abstains(self):
        learner = SymbioticLearner()
        proposal = learner.propose(_state({'missing': 1}), self.source)
        self.assertEqual(proposal.reason, 'effect_not_observed')

if __name__ == '__main__':
    unittest.main()
