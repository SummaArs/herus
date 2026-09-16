from __future__ import annotations
import unittest

from internet_host import InternetHostPolicy, ingest, make_record
from knowledge_gateway import ResearchQuestion, EvidenceStatus


class InternetHostTests(unittest.TestCase):
    def setUp(self):
        self.question = ResearchQuestion("t3s3", "host capabilities", frozenset({"esp32", "psram", "radio", "usb"}))
        self.policy = InternetHostPolicy(frozenset({"wiki.lilygo.cc", "documentation.espressif.com"}))

    def test_useful_official_evidence_is_accepted_but_not_executable(self):
        record = make_record(
            "https://wiki.lilygo.cc/products/t3-series/t3-s3-v1.3/",
            "T3-S3 V1.3 specifications",
            "ESP32-S3FH4R2 dual-core LX7 at 240 MHz; 4 MB Flash; 2 MB QSPI PSRAM; USB-C; SX1262 variants.",
            ("esp32", "psram", "radio", "usb"),
        )
        result = ingest((record,), self.question, self.policy)
        self.assertEqual(result.decisions[0].status, EvidenceStatus.ACCEPTED)
        self.assertEqual(result.useful_claims, ("esp32", "psram", "radio", "usb"))
        self.assertFalse(result.executable)

    def test_unallowlisted_content_is_rejected(self):
        record = make_record("https://example.com/firmware.py", "download", "import os; os.system('bad')", ("esp32",))
        result = ingest((record,), self.question, self.policy)
        self.assertEqual(result.decisions[0].reason, "domain_not_allowlisted")
        self.assertEqual(result.accepted, ())

    def test_tampered_source_digest_is_rejected(self):
        record = make_record("https://documentation.espressif.com/esp32-s3_datasheet_en.pdf", "datasheet", "ESP32-S3 memory", ("psram",))
        tampered = record.__class__(**{**record.__dict__, "content": record.content + " altered"})
        result = ingest((tampered,), self.question, self.policy)
        self.assertEqual(result.decisions[0].reason, "digest_mismatch")

    def test_no_claims_never_becomes_training_authority(self):
        record = make_record("https://wiki.lilygo.cc/products/t3-series/t3-s3-v1.3/", "empty", "facts", ())
        result = ingest((record,), self.question, self.policy)
        self.assertEqual(result.decisions[0].status, EvidenceStatus.NEEDS_REVIEW)
        self.assertFalse(result.executable)


if __name__ == "__main__":
    unittest.main()
