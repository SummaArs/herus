import unittest
from herus_symbiotic import Herus

class HerusFacadeTests(unittest.TestCase):
    def test_one_import_exposes_all_safe_operations(self):
        h=Herus()
        self.assertIn('Herus().data',h.help())
        plan=h.data(({'text':'a','label':'x'},{'text':'b','label':'y'}),objective='intent')
        self.assertEqual(plan.status,'PROPOSE')
        code=h.program('write a parser')
        self.assertIn(code.status,{'PROPOSE','PROPOSE_WITH_QUESTIONS'})
        self.assertEqual(h.inspect()['authority'],'none')
    def test_observe_then_propose_without_execution(self):
        h=Herus()
        self.assertTrue(h.observe({'ready':0},'enable',{'ready':1}))
        proposal=h.propose({'ready':1})
        self.assertEqual(proposal.status,'PROPOSE')
        self.assertEqual(proposal.action,'enable')
        self.assertEqual(h.inspect()['authority'],'none')

if __name__=='__main__': unittest.main()
