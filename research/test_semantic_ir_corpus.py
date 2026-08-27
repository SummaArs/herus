import json
import unittest
from pathlib import Path


CORPUS = Path(__file__).with_name("evidence") / "semantic_ir_real_corpus.json"
VALID_EVENTS = {"ARRIVE", "HELP", "CANCEL"}
VALID_STATUSES = {"DRAFT", "CANCEL_LOCAL", "UNKNOWN", "REJECTED"}


class SemanticIRCorpusTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        with CORPUS.open(encoding="utf-8") as stream:
            cls.data = json.load(stream)

    def test_corpus_has_provenance_and_is_not_claimed_as_telemetry(self):
        self.assertEqual(self.data["corpusVersion"], 1)
        self.assertEqual(self.data["provenance"]["sourceFile"], "firmware/core/test_voice.c")
        self.assertTrue(self.data["provenance"]["notProductionTelemetry"])

    def test_every_case_has_closed_oracle(self):
        self.assertEqual(len(self.data["cases"]), 13)
        ids = set()
        for case in self.data["cases"]:
            self.assertNotIn(case["id"], ids)
            ids.add(case["id"])
            self.assertIn(case["mode"], {"TEXT", "TYPED_COMMAND"})
            expected = case["expected"]
            self.assertIn(expected["status"], VALID_STATUSES)
            event = expected["eventKind"]
            self.assertTrue(event is None or event in VALID_EVENTS)
            if event == "ARRIVE":
                self.assertTrue(expected["minutes"] is None or 1 <= expected["minutes"] <= 60)
            else:
                self.assertIsNone(expected["minutes"])
            if expected["status"] == "DRAFT":
                self.assertTrue(expected["requiresConfirmation"])
            else:
                self.assertFalse(expected["requiresConfirmation"])

    def test_text_fixtures_are_present_in_declared_c_source(self):
        source = Path(__file__).parents[1] / "firmware" / "core" / "test_voice.c"
        text = source.read_text(encoding="utf-8")
        for case in self.data["cases"]:
            if case["mode"] == "TEXT":
                self.assertIn(case["input"], text, case["id"])

    def test_typed_command_cases_have_explicit_command_and_duration(self):
        typed = [case for case in self.data["cases"] if case["mode"] == "TYPED_COMMAND"]
        self.assertEqual(len(typed), 3)
        for case in typed:
            self.assertIn(case["input"]["command"], VALID_EVENTS)
            minutes = case["input"]["minutes"]
            self.assertIsInstance(minutes, int)
            self.assertTrue(0 <= minutes <= 60)
            if case["input"]["command"] != "ARRIVE":
                self.assertNotEqual(minutes, 0)


if __name__ == "__main__":
    unittest.main()
