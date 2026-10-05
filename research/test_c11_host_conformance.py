import shutil
import subprocess
import unittest
from pathlib import Path
from external_host_protocol import ExternalHostClient
from meta_symbiotic_learning import MetaSymbioticLearner, Problem

class C11HostConformanceTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        if not shutil.which('cc'):
            raise unittest.SkipTest('no C compiler')
        root = Path(__file__).parent
        cls.binary = root / 'independent_host_c11.bin'
        result = subprocess.run(['cc', '-std=c11', '-Wall', '-Wextra', '-Werror', str(root / 'independent_host_c11.c'), '-o', str(cls.binary)], capture_output=True, text=True)
        if result.returncode:
            raise AssertionError(result.stderr)

    @classmethod
    def tearDownClass(cls):
        cls.binary.unlink(missing_ok=True)

    def test_c11_host_conforms_and_rotates(self):
        client = ExternalHostClient(executable=str(self.binary))
        try:
            self.assertEqual(client.actions(), ('target_x', 'target_y'))
            first = tuple(client.probe(a) for a in client.actions())
            learner = MetaSymbioticLearner()
            problem = Problem.from_maps({'mode': 1}, context={'zone': 1}, host_kind='c11')
            proposal = learner.adapt(problem, first, current_step=max(e.step for e in first))
            self.assertEqual((proposal.status, proposal.action), ('PROPOSE', 'target_x'))
            client.reset_and_rotate()
            second = tuple(client.probe(a) for a in client.actions())
            rotated = learner.adapt(problem, second, current_step=max(e.step for e in second))
            self.assertEqual((rotated.status, rotated.action), ('PROPOSE', 'target_new'))
        finally:
            client.close()

if __name__ == '__main__': unittest.main()
