import unittest
from mintrec_gate1_identity import build, example_id


class MintrecGate1IdentityTests(unittest.TestCase):
    def test_identity_is_deterministic_and_content_bound(self):
        row = {"season": "S06", "episode": "e1", "clip": "1", "label": "Inform", "text": "hello"}
        self.assertEqual(example_id(row), example_id(dict(row)))
        changed = dict(row, text="hello changed")
        self.assertNotEqual(example_id(row), example_id(changed))

    def test_build_sorts_and_rejects_duplicate_identities(self):
        rows = [
            {"season": "S06", "episode": "e2", "clip": "2", "label": "B", "text": "two"},
            {"season": "S05", "episode": "e0", "clip": "0", "label": "A", "text": "fit"},
            {"season": "S06", "episode": "e1", "clip": "1", "label": "A", "text": "one"},
        ]
        result = build(rows)
        self.assertEqual(result["holdout_rows"], 2)
        self.assertEqual([r["example_id"] for r in result["records"]], sorted(r["example_id"] for r in result["records"]))
        with self.assertRaises(ValueError):
            build(rows + [rows[0]])

    def test_missing_identity_field_fails_closed(self):
        with self.assertRaises(ValueError):
            build([{"season": "S06", "episode": "e1", "label": "A", "text": "one"}])


if __name__ == "__main__":
    unittest.main()
