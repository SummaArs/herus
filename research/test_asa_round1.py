from __future__ import annotations

import unittest

from research.asa_round1 import _evaluate, _parse_event, run


class AsaRound1Tests(unittest.TestCase):
    def test_parser_uses_input_contract_and_abstains_on_conflict(self) -> None:
        self.assertEqual(_parse_event({"input": "Chego em dez minutos", "expected": {}}), "ARRIVE")
        self.assertIsNone(_parse_event({"input": "cancelar e chego em dez minutos", "expected": {}}))
        self.assertIsNone(_parse_event({"input": "não cancelar", "expected": {}}))

    def test_round1_effect_transfer_beats_name_baseline(self) -> None:
        result = run()
        self.assertEqual(result["metrics"]["asa"]["semantic_match"], 13)
        self.assertEqual(result["metrics"]["asa"]["unsafe_non_abstention"], 0)
        self.assertEqual(result["metrics"]["name_baseline"]["semantic_match"], 8)
        self.assertEqual(result["metrics"]["name_baseline"]["unsafe_non_abstention"], 0)

    def test_harness_declares_proposal_only_boundary(self) -> None:
        self.assertEqual(run()["authority_boundary"], "proposal-only; no external execution")


if __name__ == "__main__":
    unittest.main()
