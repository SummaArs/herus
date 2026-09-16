from __future__ import annotations
import unittest
from unittest.mock import patch

from internet_fetcher import FetchPolicy, fetch_many, fetch_public


class FakeResponse:
    def __init__(self, payload=b"official facts", content_type="text/html"):
        self.payload = payload
        self.headers = {"Content-Type": content_type}
    def __enter__(self): return self
    def __exit__(self, *args): return False
    def read(self, size=-1): return self.payload[:size]


class InternetFetcherTests(unittest.TestCase):
    def setUp(self):
        self.policy = FetchPolicy(frozenset({"official.example"}), max_bytes_per_url=32)

    def test_fetches_only_allowlisted_https_and_returns_non_executable_record(self):
        with patch("internet_fetcher.urlopen", return_value=FakeResponse()) as mocked:
            record = fetch_public("https://official.example/spec", self.policy)
        self.assertEqual(record.content, "official facts")
        self.assertEqual(record.claims, ("external_source_retrieved",))
        mocked.assert_called_once()

    def test_rejects_non_allowlisted_url_before_network(self):
        with patch("internet_fetcher.urlopen") as mocked:
            with self.assertRaisesRegex(ValueError, "url_not_allowlisted"):
                fetch_public("https://evil.example/payload", self.policy)
        mocked.assert_not_called()

    def test_rejects_non_text_content(self):
        with patch("internet_fetcher.urlopen", return_value=FakeResponse(content_type="application/zip")):
            with self.assertRaisesRegex(ValueError, "content_type_not_allowed"):
                fetch_public("https://official.example/archive", self.policy)

    def test_enforces_url_budget(self):
        with self.assertRaisesRegex(ValueError, "url_budget_exhausted"):
            fetch_many(tuple("https://official.example/" + str(i) for i in range(5)), self.policy)


if __name__ == "__main__":
    unittest.main()
