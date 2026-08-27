"""Fetch one official MInDS-14 pt-PT asset and emit redacted technical evidence.

The asset is written only to a caller-selected temporary path. No sentence,
source filename, intent label or signed URL is printed or persisted in the
result. This is a local research probe, not a dataset redistribution tool.
"""
from __future__ import annotations

import argparse
import hashlib
import json
import socket
import struct
import subprocess
import urllib.parse
import urllib.request
from pathlib import Path
from typing import Any


API_URL = (
    "https://datasets-server.huggingface.co/first-rows?"
    "dataset=PolyAI%2Fminds14&config=pt-PT&split=train&offset=0&length=1"
)
EXPECTED_DATASET = "PolyAI/minds14"
EXPECTED_CONFIG = "pt-PT"
EXPECTED_SPLIT = "train"
MAX_AUDIO_BYTES = 2 * 1024 * 1024


def sha256_bytes(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def redacted_hash(value: str) -> str:
    return sha256_bytes(value.encode("utf-8"))


def fetch_json(url: str) -> dict[str, Any]:
    request = urllib.request.Request(url, headers={"User-Agent": "herus-cycle06-research"})
    with urllib.request.urlopen(request, timeout=60) as response:
        payload = json.loads(response.read())
    if not isinstance(payload, dict):
        raise ValueError("api_root_not_object")
    return payload


def fetch_limited(url: str, output: Path) -> bytes:
    request = urllib.request.Request(url, headers={"User-Agent": "herus-cycle06-research"})
    with urllib.request.urlopen(request, timeout=60) as response:
        chunks: list[bytes] = []
        total = 0
        while True:
            block = response.read(64 * 1024)
            if not block:
                break
            total += len(block)
            if total > MAX_AUDIO_BYTES:
                raise ValueError("audio_asset_exceeds_local_limit")
            chunks.append(block)
    data = b"".join(chunks)
    output.write_bytes(data)
    return data


def parse_wav_header(data: bytes) -> dict[str, int]:
    if len(data) < 12 or data[:4] != b"RIFF" or data[8:12] != b"WAVE":
        raise ValueError("wav_header_invalid")
    offset = 12
    fmt: tuple[int, int, int, int, int, int] | None = None
    data_size: int | None = None
    while offset + 8 <= len(data):
        chunk_id = data[offset : offset + 4]
        chunk_size = struct.unpack_from("<I", data, offset + 4)[0]
        chunk_start = offset + 8
        chunk_end = chunk_start + chunk_size
        if chunk_end > len(data):
            raise ValueError("wav_chunk_truncated")
        if chunk_id == b"fmt ":
            if chunk_size < 16:
                raise ValueError("wav_fmt_too_short")
            fmt = struct.unpack_from("<HHIIHH", data, chunk_start)
        elif chunk_id == b"data":
            data_size = chunk_size
        offset = chunk_end + (chunk_size & 1)
    if fmt is None or data_size is None:
        raise ValueError("wav_fmt_or_data_missing")
    audio_format, channels, sample_rate, byte_rate, block_align, bits_per_sample = fmt
    if audio_format not in (1, 6, 7):
        raise ValueError(f"wav_audio_format_unsupported:{audio_format}")
    if channels < 1 or sample_rate < 1 or block_align < 1 or data_size < 1:
        raise ValueError("wav_audio_parameters_invalid")
    frames = data_size // block_align
    return {
        "audio_format": audio_format,
        "channels": channels,
        "sample_rate": sample_rate,
        "byte_rate": byte_rate,
        "block_align": block_align,
        "bits_per_sample": bits_per_sample,
        "data_bytes": data_size,
        "frames": frames,
        "duration_ms": round(1000 * frames / sample_rate),
    }


def run_c_parser(c_runner: Path, sentence: str) -> dict[str, int]:
    if not c_runner.is_file():
        raise FileNotFoundError(c_runner)
    if "\n" in sentence or "\r" in sentence:
        raise ValueError("sentence_contains_line_break")
    process = subprocess.run(
        [str(c_runner)],
        input=sentence.encode("utf-8") + b"\n",
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        check=False,
    )
    if process.returncode != 0:
        raise RuntimeError(
            f"c_runner_failed:{process.returncode}:{process.stderr.decode('utf-8', errors='replace')}"
        )
    result: dict[str, int] = {}
    for line in process.stdout.decode("utf-8", errors="strict").splitlines():
        key, value = line.split("=", 1)
        result[key] = int(value)
    if result.get("parser_input_rows") != 1:
        raise RuntimeError("c_runner_single_row_mismatch")
    return result


def audit(output: Path, c_runner: Path | None = None) -> dict[str, Any]:
    payload = fetch_json(API_URL)
    if payload.get("dataset") != EXPECTED_DATASET:
        raise ValueError("dataset_mismatch")
    if payload.get("config") != EXPECTED_CONFIG:
        raise ValueError("config_mismatch")
    if payload.get("split") != EXPECTED_SPLIT:
        raise ValueError("split_mismatch")
    rows = payload.get("rows")
    if not isinstance(rows, list) or not rows:
        raise ValueError("no_rows")
    row = rows[0].get("row")
    if not isinstance(row, dict):
        raise ValueError("row_not_object")
    audio_cells = row.get("audio")
    if not isinstance(audio_cells, list) or len(audio_cells) != 1:
        raise ValueError("audio_cell_missing_or_ambiguous")
    audio_cell = audio_cells[0]
    if not isinstance(audio_cell, dict) or audio_cell.get("type") != "audio/wav":
        raise ValueError("audio_type_mismatch")
    asset_url = audio_cell.get("src")
    if not isinstance(asset_url, str) or not asset_url.startswith("https://"):
        raise ValueError("audio_url_invalid")
    parsed_url = urllib.parse.urlparse(asset_url)
    if parsed_url.hostname != "datasets-server.huggingface.co":
        raise ValueError("audio_host_not_official_datasets_server")
    for field in ("path", "transcription", "english_transcription", "intent_class", "lang_id"):
        if field not in row:
            raise ValueError(f"missing_field:{field}")
    data = fetch_limited(asset_url, output)
    wav = parse_wav_header(data)
    parser = run_c_parser(c_runner, str(row["transcription"])) if c_runner else {}
    return {
        "dataset": EXPECTED_DATASET,
        "config": EXPECTED_CONFIG,
        "split": EXPECTED_SPLIT,
        "hub_revision": "40ce77cb32a384e4d50a568e1ec39ac804019d33",
        "row_index": 0,
        "path_identity_hashed_locally": True,
        "path_hash_published": False,
        "transcription_present": bool(row["transcription"]),
        "english_transcription_present": bool(row["english_transcription"]),
        "intent_class_present": isinstance(row["intent_class"], int),
        "lang_id_present": isinstance(row["lang_id"], int),
        "asset_host": socket.getfqdn(parsed_url.hostname),
        "asset_type": audio_cell["type"],
        "audio_bytes": len(data),
        "audio_sha256": sha256_bytes(data),
        "wav_audio_format": wav["audio_format"],
        "wav_channels": wav["channels"],
        "wav_sample_width_bytes": wav["bits_per_sample"] // 8,
        "wav_sample_rate_hz": wav["sample_rate"],
        "wav_frames": wav["frames"],
        "wav_data_bytes": wav["data_bytes"],
        "wav_duration_ms": wav["duration_ms"],
        "wav_decode_performed": False,
        "sentence_published": False,
        "individual_id_published": False,
        "intent_mapped_to_herus": False,
        "herus_convergence_proven": False,
        "parser_executed_on_same_record_transcription": bool(c_runner),
        **{key: value for key, value in parser.items() if key.startswith("parser_")},
        "parser_automatic_label_mapping": parser.get("automatic_label_mapping", 0),
        "parser_herus_command_authority": parser.get("herus_command_authority", 0),
        "verdict": "single_audio_text_intent_record_locally_integrity_checked; semantic_herus_convergence_not_proven",
    }


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--output-audio", type=Path, required=True)
    parser.add_argument("--output-json", type=Path, required=True)
    parser.add_argument("--c-runner", type=Path)
    args = parser.parse_args()
    result = audit(args.output_audio, args.c_runner)
    args.output_json.write_text(json.dumps(result, indent=2) + "\n", encoding="utf-8")
    print(json.dumps(result, indent=2))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
