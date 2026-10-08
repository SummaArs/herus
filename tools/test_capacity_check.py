import unittest
from capacity_check import assess, closed_form_kmax, miss_rate, p_member


class CapacityCheckTests(unittest.TestCase):
    def test_majority_requires_positive_odd_k(self):
        self.assertGreater(p_member(1),.5)
        with self.assertRaises(ValueError): p_member(2)

    def test_closed_form_bound_is_below_claimed_even_value(self):
        self.assertLess(closed_form_kmax(),308)
        self.assertFalse(assess()['claim_valid_odd'])
        self.assertFalse(assess()['safe_for_all_budgets'])

    def test_valid_odd_boundary_is_explicit(self):
        self.assertLess(miss_rate(305),.5)
        self.assertGreater(miss_rate(307),.5)


if __name__=='__main__': unittest.main()
