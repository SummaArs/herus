from __future__ import annotations
import json
from pathlib import Path
import unittest


class RealHostRegistryTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        path = Path(__file__).parent / "evidence" / "real_host_training_registry.json"
        cls.data = json.loads(path.read_text())

    def test_registry_has_physical_and_nonphysical_paths(self):
        hosts = self.data["hosts"]
        realities = {host["reality"] for host in hosts}
        self.assertIn("real", realities)
        self.assertIn("real_physical_hardware", realities)
        self.assertIn("real_external_information", realities)

    def test_every_host_has_safety_boundary_and_next_gate(self):
        for host in self.data["hosts"]:
            self.assertTrue(host["id"])
            self.assertTrue(host["access"])
            self.assertTrue(host["useful_for"])
            self.assertTrue(host["effects"])
            self.assertTrue(host["next_gate"])

    def test_registry_does_not_claim_physical_access_now(self):
        by_id = {host["id"]: host for host in self.data["hosts"]}
        self.assertEqual(by_id["esp32-s3-h1"]["access"], "pending_purchase_or_arrival")
        self.assertEqual(by_id["wrist-h2"]["access"], "blocked_by_h1")


if __name__ == "__main__":
    unittest.main()
