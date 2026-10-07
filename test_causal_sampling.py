import json
import unittest
from pathlib import Path
from causal_sampling import run, validate


class CausalSamplingTests(unittest.TestCase):
    def test_design_passes_but_no_execution_result_exists(self):
        result = run()
        self.assertEqual(result["decision"], "PASS_FOR_DESIGN_ONLY")
        self.assertFalse(result["execution_result_present"])
        self.assertGreater(result["identity_summary"]["episode_clusters"], 0)

    def test_row_bootstrap_is_rejected(self):
        protocol_path = Path("research/evidence/causal_sampling_protocol_v1.json")
        protocol = json.loads(protocol_path.read_text(encoding="utf-8"))
        protocol["sampling"]["resample_rows_individually"] = True
        self.assertIn("row_level_resampling_forbidden", validate(protocol))


if __name__ == "__main__":
    unittest.main()
