import json
import unittest
from pathlib import Path
from gate0_contract import run, validate


class Gate0ContractTests(unittest.TestCase):
    def test_protocol_is_frozen_but_execution_remains_blocked(self):
        result = run()
        self.assertEqual(result["decision"], "PASS_FOR_PROTOCOL_ONLY")
        self.assertFalse(result["execution_authorized"])
        self.assertIn("herus_implementation_id_not_frozen", result["errors"])

    def test_frozen_implementation_removes_only_that_blocker(self):
        path = Path(__file__).parent / "research" / "evidence" / "gate0_protocol_v1.json"
        protocol = json.loads(path.read_text(encoding="utf-8"))
        protocol["herus_object"]["implementation_id"] = "herus_symbiotic_v2_text_adapter@candidate"
        errors = validate(protocol)
        self.assertNotIn("herus_implementation_id_not_frozen", errors)
        self.assertFalse(any("primary_endpoint" in error for error in errors))


if __name__ == "__main__":
    unittest.main()
