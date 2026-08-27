import csv
import tempfile
import unittest
import sys
import wave
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from paired_audio_audit import GateError, verify_package


ROOT = Path(__file__).resolve().parent
MANIFEST = ROOT / "datasets_manifest.json"


class PairedAudioAuditTests(unittest.TestCase):
    """These are format/gate fixtures, not dataset measurements."""

    def write_csv(self, root: Path, rows: list[dict[str, str]], name: str = "train.csv") -> Path:
        path = root / name
        with path.open("w", encoding="utf-8", newline="") as handle:
            writer = csv.DictWriter(
                handle,
                fieldnames=["speakerId", "path", "transcription", "action", "object", "location"],
            )
            writer.writeheader()
            writer.writerows(rows)
        return path

    def write_wav(self, root: Path, relative: str, rate: int = 16000, channels: int = 1) -> None:
        path = root / relative
        path.parent.mkdir(parents=True, exist_ok=True)
        with wave.open(str(path), "wb") as handle:
            handle.setnchannels(channels)
            handle.setsampwidth(2)
            handle.setframerate(rate)
            handle.writeframes(b"\x00\x00" * 160)

    def row(self, path: str = "audio/a.wav") -> dict[str, str]:
        return {
            "speakerId": "speaker-1",
            "path": path,
            "transcription": "turn on the lights",
            "action": "activate",
            "object": "lights",
            "location": "none",
        }

    def test_valid_local_fixture_checks_same_csv_path_and_wav(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            csv_path = self.write_csv(root, [self.row()])
            self.write_wav(root, "audio/a.wav")
            result = verify_package(MANIFEST, "fluent_speech_commands", csv_path, root)
            self.assertEqual(result.verdict, "paired_structure_verified")
            self.assertEqual(result.records_checked, 1)
            self.assertEqual(result.unique_sample_ids, 1)
            self.assertEqual(result.wav_mono_16khz, 1)
            self.assertEqual(result.herus_mapping_count, 0)
            self.assertEqual(len(result.csv_sha256), 64)
            self.assertEqual(len(result.audio_sha256_aggregate), 64)

    def test_license_gate_precedes_missing_file_for_mintrec(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            with self.assertRaisesRegex(GateError, "license_unverified") as context:
                verify_package(MANIFEST, "mintrec", root / "missing.csv", root)
            self.assertEqual(context.exception.code, "license_unverified")

    def test_missing_csv_is_blocked_for_licensed_dataset(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            with self.assertRaisesRegex(GateError, "missing") as context:
                verify_package(MANIFEST, "fluent_speech_commands", root / "missing.csv", root)
            self.assertEqual(context.exception.code, "missing")

    def test_path_traversal_is_blocked(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            csv_path = self.write_csv(root, [self.row("../escape.wav")])
            with self.assertRaisesRegex(GateError, "unsafe_audio_path"):
                verify_package(MANIFEST, "fluent_speech_commands", csv_path, root)

    def test_duplicate_audio_paths_are_blocked(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            csv_path = self.write_csv(root, [self.row(), self.row()])
            self.write_wav(root, "audio/a.wav")
            with self.assertRaisesRegex(GateError, "duplicate_sample_id"):
                verify_package(MANIFEST, "fluent_speech_commands", csv_path, root)

    def test_wrong_wav_format_is_blocked(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            csv_path = self.write_csv(root, [self.row()])
            self.write_wav(root, "audio/a.wav", rate=8000, channels=2)
            with self.assertRaisesRegex(GateError, "wav_format_mismatch"):
                verify_package(MANIFEST, "fluent_speech_commands", csv_path, root)


if __name__ == "__main__":
    unittest.main()
