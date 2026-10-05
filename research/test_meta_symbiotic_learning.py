import unittest
from meta_symbiotic_learning import MetaSymbioticLearner, Problem
from symbiotic_learning import Episode

class MetaSymbioticLearningTests(unittest.TestCase):
    def setUp(self):
        self.problem = Problem.from_maps({'mode': 1}, context={'channel': 1}, host_kind='host-a')
        self.source = [Episode.from_maps({'mode': 0}, 'gesture', {'mode': 1}, context={'channel': 1})]

    def test_verified_solution_is_append_only_reference(self):
        meta = MetaSymbioticLearner()
        proposal = meta.adapt(self.problem, self.source)
        record = meta.remember(self.problem, meta.learner.propose(self.problem.goal_effect, self.source, context=self.problem.context), evidence_digest='fresh-proof', verified=True)
        self.assertEqual(proposal.action, 'gesture')
        self.assertIsNotNone(record)
        self.assertEqual(len(meta.history), 1)
        self.assertEqual(meta.history[0].verification, 'verified')

    def test_reference_never_executes_or_replaces_fresh_evidence(self):
        meta = MetaSymbioticLearner()
        raw = meta.learner.propose(self.problem.goal_effect, self.source, context=self.problem.context)
        meta.remember(self.problem, raw, evidence_digest='proof', verified=True)
        empty = meta.adapt(self.problem, [], cost_budget=4)
        self.assertEqual((empty.status, empty.reason), ('ABSTAIN', 'reference_requires_fresh_evidence'))
        self.assertFalse(hasattr(meta, 'execute'))

    def test_reference_can_guide_different_host_without_copying_action(self):
        meta = MetaSymbioticLearner()
        raw = meta.learner.propose(self.problem.goal_effect, self.source, context=self.problem.context)
        meta.remember(self.problem, raw, evidence_digest='proof', verified=True)
        other_problem = Problem.from_maps({'mode': 1}, context={'channel': 1}, host_kind='host-b')
        candidates = [Episode.from_maps({'mode': 0}, 'button_7', {'mode': 1}, context={'channel': 1})]
        adapted = meta.adapt(other_problem, candidates)
        self.assertEqual((adapted.status, adapted.action), ('PROPOSE', 'button_7'))
        self.assertEqual(adapted.reference_id, meta.history[0].solution_id)
        self.assertEqual(adapted.reason, 'fresh_evidence_matches_reference')

    def test_history_ranks_search_without_removing_candidates(self):
        meta = MetaSymbioticLearner()
        raw = meta.learner.propose(self.problem.goal_effect, self.source, context=self.problem.context)
        meta.remember(self.problem, raw, evidence_digest='proof', verified=True)
        decoy = Episode.from_maps({'mode': 0}, 'decoy', {'mode': 0}, context={'channel': 9}, cost=2)
        ranked = meta.rank_candidates(self.problem, [decoy, self.source[0]])
        self.assertEqual(len(ranked), 2)
        self.assertEqual(ranked[0].action, 'gesture')

    def test_unverified_solution_is_not_saved(self):
        meta = MetaSymbioticLearner()
        raw = meta.learner.propose(self.problem.goal_effect, self.source, context=self.problem.context)
        self.assertIsNone(meta.remember(self.problem, raw, evidence_digest='proof', verified=False))
        self.assertEqual(meta.history, ())

    def test_history_is_exportable_without_credentials_or_execution(self):
        meta = MetaSymbioticLearner()
        data = meta.export()
        self.assertEqual(data['algorithm'], 'meta-symbiotic-learning-v1')
        self.assertNotIn('execute', data)
        self.assertNotIn('credential', repr(data).lower())

if __name__ == '__main__': unittest.main()
