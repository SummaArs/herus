import hashlib
import json
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parent
MANIFEST_PATH = ROOT / "datasets_manifest.json"
AUDIT_PATH = ROOT / "evidence" / "wide_cycle_04" / "mintrec_metadata_audit.json"
DATA_ROOT = ROOT / "source_data" / "MIntRec" / "metadata"


class DatasetManifestTests(unittest.TestCase):
    def setUp(self):
        self.manifest = json.loads(MANIFEST_PATH.read_text(encoding="utf-8"))
        self.datasets = {item["id"]: item for item in self.manifest["datasets"]}

    def test_policy_is_fail_closed(self):
        policy = self.manifest["policy"]
        self.assertFalse(policy["raw_data_commit"])
        self.assertFalse(policy["synthetic_pairing"])
        self.assertFalse(policy["automatic_label_mapping_to_herus"])
        self.assertTrue(policy["multimodal_claim_requires_same_sample_id_source_and_declared_alignment"])

    def test_required_real_sources_are_classified(self):
        self.assertEqual(set(self.datasets), {
            "mintrec",
            "common_voice_pt_scripted_v26",
            "wesad",
            "slurp",
            "fluent_speech_commands",
        })
        for item in self.datasets.values():
            for field in (
                "source_urls", "version_or_snapshot", "license_or_terms",
                "license_confirmed_for_raw_data", "redistribution_status",
                "modalities", "alignment_class", "download_status",
                "local_files", "herus_label_mapping", "can_prove", "cannot_prove",
            ):
                self.assertIn(field, item, item["id"])
            self.assertIn(item["alignment_class"], {
                "PAIRED_DECLARED_BY_SOURCE",
                "INTRAMODAL_PAIRED_AUDIO_TRANSCRIPT",
                "PAIRED_INTERNAL_SENSOR_STREAMS",
            })
            self.assertEqual(item["herus_label_mapping"]["mapping_count"], 0)
        slurp = self.datasets["slurp"]
        self.assertEqual(slurp["source_commit"], "8eb16545762be97ace75334109d73824217311f1")
        self.assertEqual(slurp["download_status"]["audio"], "not_downloaded_due_cc_by_nc_4")
        self.assertEqual(slurp["herus_label_mapping"]["mapping_count"], 0)

    def test_mintrec_metadata_artifact_is_non_promotional(self):
        audit = json.loads(AUDIT_PATH.read_text(encoding="utf-8"))
        self.assertEqual(audit["total_rows"], 1779)
        self.assertEqual(audit["unique_segment_ids"], 1779)
        self.assertEqual(audit["automatic_mapping_count"], 0)
        self.assertTrue(audit["verdict"]["metadata_integrity"])
        self.assertFalse(audit["verdict"]["split_leakage"])
        self.assertFalse(audit["verdict"]["herus_convergence_proven"])
        self.assertIn("aligned audio/video", audit["verdict"]["reason"])

    def test_mintrec_hashes_and_counts_when_downloaded(self):
        expected = {
            "train.tsv": (1334, 75488, "348289d7d140b2c0716b35a03e7045b684c4b90f5c30d1281f9a6b7cb3b9903b"),
            "test.tsv": (445, 26287, "e4394869fc02898e00fad0a1b03e0333760a9b0499da2fa3f8af030a384c8f3a"),
        }
        for name, (rows, size, digest) in expected.items():
            path = DATA_ROOT / name
            if not path.exists():
                self.skipTest("real MIntRec metadata not downloaded: " + str(path))
            self.assertEqual(path.stat().st_size, size)
            actual = hashlib.sha256(path.read_bytes()).hexdigest()
            self.assertEqual(actual, digest)
            with path.open("r", encoding="utf-8") as handle:
                self.assertEqual(sum(1 for _ in handle) - 1, rows)


if __name__ == "__main__":
    unittest.main()
