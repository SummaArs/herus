import sys
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from identity_audit import audit_identity


def record(slurp_id=1, filename="a.wav"):
    return (
        "train",
        {
            "slurp_id": slurp_id,
            "sentence": "opaque sentence kept only in fixture",
            "intent": "external_intent",
            "scenario": "external_scenario",
            "recordings": [{"file": filename}],
        },
    )


def metadata(*filenames):
    return {
        str(index): {"recordings": {filename: {"sample_rate": 8000}}}
        for index, filename in enumerate(filenames)
    }


class IdentityAuditTests(unittest.TestCase):
    def test_unique_filename_join_is_verified(self):
        result = audit_identity([record()], metadata("a.wav"), {"train": {"rows": 1}}, "metadata-hash")
        self.assertEqual(result["identity_status"], "VERIFIED")
        self.assertTrue(result["identity_gate_passed"])
        self.assertEqual(result["metadata_duplicate_filename_count"], 0)
        self.assertEqual(result["herus_automatic_mapping_count"], 0)

    def test_duplicate_metadata_filename_is_ambiguous(self):
        payload = {
            "one": {"recordings": {"a.wav": {"sample_rate": 8000}}},
            "two": {"recordings": {"a.wav": {"sample_rate": 8000}}},
        }
        result = audit_identity([record()], payload)
        self.assertEqual(result["identity_status"], "AMBIGUOUS")
        self.assertFalse(result["identity_gate_passed"])
        self.assertEqual(result["metadata_duplicate_filename_conflict_count"], 0)

    def test_conflicting_metadata_filename_is_conflict(self):
        payload = {
            "one": {"recordings": {"a.wav": {"sample_rate": 8000}}},
            "two": {"recordings": {"a.wav": {"sample_rate": 16000}}},
        }
        result = audit_identity([record()], payload)
        self.assertEqual(result["identity_status"], "CONFLICT")
        self.assertFalse(result["identity_gate_passed"])
        self.assertEqual(result["metadata_duplicate_filename_conflict_count"], 1)

    def test_repeated_text_reference_is_ambiguous(self):
        result = audit_identity([record(1), record(2)], metadata("a.wav"))
        self.assertEqual(result["identity_status"], "AMBIGUOUS")
        self.assertEqual(result["text_repeated_reference_count"], 1)

    def test_missing_reference_is_conflict(self):
        result = audit_identity([record(filename="missing.wav")], metadata("a.wav"))
        self.assertEqual(result["identity_status"], "CONFLICT")
        self.assertEqual(result["missing_reference_count"], 1)

    def test_numeric_ids_are_not_a_join_shortcut(self):
        result = audit_identity([record(1), record(1, "b.wav")], metadata("a.wav", "b.wav"))
        self.assertEqual(result["duplicate_numeric_id_count"], 1)
        self.assertFalse(result["numeric_id_join_used"])
        self.assertFalse(result["label_join_used"])
        self.assertFalse(result["order_join_used"])
        self.assertFalse(result["basename_join_used"])
        self.assertFalse(result["identity_gate_passed"])

    def test_empty_inputs_fail_closed(self):
        with self.assertRaisesRegex(ValueError, "empty_text_records"):
            audit_identity([], metadata("a.wav"))
        with self.assertRaisesRegex(ValueError, "empty_or_invalid_metadata"):
            audit_identity([record()], {})


if __name__ == "__main__":
    unittest.main()
