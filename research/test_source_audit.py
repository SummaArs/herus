import json
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parent
AUDIT = ROOT / "evidence" / "wide_cycle_05" / "source_audit.json"
SLURP_METADATA_AUDIT = ROOT / "evidence" / "wide_cycle_05" / "slurp_metadata_audit.json"
SLURP_TEXT_AUDIT = ROOT / "evidence" / "wide_cycle_05" / "slurp_text_audit.json"
SLURP_VOICE_AUDIT = ROOT / "evidence" / "wide_cycle_05" / "slurp_voice_parser_audit.json"


class SourceAuditTests(unittest.TestCase):
    def setUp(self):
        self.audit = json.loads(AUDIT.read_text(encoding="utf-8"))

    def test_policy_forbids_unverified_mirrors_and_raw_media_commit(self):
        policy = self.audit["policy"]
        self.assertFalse(policy["download_without_license_confirmation"])
        self.assertFalse(policy["download_full_large_archive_without_need"])
        self.assertFalse(policy["use_mirror_without_lineage"])
        self.assertFalse(policy["synthetic_pairing"])
        self.assertFalse(policy["raw_media_commit"])

    def test_every_source_declares_local_result_and_limitation(self):
        self.assertGreaterEqual(len(self.audit["sources"]), 4)
        for source in self.audit["sources"]:
            self.assertIn("access_status", source)
            self.assertIn("local_files", source)
            self.assertIn("result", source)
            self.assertIn("limitation", source)

    def test_final_verdict_is_not_promoted(self):
        verdict = self.audit["verdict"]
        self.assertFalse(verdict["paired_audio_text_intent_locally_verified"])
        self.assertFalse(verdict["herus_convergence_proven"])

    def test_zenodo_probe_cannot_be_called_full_content_validation(self):
        zenodo = next(source for source in self.audit["sources"] if source["id"] == "slurp_zenodo_11106554")
        self.assertEqual(zenodo["access_status"], "central_directory_verified_content_blocked")
        self.assertTrue(zenodo["range_probe"]["central_directory_fully_read"])
        self.assertEqual(zenodo["range_probe"]["member_count"], 141759)
        self.assertEqual(zenodo["result"], "blocked")
        self.assertIn("CC BY-NC", zenodo["limitation"])

    def test_slurp_aggregate_artifacts_are_non_promotional(self):
        metadata = json.loads(SLURP_METADATA_AUDIT.read_text(encoding="utf-8"))
        text = json.loads(SLURP_TEXT_AUDIT.read_text(encoding="utf-8"))
        voice = json.loads(SLURP_VOICE_AUDIT.read_text(encoding="utf-8"))
        self.assertEqual(metadata["recordings"], 72396)
        self.assertFalse(metadata["audio_loaded"])
        self.assertFalse(metadata["verdict"]["herus_convergence_proven"])
        self.assertEqual(text["text_rows"], 16521)
        self.assertEqual(text["unique_slurp_ids"], 16521)
        self.assertEqual(text["metadata_joined_rows_by_recording_filename"], 16521)
        self.assertEqual(text["missing_recording_references"], 0)
        self.assertEqual(text["metadata_duplicate_recording_names"], 1)
        self.assertFalse(text["verdict"]["text_recording_metadata_alignment_verified"])
        self.assertFalse(text["sentences_published_in_result"])
        self.assertFalse(text["individual_ids_published_in_result"])
        self.assertEqual(voice["jsonl_rows_seen"], 16521)
        self.assertEqual(voice["parser_draft"], 0)
        self.assertEqual(voice["herus_command_authority"], 0)

    def test_mintrec_is_metadata_only(self):
        mintrec = next(source for source in self.audit["sources"] if source["id"] == "mintrec")
        self.assertEqual(mintrec["result"], "metadata_integrity_only")
        self.assertEqual(len(mintrec["local_files"]), 2)
        self.assertIn("Audio/video were not downloaded", mintrec["limitation"])


if __name__ == "__main__":
    unittest.main()
