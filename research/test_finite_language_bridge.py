from __future__ import annotations

import unittest

from finite_language_bridge import translate


class FiniteLanguageBridgeTests(unittest.TestCase):
    def test_known_phrase_becomes_proposal_only(self) -> None:
        result = translate("Cheguei em segurança")
        self.assertEqual((result.status, result.intent), ("PROPOSAL_ONLY", "STATUS_ARRIVED"))

    def test_accents_are_normalized_without_changing_intent(self) -> None:
        result = translate("Preciso de ajuda")
        self.assertEqual(result.intent, "ALERT_HELP")
        self.assertEqual(result.status, "PROPOSAL_ONLY")

    def test_unknown_language_is_refused(self) -> None:
        result = translate("Faça algo inteligente")
        self.assertEqual((result.status, result.reason), ("BLOCKED", "unknown_language_form"))
        self.assertIsNone(result.intent)

    def test_empty_input_reports_missing_intent(self) -> None:
        result = translate("   ")
        self.assertEqual((result.status, result.missing), ("BLOCKED", ("intent",)))


if __name__ == "__main__":
    unittest.main()
