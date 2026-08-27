import json
import sys
import tempfile
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from analyze_slurp_text import audit


class SlurpTextAuditTests(unittest.TestCase):
    def write_jsonl(self, root: Path, name: str, filename: str = "audio-a.flac") -> Path:
        row = {
            "slurp_id": 99,
            "sentence": "turn on the lights",
            "sentence_annotation": "turn on the lights",
            "intent": "iot_hue_lighton",
            "action": "activate",
            "tokens": [],
            "scenario": "iot",
            "recordings": [{"file": filename, "status": "correct"}],
            "entities": [],
        }
        path = root / name
        path.write_text(json.dumps(row) + "\n", encoding="utf-8")
        return path

    def write_metadata(self, root: Path, entries: dict[str, dict]) -> Path:
        path = root / "metadata.json"
        path.write_text(json.dumps(entries), encoding="utf-8")
        return path

    def test_join_uses_recording_filename_not_numeric_ids(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            paths = {
                split: self.write_jsonl(root, f"{split}.jsonl", f"audio-{split}.flac")
                for split in ("train", "devel", "test")
            }
            metadata = self.write_metadata(
                root,
                {
                    "0": {"nlub_id": 0, "recordings": {"audio-train.flac": {"status": "correct"}}},
                    "1": {"nlub_id": 1, "recordings": {"audio-devel.flac": {"status": "correct"}}},
                    "2": {"nlub_id": 2, "recordings": {"audio-test.flac": {"status": "correct"}}},
                },
            )
            result = audit(paths, metadata)
            self.assertEqual(result["metadata_joined_rows_by_recording_filename"], 3)
            self.assertEqual(result["missing_recording_references"], 0)
            self.assertTrue(result["verdict"]["text_recording_metadata_alignment_verified"])

    def test_duplicate_metadata_filename_blocks_unambiguous_alignment(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            paths = {
                split: self.write_jsonl(root, f"{split}.jsonl")
                for split in ("train", "devel", "test")
            }
            metadata = self.write_metadata(
                root,
                {
                    "0": {"nlub_id": 0, "recordings": {"audio-a.flac": {"status": "correct"}}},
                    "1": {"nlub_id": 1, "recordings": {"audio-a.flac": {"status": "correct"}}},
                },
            )
            result = audit(paths, metadata)
            self.assertEqual(result["metadata_duplicate_recording_names"], 1)
            self.assertFalse(result["verdict"]["text_recording_metadata_alignment_verified"])

    def test_missing_recording_is_not_promoted(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            paths = {
                split: self.write_jsonl(root, f"{split}.jsonl", f"{split}.flac")
                for split in ("train", "devel", "test")
            }
            metadata = self.write_metadata(root, {"0": {"recordings": {"other.flac": {}}}})
            result = audit(paths, metadata)
            self.assertEqual(result["missing_recording_references"], 3)
            self.assertFalse(result["verdict"]["text_schema_integrity"])


if __name__ == "__main__":
    unittest.main()
