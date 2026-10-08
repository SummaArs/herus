import unittest
from risk_coverage_policy import CalibratedRiskCoverage

class RiskCoveragePolicyTests(unittest.TestCase):
    def test_selects_widest_safe_region(self):
        p=CalibratedRiskCoverage(.95)
        d=p.fit([3,3,2,1],[True,True,False,False])
        self.assertEqual(d.agreement_threshold, 1)
        self.assertLessEqual(d.risk_upper, .95)

    def test_fails_closed_when_target_is_impossible(self):
        with self.assertRaises(ValueError):
            CalibratedRiskCoverage(.1).fit([1,1,1],[False,False,False])

    def test_requires_fit_before_accept(self):
        with self.assertRaises(RuntimeError):
            CalibratedRiskCoverage().accept(['a','a','a'])

if __name__=='__main__': unittest.main()
