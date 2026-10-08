import unittest
from invariance_gate import DecisionInvarianceGate


class InvarianceGateTests(unittest.TestCase):
    def test_unfitted_fails_closed(self):
        with self.assertRaises(RuntimeError):
            DecisionInvarianceGate(lambda x: x).accept('x')

    def test_disagreement_is_rejected(self):
        gate=DecisionInvarianceGate(lambda x: 'changed' if 'hello' in x else 'same')
        gate.fit(['pay bill','check account'])
        self.assertFalse(gate.accept('hello: pay bill'))

    def test_empty_calibration_is_rejected(self):
        with self.assertRaises(ValueError):
            DecisionInvarianceGate(lambda x: x).fit([])


if __name__=='__main__': unittest.main()
