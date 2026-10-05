import unittest
from blackbox_symbiosis import BlackBoxHost, run

class BlackBoxSymbiosisTests(unittest.TestCase):
    def test_blackbox_transfer_and_interface_rotation(self):
        result = run()
        self.assertEqual(result['source_action'], 'source_a')
        self.assertEqual(result['target_action'], 'target_x')
        self.assertEqual(result['rotated_action'], 'target_new')
        self.assertTrue(result['target_host_private_state_hidden'])
        self.assertTrue(result['target_action_space_changed'])
        self.assertTrue(result['fresh_evidence_required'])
        self.assertFalse(result['authority_granted'])

    def test_host_does_not_expose_private_state(self):
        host = BlackBoxHost(host_kind='x', action_map={'a': {'mode': 1}}, hidden={'secret': 42})
        self.assertFalse(hasattr(host, 'state'))
        self.assertNotIn('secret', dict(host.public_state()))
        self.assertNotIn('secret', host.action_names())

if __name__ == '__main__': unittest.main()
