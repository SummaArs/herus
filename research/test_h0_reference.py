from __future__ import annotations
import json
from pathlib import Path
import unittest

from h0_reference import H0Input, evaluate, evaluate_json


class H0ReferenceTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.vectors = json.loads(Path(__file__).with_name("evidence").joinpath("h0_golden_vectors.json").read_text())

    def test_golden_vectors(self):
        for vector in self.vectors:
            case = H0Input(
                case_id=vector["case_id"], features=tuple(vector["features"]), source=vector["source"],
                digest=vector["digest"], representations=tuple(vector["representations"]),
                budget_bytes=vector["budget_bytes"], budget_steps=vector["budget_steps"],
                authority=vector["authority"],
            )
            result = evaluate(case).canonical()
            for key, expected in vector["expected"].items():
                self.assertEqual(result[key], expected, vector["case_id"])

    def test_h0_json_is_canonical_and_execution_is_always_abstain(self):
        vector = self.vectors[0].copy()
        vector.pop("expected")
        first = evaluate_json(json.dumps(vector))
        second = evaluate_json(json.dumps(dict(reversed(list(vector.items())))))
        self.assertEqual(first, second)
        self.assertEqual(json.loads(first)["execution"], "ABSTAIN")

    def test_unknown_case_is_rejected(self):
        with self.assertRaises(ValueError):
            evaluate(H0Input("", (4, 0, 0, 0), "s", "d", ("SIM-INT8",), 4096, 12))


if __name__ == "__main__":
    unittest.main()
