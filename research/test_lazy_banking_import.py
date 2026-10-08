import subprocess
import sys
import unittest


class LazyBankingImportTests(unittest.TestCase):
    def test_banking_path_does_not_import_heavy_minds14_stack(self):
        code = "import sys; import policy_selection_banking77; print(any(x in sys.modules for x in ('torch','transformers','numpy'))); print('independent_minds14_benchmark' in sys.modules)"
        out = subprocess.check_output([sys.executable, '-c', code], text=True, env={'PYTHONPATH':'research'}).splitlines()
        self.assertEqual(out, ['False', 'False'])


if __name__ == '__main__': unittest.main()
