import unittest
from symbiotic_policy import Candidate, HostBudget, SymbioticDecisionPolicy, VerifiedFeedback


class SymbioticPolicyTests(unittest.TestCase):
    def setUp(self):
        self.host = HostBudget('text-host', max_cost=1, max_risk=0, min_evidence=1)
        self.calibration = [
            Candidate(f'e{i}', 'safe', score, correct, evidence_count=1, host_id='text-host')
            for i, (score, correct) in enumerate(((0.99, True), (0.95, True), (0.80, False), (0.60, True)))
        ]

    def test_fit_is_bounded_and_selects_safe_point(self):
        policy = SymbioticDecisionPolicy(min_precision=1.0)
        fit = policy.fit(self.calibration, self.host)
        self.assertEqual(fit.status, 'FITTED')
        self.assertEqual(fit.precision, 1.0)
        self.assertEqual(policy.decide(self.calibration[0], self.host).status, 'ACCEPT')
        self.assertEqual(policy.decide(self.calibration[2], self.host).status, 'ABSTAIN')

    def test_no_calibration_fails_closed(self):
        policy = SymbioticDecisionPolicy()
        self.assertEqual(policy.fit([], self.host).reason, 'no_calibration_data')
        self.assertEqual(policy.decide(self.calibration[0], self.host).reason, 'policy_not_fitted')

    def test_host_and_evidence_constraints_abstain(self):
        policy = SymbioticDecisionPolicy(min_precision=1.0)
        policy.fit(self.calibration, self.host)
        self.assertEqual(policy.decide(Candidate('x', 'safe', .99, True, evidence_count=0, host_id='text-host'), self.host).reason, 'evidence_deficit')
        self.assertEqual(policy.decide(Candidate('x', 'safe', .99, True, evidence_count=1, host_id='other'), self.host).reason, 'host_mismatch')

    def test_holdout_label_is_not_needed_for_decision(self):
        policy = SymbioticDecisionPolicy(min_precision=1.0)
        policy.fit(self.calibration, self.host)
        candidate = Candidate('holdout', 'candidate-label', .99, None, evidence_count=1, host_id='text-host')
        self.assertEqual(policy.decide(candidate, self.host).status, 'ACCEPT')

    def test_feedback_requires_provenance_and_deduplicates(self):
        policy = SymbioticDecisionPolicy()
        good = VerifiedFeedback('e1', True, True, 'text-host', 'verified:test')
        self.assertTrue(policy.update(good))
        self.assertFalse(policy.update(good))
        self.assertFalse(policy.update(VerifiedFeedback('', True, True, 'text-host', 'verified:test')))
        self.assertEqual(policy.inspect()['feedback_count'], 1)


if __name__ == '__main__':
    unittest.main()
