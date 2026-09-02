import json
import struct
import sys
import tempfile
import unittest
from unittest.mock import patch
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from fetch_minds14_sample import (
    MAX_AUDIO_BYTES,
    audit,
    fetch_json,
    fetch_limited,
    parse_wav_header,
    run_c_parser,
)


def wav_fixture(audio_format: int = 7, payload: bytes = b"\x00\x01") -> bytes:
    fmt = struct.pack("<HHIIHH", audio_format, 1, 8000, 8000, 1, 8)
    body = b"fmt " + struct.pack("<I", len(fmt)) + fmt
    body += b"data" + struct.pack("<I", len(payload)) + payload
    return b"RIFF" + struct.pack("<I", len(body) + 4) + b"WAVE" + body


class Minds14SampleTests(unittest.TestCase):
    def test_g711_mulaw_wav_is_structurally_valid(self):
        result = parse_wav_header(wav_fixture(7, b"\x00" * 8000))
        self.assertEqual(result["audio_format"], 7)
        self.assertEqual(result["channels"], 1)
        self.assertEqual(result["sample_rate"], 8000)
        self.assertEqual(result["frames"], 8000)
        self.assertEqual(result["duration_ms"], 1000)

    def test_pcm_and_alaw_are_also_structurally_valid(self):
        self.assertEqual(parse_wav_header(wav_fixture(1))["audio_format"], 1)
        self.assertEqual(parse_wav_header(wav_fixture(6))["audio_format"], 6)

    def test_unknown_format_is_rejected(self):
        with self.assertRaisesRegex(ValueError, "wav_audio_format_unsupported"):
            parse_wav_header(wav_fixture(3))

    def test_truncated_chunk_is_rejected(self):
        data = wav_fixture()[:-1]
        with self.assertRaisesRegex(ValueError, "wav_chunk_truncated"):
            parse_wav_header(data)

    def test_missing_data_chunk_is_rejected(self):
        fmt = struct.pack("<HHIIHH", 7, 1, 8000, 8000, 1, 8)
        data = b"RIFF" + struct.pack("<I", len(fmt) + 12) + b"WAVEfmt " + struct.pack("<I", len(fmt)) + fmt
        with self.assertRaisesRegex(ValueError, "wav_fmt_or_data_missing"):
            parse_wav_header(data)

    def test_malformed_json_is_rejected(self):
        class Response:
            def __enter__(self):
                return self

            def __exit__(self, *_):
                return False

            def read(self):
                return b"not-json"

        with patch("fetch_minds14_sample.urllib.request.urlopen", return_value=Response()):
            with self.assertRaises(json.JSONDecodeError):
                fetch_json("https://example.invalid/rows")

    def test_embedded_newline_is_rejected_before_runner(self):
        with self.assertRaisesRegex(ValueError, "sentence_contains_line_break"):
            run_c_parser(Path(sys.executable), "ajuda\npara pagar")

    def test_non_official_audio_host_is_rejected(self):
        payload = {
            "dataset": "PolyAI/minds14",
            "config": "pt-PT",
            "split": "train",
            "rows": [{
                "row": {
                    "path": "synthetic-fixture.wav",
                    "audio": [{"type": "audio/wav", "src": "https://evil.invalid/audio.wav"}],
                    "transcription": "socorro",
                    "english_transcription": "help",
                    "intent_class": 0,
                    "lang_id": 0,
                }
            }],
        }
        with tempfile.TemporaryDirectory() as directory:
            with patch("fetch_minds14_sample.fetch_json", return_value=payload):
                with self.assertRaisesRegex(ValueError, "audio_host_not_official_datasets_server"):
                    audit(Path(directory) / "sample.wav")

    def test_oversized_audio_is_rejected_before_write(self):
        class Response:
            def __enter__(self):
                return self

            def __exit__(self, *_):
                return False

            def read(self, _block_size):
                return b"x" * (MAX_AUDIO_BYTES + 1)

        with tempfile.TemporaryDirectory() as directory:
            output = Path(directory) / "sample.wav"
            with patch("fetch_minds14_sample.urllib.request.urlopen", return_value=Response()):
                with self.assertRaisesRegex(ValueError, "audio_asset_exceeds_local_limit"):
                    fetch_limited("https://datasets-server.huggingface.co/audio.wav", output)
            self.assertFalse(output.exists())


if __name__ == "__main__":
    unittest.main()
