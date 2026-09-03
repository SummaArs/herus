from __future__ import annotations

import copy
from pathlib import Path
import unittest

from critical_sink_inventory import inventory, load_profile


class CriticalSinkInventoryTests(unittest.TestCase):
    def setUp(self) -> None:
        self.root = Path(__file__).resolve().parents[1]
        self.profile = load_profile(self.root / "research" / "hcae_profile.json")

    def test_real_inventory_is_profiled(self) -> None:
        results = inventory(self.profile, self.root)
        self.assertTrue(results, "known operation inventory must not be empty")
        self.assertTrue(all(item.status == "PROFILED" for item in results), results)

    def test_omitted_sink_is_unprofiled(self) -> None:
        profile = copy.deepcopy(self.profile)
        del profile["critical_sinks"]["nucleus-seal"]
        results = inventory(profile, self.root)
        omitted = [item for item in results if item.operation == "core_link_seal_nucleus_intent("]
        self.assertTrue(omitted)
        self.assertTrue(all(item.status == "UNPROFILED" for item in omitted), omitted)


if __name__ == "__main__":
    unittest.main()
