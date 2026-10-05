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
        for episode in self.source: self.assertTrue(learner.observe(episode))
        skills = learner.induce()
        self.assertEqual(len(skills), 2)
        self.assertTrue(all(skill.status == 'CANDIDATE' for skill in skills))
        self.assertFalse(hasattr(learner, 'execute'))

    def test_transfers_by_effect_not_action_name(self):
        learner = SymbioticLearner(); learner.observe(self.source[0])
        target = [Episode.from_maps({'mode': 0}, 'gesture_double', {'mode': 1})]
        proposal = learner.propose(self.source[0].effect, target)
        self.assertEqual((proposal.status, proposal.action), ('PROPOSE', 'gesture_double'))

    def test_ambiguous_effect_abstains(self):
        learner = SymbioticLearner()
        target = [Episode.from_maps({'mode': 0}, 'a', {'mode': 1}), Episode.from_maps({'mode': 0}, 'b', {'mode': 1})]
        self.assertEqual(learner.propose(target[0].effect, target).status, 'ABSTAIN')

    def test_budget_and_risk_fail_closed(self):
        learner = SymbioticLearner(max_observations=1, max_cost=1, max_risk=0)
        self.assertTrue(learner.observe(self.source[0]))
        self.assertFalse(learner.observe(Episode.from_maps({}, 'unsafe', {'x': 1}, cost=1, risk=1)))
        self.assertFalse(learner.observe(self.source[1]))
        self.assertEqual(learner.propose(self.source[0].effect, self.source, cost_budget=0).reason, 'budget_exhausted')

    def test_rollback_restores_learning_state(self):
        learner = SymbioticLearner(); snapshot = learner.snapshot(); learner.observe(self.source[0]); learner.rollback(snapshot)
        self.assertEqual((learner.version, learner.induce()), (0, ()))

    def test_missing_effect_abstains(self):
        learner = SymbioticLearner()
        self.assertEqual(learner.propose(_state({'missing': 1}), self.source).reason, 'effect_not_observed')

    def test_context_prevents_cross_context_transfer(self):
        learner = SymbioticLearner()
        target = [Episode.from_maps({'mode': 0}, 'silent', {'mode': 1}, context={'channel': 1})]
        proposal = learner.propose(target[0].effect, target, context=_state({'channel': 2}))
        self.assertEqual(proposal.reason, 'effect_not_observed')

    def test_temporal_drift_abstains(self):
        learner = SymbioticLearner(max_age=2)
        target = [Episode.from_maps({'mode': 0}, 'old_action', {'mode': 1}, step=1), Episode.from_maps({'mode': 0}, 'old_action', {'mode': 1}, step=9)]
        for episode in target: learner.observe(episode)
        self.assertEqual(learner.induce()[0].status, 'ABSTAIN')
        self.assertEqual(learner.induce()[0].reason, 'temporal_drift')

    def test_negative_evidence_is_never_promoted(self):
        learner = SymbioticLearner()
        negative = Episode.from_maps({'mode': 0}, 'unsafe', {'mode': 0}, outcome='negative')
        self.assertTrue(learner.observe(negative))
        self.assertEqual(learner.induce(), ())

if __name__ == '__main__': unittest.main()
