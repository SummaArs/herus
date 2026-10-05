import unittest
from external_host_protocol import ExternalHostClient
from meta_symbiotic_learning import MetaSymbioticLearner, Problem

class ExternalHostProtocolTests(unittest.TestCase):
    def test_external_process_transfer_and_rotation(self):
        client = ExternalHostClient()
        try:
            self.assertEqual(client.actions(), ('target_x', 'target_y'))
            first = tuple(client.probe(action) for action in client.actions())
            learner = MetaSymbioticLearner()
            problem = Problem.from_maps({'mode': 1}, context={'zone': 1}, host_kind='external')
            proposal = learner.adapt(problem, first, current_step=max(e.step for e in first))
            self.assertEqual((proposal.status, proposal.action), ('PROPOSE', 'target_x'))
            client.reset_and_rotate()
            self.assertEqual(client.actions(), ('target_level', 'target_new'))
            second = tuple(client.probe(action) for action in client.actions())
            rotated = learner.adapt(problem, second, current_step=max(e.step for e in second))
            self.assertEqual((rotated.status, rotated.action), ('PROPOSE', 'target_new'))
            self.assertTrue(rotated.fresh_evidence)
        finally:
            client.close()

    def test_corrupt_message_returns_error_and_server_survives(self):
        client = ExternalHostClient()
        try:
            self.assertIn('invalid_json', client.raw('{not-json'))
            self.assertEqual(client.actions(), ('target_x', 'target_y'))
        finally:
            client.close()

    def test_timeout_is_fail_closed(self):
        client = ExternalHostClient(timeout=0.03, startup_timeout=1.0)
        with self.assertRaises(TimeoutError):
            client.raw('{"op":"delay","seconds":0.2}')
        self.assertIsNotNone(client.proc.poll())

if __name__ == '__main__': unittest.main()
