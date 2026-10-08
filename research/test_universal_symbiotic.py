import unittest
from universal_symbiotic import ParadigmCandidate, UniversalContract, UniversalSymbioticLearner


class UniversalSymbioticTests(unittest.TestCase):
    def setUp(self):
        self.contract=UniversalContract('host', max_risk=0, max_cost=1, min_evidence=1, min_precision=1.0)
        self.cal=[
            ParadigmCandidate('c1','supervised','A',.98,True,evidence=2,host_id='host'),
            ParadigmCandidate('c2','reinforcement','B',.91,True,evidence=1,host_id='host'),
            ParadigmCandidate('c3','unsupervised','C',.99,False,evidence=1,host_id='host',risk=1),
        ]

    def test_routes_to_best_contract_satisfying_paradigm(self):
        u=UniversalSymbioticLearner(); fit=u.fit(self.cal,self.contract)
        self.assertEqual(fit.status,'FITTED')
        d=u.decide([ParadigmCandidate('x','supervised','A',.99,None,evidence=2,host_id='host'), ParadigmCandidate('x','reinforcement','B',.92,None,evidence=1,host_id='host')],example_id='x')
        self.assertEqual((d.status,d.paradigm,d.label),('ACCEPT','supervised','A'))

    def test_abstains_on_cross_paradigm_conflict(self):
        u=UniversalSymbioticLearner(); u.fit(self.cal,self.contract)
        d=u.decide([ParadigmCandidate('x','supervised','A',.99,None,evidence=1,host_id='host'), ParadigmCandidate('x','reinforcement','B',.99,None,evidence=1,host_id='host')],example_id='x')
        self.assertEqual(d.reason,'cross_paradigm_conflict')

    def test_abstains_when_no_candidate_meets_contract(self):
        u=UniversalSymbioticLearner(); u.fit(self.cal,self.contract)
        d=u.decide([ParadigmCandidate('x','supervised','A',.5,None,evidence=1,host_id='host')],example_id='x')
        self.assertEqual(d.reason,'no_candidate_meets_contract')

    def test_holdout_correctness_is_not_required(self):
        u=UniversalSymbioticLearner(); u.fit(self.cal,self.contract)
        d=u.decide([ParadigmCandidate('x','supervised','A',.99,None,evidence=2,host_id='host')],example_id='x')
        self.assertEqual(d.status,'ACCEPT')

    def test_invalid_contract_fails_closed(self):
        u=UniversalSymbioticLearner(); self.assertEqual(u.fit(self.cal,UniversalContract('',max_risk=0)).status,'BLOCKED')

    def test_policy_promotion_is_blocked_without_cross_host_stability(self):
        u=UniversalSymbioticLearner()
        decision=u.promote_policy([
            {'dataset':'MIntRec','validation_accuracy':{'score_calibrated':.44,'universal_default':.33}},
            {'dataset':'MInDS-14','validation_accuracy':{'score_calibrated':.84,'universal_default':.84}},
            {'dataset':'Banking77','validation_accuracy':{'score_calibrated':.725,'universal_default':.721}},
        ])
        self.assertEqual(decision['status'],'ABSTAIN')
        self.assertEqual(decision['policy'],'universal_default')


if __name__=='__main__': unittest.main()
