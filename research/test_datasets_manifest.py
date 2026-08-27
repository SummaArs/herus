import hashlib
import json
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parent
MANIFEST_PATH = ROOT / "datasets_manifest.json"
AUDIT_PATH = ROOT / "evidence" / "wide_cycle_04" / "mintrec_metadata_audit.json"
MIND14_SINGLE_PATH = ROOT / "evidence" / "wide_cycle_06" / "minds14_single_sample_audit.json"
MIND14_BATCH_PATH = ROOT / "evidence" / "wide_cycle_06" / "minds14_batch_audit.json"
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
            "minds14",
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
        minds14 = self.datasets["minds14"]
        self.assertEqual(minds14["source_commit"], "40ce77cb32a384e4d50a568e1ec39ac804019d33")
        self.assertEqual(minds14["alignment_class"], "PAIRED_DECLARED_BY_SOURCE")
        self.assertFalse(minds14["license_confirmed_for_raw_data"])
        self.assertEqual(minds14["herus_label_mapping"]["mapping_count"], 0)

    def test_mintrec_metadata_artifact_is_non_promotional(self):
        audit = json.loads(AUDIT_PATH.read_text(encoding="utf-8"))
        self.assertEqual(audit["total_rows"], 1779)
        self.assertEqual(audit["unique_segment_ids"], 1779)
        self.assertEqual(audit["automatic_mapping_count"], 0)
        self.assertTrue(audit["verdict"]["metadata_integrity"])
        self.assertFalse(audit["verdict"]["split_leakage"])
        self.assertFalse(audit["verdict"]["herus_convergence_proven"])
        self.assertIn("aligned audio/video", audit["verdict"]["reason"])

    def test_minds14_sample_artifacts_are_non_promotional(self):
        if not MIND14_SINGLE_PATH.exists() or not MIND14_BATCH_PATH.exists():
            self.skipTest("real MInDS-14 sample artifacts not generated")
        single = json.loads(MIND14_SINGLE_PATH.read_text(encoding="utf-8"))
        batch = json.loads(MIND14_BATCH_PATH.read_text(encoding="utf-8"))
        self.assertFalse(single["herus_convergence_proven"])
        self.assertFalse(single["intent_mapped_to_herus"])
        self.assertFalse(single["individual_id_published"])
        self.assertEqual(single["audio_sha256"], "fc084982ad50c6ea6cf066f08374b9b3aaa628d9a9accb167be5ae9376dbd275")
        self.assertEqual(single["wav_audio_format"], 7)
        self.assertEqual(single["parser_automatic_label_mapping"], 0)
        self.assertEqual(single["parser_herus_command_authority"], 0)
        self.assertFalse(batch["herus_convergence_proven"])
        self.assertFalse(batch["intent_mapped_to_herus"])
        self.assertTrue(batch["audio_deleted_after_audit"])
        self.assertFalse(batch["audio_sha256_values_published"])
        self.assertEqual(batch["records"], 4)
        minds14 = next(item for item in json.loads(MANIFEST_PATH.read_text(encoding="utf-8"))["datasets"] if item["id"] == "minds14")
        local_files = {item["path"]: item["sha256"] for item in minds14["local_files"]}
        self.assertEqual(local_files["research/evidence/wide_cycle_06/minds14_single_sample_audit.json"], "b6d5ae3e711c0b068bbb2332da1ce73277d0a2efc786958aed133101d5c0381e")
        self.assertEqual(local_files["research/evidence/wide_cycle_06/minds14_batch_audit.json"], "6a3bf548e1dc17efc2d41a2de1fda950436c894de577c0d659d8ef7fffd7945c")
        pending_marker = "pending_after_" + "cycle_artifact_freeze"
        self.assertNotIn(pending_marker, json.dumps(minds14))
        self.assertEqual(batch["valid_wav_records"], 4)
        self.assertEqual(batch["duplicate_path_hashes"], 0)
        self.assertEqual(batch["duplicate_audio_hashes"], 0)
        self.assertEqual(batch["parser_automatic_label_mapping"], 0)
        self.assertEqual(batch["parser_herus_command_authority"], 0)

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
