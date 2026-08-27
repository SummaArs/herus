import sys
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from fetch_minds14_batch import MAX_BATCH, audit, sample_url


class Minds14BatchTests(unittest.TestCase):
    def test_sample_url_uses_rows_and_explicit_offset(self):
        url = sample_url(100)
        self.assertIn("datasets-server.huggingface.co/rows?", url)
        self.assertIn("offset=100", url)
        self.assertIn("length=1", url)

    def test_invalid_offsets_are_rejected_before_network(self):
        missing_runner = Path("/definitely/missing/c-runner")
        for offsets, reason in (
            ([], "invalid_offsets"),
            ([-1], "invalid_offsets"),
            (list(range(MAX_BATCH + 1)), "invalid_offsets"),
            ([1, 1], "duplicate_offsets"),
        ):
            with self.subTest(offsets=offsets), self.assertRaisesRegex(ValueError, reason):
                audit(offsets, missing_runner)


if __name__ == "__main__":
    unittest.main()
