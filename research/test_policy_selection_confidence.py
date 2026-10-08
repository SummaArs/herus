import unittest
from policy_selection_confidence import paired_bootstrap_delta, choose


class PolicySelectionConfidenceTests(unittest.TestCase):
    def test_invalid_lengths_fail_closed(self):
        with self.assertRaises(ValueError):
            paired_bootstrap_delta(['a'],['a'],[])

    def test_clear_paired_gain_selects_alternative(self):
        labels=['x']*20
        default=['y']*20
        alternative=['x']*20
        selected,ci=choose(default,alternative,labels,rounds=500)
        self.assertEqual(selected,'alternative')
        self.assertGreater(ci['lower_95'],0)

    def test_uncertain_gain_abstains_to_default(self):
        labels=['x']*20
        default=['x']*19+['y']
        alternative=['x']*20
        selected,ci=choose(default,alternative,labels,rounds=500)
        self.assertEqual(selected,'default')
        self.assertLessEqual(ci['lower_95'],0)


if __name__=='__main__': unittest.main()
