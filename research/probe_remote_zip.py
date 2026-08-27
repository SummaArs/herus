"""Probe a remote ZIP with bounded HTTP Range requests.

The probe is intentionally not a downloader. It fetches only the final byte
range, verifies the server response, and reports central-directory names when
that directory is fully contained in the range. It never extracts or executes
archive members.
"""
from __future__ import annotations

import argparse
import json
import re
import sys
import urllib.request
from pathlib import Path


class ProbeError(RuntimeError):
    pass


def _u16(data: bytes, offset: int) -> int:
    return int.from_bytes(data[offset : offset + 2], "little")


def _u32(data: bytes, offset: int) -> int:
    return int.from_bytes(data[offset : offset + 4], "little")


def _u64(data: bytes, offset: int) -> int:
    return int.from_bytes(data[offset : offset + 8], "little")


def _parse_eocd(data: bytes, absolute_start: int, total_size: int) -> tuple[int, int, str]:
    marker = b"PK\x05\x06"
    position = data.rfind(marker)
    if position < 0 or position + 22 > len(data):
        raise ProbeError("end_of_central_directory_not_in_range")
    cd_size = _u32(data, position + 12)
    cd_offset = _u32(data, position + 16)
    if cd_size != 0xFFFFFFFF and cd_offset != 0xFFFFFFFF:
        return cd_offset, cd_size, "zip32"

    locator = data.rfind(b"PK\x06\x07", 0, position)
    if locator < 0 or locator + 20 > len(data):
        raise ProbeError("zip64_locator_not_in_range")
    zip64_offset = _u64(data, locator + 8)
    local = zip64_offset - absolute_start
    if local < 0 or local + 56 > len(data) or data[local : local + 4] != b"PK\x06\x06":
        raise ProbeError("zip64_end_of_central_directory_not_in_range")
    cd_size = _u64(data, local + 40)
    cd_offset = _u64(data, local + 48)
    return cd_offset, cd_size, "zip64"


def _central_entries(data: bytes, absolute_start: int, cd_offset: int, cd_size: int) -> list[dict[str, int | str]]:
    local = cd_offset - absolute_start
    if local < 0 or local + cd_size > len(data):
        raise ProbeError("central_directory_not_fully_in_range")
    end = local + cd_size
    entries: list[dict[str, int | str]] = []
    position = local
    while position < end:
        if data[position : position + 4] != b"PK\x01\x02":
            raise ProbeError(f"invalid_central_directory_signature_at_{absolute_start + position}")
        if position + 46 > end:
            raise ProbeError("truncated_central_directory_entry")
        filename_length = _u16(data, position + 28)
        extra_length = _u16(data, position + 30)
        comment_length = _u16(data, position + 32)
        entry_end = position + 46 + filename_length + extra_length + comment_length
        if entry_end > end:
            raise ProbeError("central_directory_entry_out_of_range")
        raw_name = data[position + 46 : position + 46 + filename_length]
        entries.append({
            "name": raw_name.decode("utf-8", errors="replace"),
            "compression_method": _u16(data, position + 10),
            "crc32": _u32(data, position + 16),
            "compressed_size": _u32(data, position + 20),
            "uncompressed_size": _u32(data, position + 24),
            "local_header_offset": _u32(data, position + 42),
        })
        position = entry_end
    return entries


def _central_names(data: bytes, absolute_start: int, cd_offset: int, cd_size: int) -> list[str]:
    return [str(entry["name"]) for entry in _central_entries(data, absolute_start, cd_offset, cd_size)]


def probe(
    url: str,
    total_size: int,
    tail_bytes: int,
    suffixes: tuple[str, ...] = (),
    contains: tuple[str, ...] = (),
) -> dict[str, object]:
    if total_size <= 0 or tail_bytes <= 0 or tail_bytes > 32 * 1024 * 1024:
        raise ProbeError("invalid_bounded_range")
    absolute_start = max(0, total_size - tail_bytes)
    absolute_end = total_size - 1
    request = urllib.request.Request(url, headers={"Range": f"bytes={absolute_start}-{absolute_end}"})
    with urllib.request.urlopen(request, timeout=120) as response:
        body = response.read()
        status = getattr(response, "status", None)
        content_range = response.headers.get("Content-Range", "")
        content_length = response.headers.get("Content-Length", "")
    if status != 206:
        raise ProbeError(f"range_not_honored_status_{status}")
    expected_range = f"bytes {absolute_start}-{absolute_end}/{total_size}"
    if content_range != expected_range:
        raise ProbeError(f"content_range_mismatch:{content_range}")
    if len(body) != absolute_end - absolute_start + 1:
        raise ProbeError("body_length_mismatch")
    if content_length and int(content_length) != len(body):
        raise ProbeError("content_length_mismatch")

    cd_offset, cd_size, zip_mode = _parse_eocd(body, absolute_start, total_size)
    try:
        entries = _central_entries(body, absolute_start, cd_offset, cd_size)
        names = [str(entry["name"]) for entry in entries]
    except ProbeError as exc:
        return {
            "verdict": "blocked",
            "reason": str(exc),
            "url": url,
            "total_size": total_size,
            "range_start": absolute_start,
            "range_end": absolute_end,
            "downloaded_bytes": len(body),
            "zip_mode": zip_mode,
            "central_directory_offset": cd_offset,
            "central_directory_size": cd_size,
            "central_directory_end": cd_offset + cd_size,
            "raw_data_downloaded": False,
            "extracted": False,
            "executed": False,
        }
    preview = names if len(names) <= 64 else names[:32] + names[-32:]
    filtered = [
        entry for entry in entries
        if (suffixes and any(str(entry["name"]).lower().endswith(suffix.lower()) for suffix in suffixes))
        or (contains and any(fragment.lower() in str(entry["name"]).lower() for fragment in contains))
    ]
    filtered_preview = filtered if len(filtered) <= 64 else filtered[:32] + filtered[-32:]
    result = {
        "verdict": "structure_verified",
        "url": url,
        "total_size": total_size,
        "range_start": absolute_start,
        "range_end": absolute_end,
        "downloaded_bytes": len(body),
        "zip_mode": zip_mode,
        "central_directory_offset": cd_offset,
        "central_directory_size": cd_size,
        "member_count": len(names),
        "member_names_complete": len(names) <= 64,
        "member_names_preview": preview,
        "member_name_suffix_filters": list(suffixes),
        "member_name_contains_filters": list(contains),
        "filtered_member_count": len(filtered),
        "filtered_member_names_complete": len(filtered) <= 64,
        "filtered_member_names": [str(entry["name"]) for entry in filtered_preview],
        "filtered_member_entries": filtered_preview,
        "raw_data_downloaded": False,
        "extracted": False,
        "executed": False,
    }
    if len(names) <= 64:
        result["member_names"] = names
    return result


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--url", required=True)
    parser.add_argument("--size", type=int, required=True)
    parser.add_argument("--tail-bytes", type=int, default=4 * 1024 * 1024)
    parser.add_argument("--suffix", action="append", default=[])
    parser.add_argument("--contains", action="append", default=[])
    args = parser.parse_args(argv)
    try:
        result = probe(args.url, args.size, args.tail_bytes, tuple(args.suffix), tuple(args.contains))
    except (OSError, ProbeError, ValueError) as exc:
        print(json.dumps({"verdict": "blocked", "error": str(exc)}, ensure_ascii=False))
        return 2
    print(json.dumps(result, ensure_ascii=False, indent=2))
    return 0


if __name__ == "__main__":
    sys.exit(main())
