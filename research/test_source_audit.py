import json
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parent
AUDIT = ROOT / "evidence" / "wide_cycle_05" / "source_audit.json"
SLURP_METADATA_AUDIT = ROOT / "evidence" / "wide_cycle_05" / "slurp_metadata_audit.json"
SLURP_TEXT_AUDIT = ROOT / "evidence" / "wide_cycle_05" / "slurp_text_audit.json"
SLURP_VOICE_AUDIT = ROOT / "evidence" / "wide_cycle_05" / "slurp_voice_parser_audit.json"
MIND14_SINGLE_AUDIT = ROOT / "evidence" / "wide_cycle_06" / "minds14_single_sample_audit.json"
MIND14_BATCH_AUDIT = ROOT / "evidence" / "wide_cycle_06" / "minds14_batch_audit.json"
MIND14_FULL_AUDIT = ROOT / "evidence" / "wide_cycle_06" / "minds14_ptpt_train_audit.json"
SLURP_IDENTITY_AUDIT = ROOT / "evidence" / "wide_cycle_06" / "slurp_identity_audit.json"
CYCLE06_SOURCE_AUDIT = ROOT / "evidence" / "wide_cycle_06" / "source_audit.json"


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

    def test_cycle06_artifacts_are_non_promotional_and_redacted(self):
        single = json.loads(MIND14_SINGLE_AUDIT.read_text(encoding="utf-8"))
        batch = json.loads(MIND14_BATCH_AUDIT.read_text(encoding="utf-8"))
        identity = json.loads(SLURP_IDENTITY_AUDIT.read_text(encoding="utf-8"))
        full = json.loads(MIND14_FULL_AUDIT.read_text(encoding="utf-8"))
        self.assertNotIn("path_hash", single)
        self.assertFalse(single["path_hash_published"])
        self.assertFalse(single["herus_convergence_proven"])
        self.assertEqual(single["audio_sha256"], "fc084982ad50c6ea6cf066f08374b9b3aaa628d9a9accb167be5ae9376dbd275")
        self.assertEqual(single["parser_automatic_label_mapping"], 0)
        self.assertEqual(single["parser_herus_command_authority"], 0)
        self.assertEqual(batch["records"], 4)
        self.assertEqual(batch["valid_wav_records"], 4)
        self.assertEqual(batch["duplicate_path_hashes"], 0)
        self.assertEqual(batch["duplicate_audio_hashes"], 0)
        self.assertFalse(batch["audio_sha256_values_published"])
        self.assertTrue(batch["audio_deleted_after_audit"])
        self.assertEqual(batch["parser_automatic_label_mapping"], 0)
        self.assertEqual(batch["parser_herus_command_authority"], 0)
        self.assertEqual(identity["text_rows"], 16521)
        self.assertEqual(identity["metadata_duplicate_filename_count"], 1)
        self.assertEqual(identity["text_repeated_reference_count"], 1)
        self.assertEqual(identity["identity_status"], "AMBIGUOUS")
        self.assertFalse(identity["identity_gate_passed"])
        self.assertFalse(identity["individual_identifiers_published"])
        self.assertFalse(identity["individual_filenames_published"])
        self.assertFalse(identity["herus_convergence_proven"])
        self.assertEqual(full["rows_total"], 604)
        self.assertEqual(full["rows_seen"], 604)
        self.assertEqual(full["audio_cells_present_and_official_host"], 604)
        self.assertEqual(full["audio_bytes_fetched"], 0)
        self.assertFalse(full["audio_decode_performed"])
        self.assertEqual(full["transcriptions_present"], 604)
        self.assertEqual(full["distinct_external_intent_ids"], 14)
        self.assertEqual(full["parser_input_rows"], 604)
        self.assertEqual(full["parser_draft"], 0)
        self.assertEqual(full["parser_cancel"], 0)
        self.assertEqual(full["parser_unknown"], 53)
        self.assertEqual(full["parser_rejected"], 551)
        self.assertEqual(full["parser_non_ascii"], 545)
        self.assertEqual(full["automatic_label_mapping"], 0)
        self.assertEqual(full["herus_command_authority"], 0)
        self.assertFalse(full["individual_paths_published"])
        self.assertFalse(full["individual_sentences_published"])
        self.assertFalse(full["herus_convergence_proven"])

    def test_cycle06_structured_dossier_is_fail_closed(self):
        dossier = json.loads(CYCLE06_SOURCE_AUDIT.read_text(encoding="utf-8"))
        self.assertFalse(dossier["policy"]["synthetic_pairing"])
        self.assertFalse(dossier["policy"]["herus_automatic_label_mapping"])
        self.assertTrue(dossier["policy"]["ambiguous_identity_blocks_pair"])
        self.assertFalse(dossier["verdict"]["paired_audio_text_intent_locally_verified"])
        self.assertFalse(dossier["verdict"]["herus_convergence_proven"])
        self.assertEqual(dossier["verdict"]["herus_command_authority_from_external_data"], 0)
        self.assertFalse(dossier["raw_audio_published"])
        minds14 = next(source for source in dossier["sources"] if source["id"] == "minds14")
        full = next(source for source in dossier["sources"] if source["id"] == "minds14_full_text")
        self.assertEqual(minds14["sample_records"], 4)
        self.assertEqual(minds14["audio_records_structurally_valid"], 4)
        self.assertFalse(minds14["audio_decode_performed"])
        self.assertTrue(minds14["audio_deleted_after_audit"])
        self.assertFalse(minds14["herus_convergence_proven"])
        self.assertEqual(full["rows_total"], 604)
        self.assertEqual(full["rows_seen"], 604)
        self.assertEqual(full["audio_bytes_fetched"], 0)
        self.assertEqual(full["parser_draft"], 0)
        self.assertEqual(full["parser_cancel"], 0)
        self.assertEqual(full["parser_unknown"], 53)
        self.assertEqual(full["parser_rejected"], 551)
        self.assertEqual(full["automatic_label_mapping"], 0)
        self.assertEqual(full["herus_command_authority"], 0)
        self.assertFalse(full["herus_convergence_proven"])
        protocol = next(source for source in dossier["sources"] if source["id"] == "herus_annotation_protocol_v1")
        self.assertEqual(protocol["bridge_path"], "none")
        self.assertEqual(protocol["external_label_mapping"], "forbidden")
        self.assertFalse(protocol["herus_convergence_proven"])
        slurp = next(source for source in dossier["sources"] if source["id"] == "slurp_identity")
        self.assertEqual(slurp["identity_status"], "AMBIGUOUS")
        self.assertFalse(slurp["identity_gate_passed"])

    def test_mintrec_is_metadata_only(self):
        mintrec = next(source for source in self.audit["sources"] if source["id"] == "mintrec")
        self.assertEqual(mintrec["result"], "metadata_integrity_only")
        self.assertEqual(len(mintrec["local_files"]), 2)
        self.assertIn("Audio/video were not downloaded", mintrec["limitation"])


if __name__ == "__main__":
    unittest.main()
