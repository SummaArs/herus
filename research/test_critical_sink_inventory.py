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

    def test_unprofiled_critical_annotation_is_detected(self) -> None:
        with self.root.joinpath("firmware/core/interaction.c").open(encoding="utf-8") as handle:
            original = handle.read()
        mutated_root = self.root / "research" / ".tmp_inventory_mutant"
        source = mutated_root / "firmware/core/interaction.c"
        source.parent.mkdir(parents=True, exist_ok=True)
        registry = mutated_root / "research/critical_effect_registry.json"
        registry.parent.mkdir(parents=True, exist_ok=True)
        registry.write_text((self.root / "research/critical_effect_registry.json").read_text(encoding="utf-8"), encoding="utf-8")
        source.write_text(original.replace(
            "/* HERUS_CRITICAL_SINK: interaction-send class=external-transmission operation=interaction_take_send( */",
            "/* HERUS_CRITICAL_SINK: unprofiled-new-sink class=external-transmission operation=interaction_take_send( */\n/* HERUS_CRITICAL_SINK: interaction-send class=external-transmission operation=interaction_take_send( */",
            1,
        ), encoding="utf-8")
        try:
            results = inventory(self.profile, mutated_root)
            self.assertTrue(any(item.status == "UNPROFILED" and item.detail == "critical_annotation_profile_registry_mismatch" for item in results), results)
        finally:
            import shutil
            shutil.rmtree(mutated_root)

    def test_omitted_sink_is_unprofiled(self) -> None:
        profile = copy.deepcopy(self.profile)
        del profile["critical_sinks"]["nucleus-seal"]
        results = inventory(profile, self.root)
        omitted = [item for item in results if item.operation == "core_link_seal_nucleus_intent("]
        self.assertTrue(omitted)
        self.assertTrue(all(item.status == "UNPROFILED" for item in omitted), omitted)


if __name__ == "__main__":
    unittest.main()
