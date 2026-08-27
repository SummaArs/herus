import json
import re
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parent.parent
FIRMWARE = ROOT / "firmware"
RESEARCH = ROOT / "research"
DOCS = ROOT / "docs"


class EvolutionCompatibilityTests(unittest.TestCase):
    def test_semantic_ir_is_closed_and_proposal_only(self):
        schema = json.loads((RESEARCH / "semantic_ir.schema.json").read_text(encoding="utf-8"))
        self.assertEqual(schema["properties"]["schemaVersion"]["const"], 1)
        self.assertFalse(schema["additionalProperties"])
        self.assertEqual(schema["properties"]["authority"]["const"], "PROPOSAL_ONLY")
        self.assertEqual(schema["properties"]["eventKind"]["enum"], ["ARRIVE", "HELP", "CANCEL"])

    def test_core_link_has_fixed_wire_and_rejects_unknown_version(self):
        header = (FIRMWARE / "net" / "core_link.h").read_text(encoding="utf-8")
        source = (FIRMWARE / "net" / "core_link.c").read_text(encoding="utf-8")
        self.assertIn("#define CORE_LINK_VERSION       1u", header)
        self.assertIn("#define CORE_LINK_WIRE_LEN (CORE_LINK_HEADER_LEN + CORE_LINK_PAYLOAD_LEN + CORE_LINK_TAG_LEN)", header)
        self.assertRegex(source, r"wire_len != CORE_LINK_WIRE_LEN")
        self.assertRegex(source, r"wire\[OFF_VER\] != CORE_LINK_VERSION")

    def test_hcp_keeps_forward_compatibility_rule_explicit(self):
        protocol = (DOCS / "02-PROTOCOL.md").read_text(encoding="utf-8")
        hcp = (FIRMWARE / "core" / "hcp.h").read_text(encoding="utf-8")
        self.assertIn("Unknown role ids are **skipped, never rejected**", protocol)
        self.assertIn("ignores the rest", hcp)
        self.assertIn("out->pos[] carries each kept slot's ORIGINAL", hcp)

    def test_memory_is_versioned_and_rollback_is_blocking(self):
        header = (FIRMWARE / "core" / "memory_collection.h").read_text(encoding="utf-8")
        source = (FIRMWARE / "core" / "memory_collection.c").read_text(encoding="utf-8")
        self.assertIn("#define MEMORY_COLLECTION_VERSION 1u", header)
        self.assertIn("#define MEMORY_COLLECTION_MAX_CARDS 8u", header)
        self.assertIn("MEMORY_COLLECTION_E_ROLLBACK", header)
        self.assertRegex(source, r"state = MEMORY_COLLECTION_BLOCKED")
        self.assertIn("committed.generation != floor", source)

    def test_context_hint_has_bounded_confidence_contract(self):
        header = (FIRMWARE / "core" / "intent_gate.h").read_text(encoding="utf-8")
        source = (FIRMWARE / "core" / "intent_gate.c").read_text(encoding="utf-8")
        self.assertIn("confidence_pct;   /* support / all retained context observations, 0..100 */", header)
        self.assertIn("hint->confidence_pct <= 100u", source)

    def test_interaction_preserves_physical_confirmation_boundary(self):
        header = (FIRMWARE / "core" / "interaction.h").read_text(encoding="utf-8")
        source = (FIRMWARE / "core" / "interaction.c").read_text(encoding="utf-8")
        self.assertIn("Physical button press is the sole transition that starts capture", header)
        self.assertIn("The physical confirmation handler may release a pending draft exactly once", header)
        self.assertIn("stops at AWAIT_CONFIRM", source)
        self.assertIn("interaction_take_send_assured", header)


if __name__ == "__main__":
    unittest.main()
