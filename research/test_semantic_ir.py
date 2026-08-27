import unittest

from semantic_ir import (
    EventKind,
    HypothesisStatus,
    compile_ir,
    meaning_key,
    to_firmware_command,
    validate_ir,
)


def valid_ir(**overrides):
    value = {
        "schemaVersion": 1,
        "eventKind": "ARRIVE",
        "source": "TEXT",
        "confidencePct": 96,
        "runnerUpPct": 10,
        "slots": {"minutes": 15},
        "evidence": [
            {"kind": "OBSERVATION", "ref": "pt.command", "polarity": "POSITIVE", "weight": 96}
        ],
        "hypothesisStatus": "TRUE",
        "authority": "PROPOSAL_ONLY",
    }
    value.update(overrides)
    return value


class SemanticIRTests(unittest.TestCase):
    def test_valid_ir_compiles_to_proposal_only(self):
        proposal, issues = compile_ir(valid_ir())
        self.assertEqual(issues, ())
        self.assertIsNotNone(proposal)
        self.assertTrue(proposal.proposal_only)
        self.assertEqual(to_firmware_command(proposal), ("VOICE_COMMAND_ARRIVE", 15))

    def test_equivalent_modalities_have_same_meaning_key(self):
        text, text_issues = compile_ir(valid_ir(source="TEXT"))
        voice, voice_issues = compile_ir(valid_ir(source="VOICE"))
        button, button_issues = compile_ir(valid_ir(source="BUTTON"))
        self.assertEqual(text_issues, ())
        self.assertEqual(voice_issues, ())
        self.assertEqual(button_issues, ())
        self.assertEqual(meaning_key(text), meaning_key(voice))
        self.assertEqual(meaning_key(voice), meaning_key(button))

    def test_unknown_keys_are_rejected_without_coercion(self):
        value = valid_ir()
        value["freeText"] = "send this"
        issues = validate_ir(value)
        self.assertTrue(any(issue.code == "UNKNOWN_KEY" for issue in issues))
        proposal, _ = compile_ir(value)
        self.assertIsNone(proposal)

    def test_invalid_schema_version_and_authority_are_rejected(self):
        value = valid_ir(schemaVersion="1", authority="EXECUTE")
        issues = validate_ir(value)
        self.assertTrue(any(issue.path == "$.schemaVersion" for issue in issues))
        self.assertTrue(any(issue.path == "$.authority" for issue in issues))

    def test_minutes_cannot_leak_into_other_event_kinds(self):
        value = valid_ir(eventKind="HELP", slots={"minutes": 15})
        issues = validate_ir(value)
        self.assertTrue(any(issue.code == "SEMANTIC" for issue in issues))
        self.assertIsNone(compile_ir(value)[0])

    def test_contradiction_blocks_firmware_command(self):
        proposal, issues = compile_ir(valid_ir(hypothesisStatus="BOTH"))
        self.assertEqual(issues, ())
        self.assertEqual(proposal.hypothesis_status, HypothesisStatus.BOTH)
        self.assertIsNone(to_firmware_command(proposal))

    def test_low_confidence_and_small_margin_block(self):
        low, _ = compile_ir(valid_ir(confidencePct=79))
        narrow, _ = compile_ir(valid_ir(confidencePct=90, runnerUpPct=80))
        self.assertIsNone(to_firmware_command(low))
        self.assertIsNone(to_firmware_command(narrow))

    def test_unknown_or_invalid_evidence_is_rejected(self):
        value = valid_ir(evidence=[{"kind": "HUNCH", "ref": "bad ref", "polarity": "MAYBE", "weight": 101}])
        issues = validate_ir(value)
        self.assertGreaterEqual(len(issues), 3)
        self.assertIsNone(compile_ir(value)[0])

    def test_missing_optional_slot_is_not_silently_defaulted(self):
        value = valid_ir(slots={})
        issues = validate_ir(value)
        self.assertTrue(any(issue.path == "$.slots.minutes" for issue in issues))

    def test_cancel_and_help_never_carry_minutes(self):
        expected = {
            EventKind.CANCEL.value: ("VOICE_COMMAND_CANCEL", 0),
            EventKind.HELP.value: ("VOICE_COMMAND_HELP", 0),
        }
        for event, command in expected.items():
            value = valid_ir(eventKind=event, slots={"minutes": None})
            proposal, issues = compile_ir(value)
            self.assertEqual(issues, ())
            self.assertEqual(proposal.minutes, None)
            self.assertEqual(to_firmware_command(proposal), command)


if __name__ == "__main__":
    unittest.main()
