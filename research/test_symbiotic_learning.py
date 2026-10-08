import unittest
from symbiotic_learning import Episode, Feedback, HostContract, SymbioticLearner, UtilityWeights, _state, _wilson_lower

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

    def test_proposal_exposes_reproducible_evidence_trace(self):
        learner = SymbioticLearner()
        episode = self.source[0]
        proposal = learner.propose(episode.effect, [episode])
        self.assertEqual(proposal.status, 'PROPOSE')
        self.assertEqual(proposal.evidence_count, len(proposal.evidence_ids))
        self.assertTrue(proposal.evidence_ids[0])
        self.assertIn('episódio', proposal.explanation)

    def test_induced_hypothesis_exposes_evidence_trace(self):
        learner = SymbioticLearner()
        self.assertTrue(learner.observe(self.source[0]))
        hypothesis = learner.induce()[0]
        self.assertEqual(hypothesis.observations, len(hypothesis.evidence_ids))
        self.assertTrue(hypothesis.evidence_ids[0])

    def test_empty_action_is_rejected_fail_closed(self):
        learner = SymbioticLearner()
        self.assertFalse(learner.observe(Episode.from_maps({}, '', {'x': 1})))

    def test_negative_risk_is_rejected_fail_closed(self):
        learner = SymbioticLearner()
        self.assertFalse(learner.observe(Episode.from_maps({}, 'unsafe', {'x': 1}, risk=-1)))

    def test_confidence_is_conservative_for_small_evidence(self):
        self.assertLess(_wilson_lower(1, 1), 1000)
        self.assertGreater(_wilson_lower(10, 10), _wilson_lower(1, 1))

    def test_objective_penalizes_risk_cost_authority_and_evidence(self):
        learner = SymbioticLearner()
        score = learner.objective(utility=10, risk=2, cost=3, authority_violation=1, evidence_deficit=2,
                                  weights=UtilityWeights(risk=2, cost=1, authority=4, evidence=0.5))
        self.assertEqual(score, 10 - 4 - 3 - 4 - 1)

    def test_update_accepts_verified_positive_feedback_and_is_auditable(self):
        learner = SymbioticLearner()
        effect = Episode.from_maps({'mode': 0}, 'button_a', {'mode': 1}).effect
        result = learner.update(Feedback(_state({'mode': 0}), 'button_a', _state({'mode': 1}), effect,
                                         utility=4, provenance='fixture-1', verifier='test'))
        self.assertTrue(result.accepted)
        self.assertEqual(result.status, 'UPDATED')
        self.assertTrue(result.evidence_id)
        self.assertEqual(len(learner.snapshot()[0]), 1)

    def test_update_rejects_authority_violation_without_mutation(self):
        learner = SymbioticLearner()
        result = learner.update(Feedback((), 'unsafe', _state({'x': 1}), _state({'x': 1}), authority_violation=1))
        self.assertFalse(result.accepted)
        self.assertEqual(result.reason, 'authority_violation')
        self.assertEqual(learner.version, 0)

    def test_negative_feedback_blocks_the_same_action_when_target_is_bound(self):
        learner = SymbioticLearner()
        positive = Episode.from_maps({'mode': 0}, 'button_a', {'mode': 1})
        negative = Episode.from_maps({'mode': 0}, 'button_a', {'mode': 0}, outcome='negative', target_effect=positive.effect)
        self.assertTrue(learner.observe(positive))
        self.assertTrue(learner.observe(negative))
        proposal = learner.propose(positive.effect, [positive, negative], current_state=positive.before)
        self.assertEqual(proposal.reason, 'negative_evidence')

    def test_current_state_is_required_for_state_conditioned_proposal(self):
        learner = SymbioticLearner()
        episode = Episode.from_maps({'mode': 0}, 'button_a', {'mode': 1})
        proposal = learner.propose(episode.effect, [episode], current_state=_state({'mode': 99}))
        self.assertEqual(proposal.reason, 'effect_not_observed')

    def test_empty_context_does_not_match_nonempty_context(self):
        learner = SymbioticLearner()
        episode = Episode.from_maps({'mode': 0}, 'button_a', {'mode': 1}, context={'channel': 1})
        proposal = learner.propose(episode.effect, [episode], context=())
        self.assertEqual(proposal.reason, 'effect_not_observed')

    def test_weight_optimizer_is_bounded_deterministic_and_fit_only(self):
        learner = SymbioticLearner()
        feedback = [
            Feedback((), 'safe', _state({'ok': 1}), _state({'ok': 1}), utility=4, risk=0, cost=1, evidence_deficit=0),
            Feedback((), 'uncertain', _state({'ok': 0}), _state({'ok': 1}), utility=1, risk=2, cost=1, evidence_deficit=2),
        ]
        first = learner.optimize_weights(feedback, grid=(0, 1, 2))
        second = learner.optimize_weights(feedback, grid=(0, 1, 2))
        self.assertEqual(first, second)
        self.assertEqual(first.evaluations, 81)
        self.assertEqual(first.fit_count, 2)
        self.assertEqual(learner.version, 0)

    def test_weight_optimizer_blocks_empty_fit(self):
        result = SymbioticLearner().optimize_weights([])
        self.assertEqual(result.status, 'NO_FIT_DATA')
        self.assertEqual(result.evaluations, 0)

    def test_host_contract_limits_proposals_without_granting_authority(self):
        learner = SymbioticLearner()
        episode = Episode.from_maps({'mode': 0}, 'button_a', {'mode': 1})
        host = HostContract('host-b', capabilities=('button_b',), max_cost=4, max_risk=0)
        proposal = learner.propose(episode.effect, [episode], host=host)
        self.assertEqual(proposal.reason, 'effect_not_observed')
        self.assertFalse(hasattr(learner, 'execute'))

    def test_host_contract_blocks_update_outside_capability_budget(self):
        learner = SymbioticLearner()
        feedback = Feedback(_state({'mode': 0}), 'button_a', _state({'mode': 1}), _state({'mode': 1}), utility=3, cost=2, example_id='host-1')
        host = HostContract('host-a', capabilities=('button_a',), max_cost=1, max_risk=0)
        result = learner.update(feedback, host=host)
        self.assertFalse(result.accepted)
        self.assertEqual(result.reason, 'host_contract_violation')
        self.assertEqual(learner.version, 0)

    def test_migration_retains_compatible_evidence_and_quarantines_the_rest(self):
        learner = SymbioticLearner()
        self.assertTrue(learner.observe(Episode.from_maps({}, 'button_a', {'ok': 1}, cost=1)))
        self.assertTrue(learner.observe(Episode.from_maps({}, 'button_b', {'ok': 2}, cost=1)))
        source = HostContract('host-a', capabilities=('button_a', 'button_b'), max_cost=2, max_risk=0)
        target = HostContract('host-b', capabilities=('button_a',), max_cost=1, max_risk=0)
        before = learner.snapshot()
        result = learner.migration_plan(source, target)
        self.assertTrue(result.accepted)
        self.assertEqual(len(result.retained_evidence_ids), 1)
        self.assertEqual(len(result.quarantined_evidence_ids), 1)
        self.assertEqual(learner.snapshot(), before)
        self.assertEqual(result.reason, 'migration_planned_no_authority_transfer')

    def test_destination_revalidates_with_own_feedback_while_quarantine_stays_blocked(self):
        learner = SymbioticLearner()
        learner.observe(Episode.from_maps({}, 'button_b', {'ok': 2}, cost=1))
        source = HostContract('host-a', capabilities=('button_b',), max_cost=2, max_risk=0)
        target = HostContract('host-b', capabilities=('button_a',), max_cost=2, max_risk=0)
        migration = learner.migration_plan(source, target)
        self.assertEqual(len(migration.quarantined_evidence_ids), 1)
        feedback = Feedback((), 'button_a', _state({'ok': 1}), _state({'ok': 1}), utility=2, example_id='host-b-1')
        self.assertTrue(learner.update(feedback, host=target).accepted)
        self.assertEqual(learner.propose(_state({'ok': 2}), learner.snapshot()[0], host=target).reason, 'effect_not_observed')
        self.assertEqual(learner.propose(_state({'ok': 1}), learner.snapshot()[0], host=target).action, 'button_a')

if __name__ == '__main__': unittest.main()
