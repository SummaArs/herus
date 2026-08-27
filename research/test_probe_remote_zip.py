import io
import unittest
import sys
import urllib.request
import zipfile
from pathlib import Path
from unittest import mock

sys.path.insert(0, str(Path(__file__).resolve().parent))
from probe_remote_zip import ProbeError, _central_names, probe


class FakeResponse:
    def __init__(self, body: bytes, start: int, end: int, total: int, status: int = 206):
        self.body = body
        self.status = status
        self.headers = {
            "Content-Range": f"bytes {start}-{end}/{total}",
            "Content-Length": str(len(body)),
        }

    def __enter__(self):
        return self

    def __exit__(self, exc_type, exc, traceback):
        return False

    def read(self):
        return self.body


def make_zip() -> bytes:
    handle = io.BytesIO()
    with zipfile.ZipFile(handle, "w", compression=zipfile.ZIP_STORED) as archive:
        archive.writestr("dataset/train.csv", "speakerId,path,transcription,action,object,location\n")
    return handle.getvalue()


class RemoteZipProbeTests(unittest.TestCase):
    def test_full_tail_reports_members_without_extraction(self):
        body = make_zip()
        total = len(body)
        start = 0
        end = total - 1
        fake = FakeResponse(body, start, end, total)
        with mock.patch.object(urllib.request, "urlopen", return_value=fake):
            result = probe("https://example.invalid/file.zip", total, total)
        self.assertEqual(result["verdict"], "structure_verified")
        self.assertEqual(result["member_count"], 1)
        self.assertEqual(result["member_names"], ["dataset/train.csv"])
        self.assertFalse(result["raw_data_downloaded"])
        self.assertFalse(result["extracted"])
        self.assertFalse(result["executed"])

    def test_range_header_mismatch_is_refused(self):
        body = make_zip()
        total = len(body)
        fake = FakeResponse(body, 1, total, total)
        with mock.patch.object(urllib.request, "urlopen", return_value=fake):
            with self.assertRaisesRegex(ProbeError, "content_range_mismatch"):
                probe("https://example.invalid/file.zip", total, total)

    def test_central_directory_outside_tail_is_refused(self):
        body = make_zip()
        with self.assertRaisesRegex(ProbeError, "central_directory_not_fully_in_range"):
            _central_names(body[:8], 0, 100, 20)


if __name__ == "__main__":
    unittest.main()
