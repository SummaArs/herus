"""Fetch one ZIP member with bounded HTTP Range and verify it locally.

This utility is for metadata-only research audits. It accepts an explicit
central-directory location from a prior probe, fetches only the selected local
member, validates the local header, decompression, size and CRC, and never
executes archive content.
"""
from __future__ import annotations

import argparse
import binascii
import hashlib
import json
import sys
import urllib.request
import zlib
from pathlib import Path


class MemberError(RuntimeError):
    pass


def _u16(data: bytes, offset: int) -> int:
    return int.from_bytes(data[offset : offset + 2], "little")


def _u32(data: bytes, offset: int) -> int:
    return int.from_bytes(data[offset : offset + 4], "little")


def fetch_member(
    url: str,
    local_header_offset: int,
    compressed_size: int,
    expected_name: str,
    expected_crc32: int | None = None,
    expected_uncompressed_size: int | None = None,
) -> tuple[bytes, dict[str, object]]:
    if local_header_offset < 0 or compressed_size < 0 or compressed_size > 64 * 1024 * 1024:
        raise MemberError("invalid_bounded_member_range")
    start = local_header_offset
    end = local_header_offset + 30 + 65535 + compressed_size - 1
    request = urllib.request.Request(url, headers={"Range": f"bytes={start}-{end}"})
    with urllib.request.urlopen(request, timeout=120) as response:
        body = response.read()
        status = getattr(response, "status", None)
        content_range = response.headers.get("Content-Range", "")
    if status != 206:
        raise MemberError(f"range_not_honored_status_{status}")
    if not content_range.startswith(f"bytes {start}-"):
        raise MemberError(f"content_range_mismatch:{content_range}")
    if len(body) < 30 or body[:4] != b"PK\x03\x04":
        raise MemberError("local_header_missing")
    flags = _u16(body, 6)
    method = _u16(body, 8)
    local_crc = _u32(body, 14)
    local_compressed_size = _u32(body, 18)
    local_uncompressed_size = _u32(body, 22)
    name_length = _u16(body, 26)
    extra_length = _u16(body, 28)
    header_end = 30 + name_length + extra_length
    if header_end > len(body):
        raise MemberError("local_header_truncated")
    name = body[30 : 30 + name_length].decode("utf-8", errors="strict")
    if name != expected_name:
        raise MemberError(f"member_name_mismatch:{name}")
    if local_compressed_size == 0xFFFFFFFF or local_uncompressed_size == 0xFFFFFFFF:
        raise MemberError("zip64_local_member_not_supported_in_bounded_probe")
    if flags & 0x0008:
        member_compressed_size = compressed_size
        crc_expected = expected_crc32 if expected_crc32 is not None else local_crc
        member_uncompressed_size = expected_uncompressed_size if expected_uncompressed_size is not None else local_uncompressed_size
    else:
        if local_compressed_size != compressed_size:
            raise MemberError(f"compressed_size_mismatch:{local_compressed_size}:{compressed_size}")
        member_compressed_size = local_compressed_size
        crc_expected = expected_crc32 if expected_crc32 is not None else local_crc
        member_uncompressed_size = expected_uncompressed_size if expected_uncompressed_size is not None else local_uncompressed_size
    data_end = header_end + member_compressed_size
    if data_end > len(body):
        raise MemberError("member_data_truncated")
    compressed = body[header_end:data_end]
    if method == 0:
        content = compressed
    elif method == 8:
        try:
            content = zlib.decompress(compressed, -15)
        except zlib.error as exc:
            raise MemberError(f"deflate_error:{exc}") from exc
    else:
        raise MemberError(f"unsupported_compression_method_{method}")
    crc_actual = binascii.crc32(content) & 0xFFFFFFFF
    if crc_actual != crc_expected:
        raise MemberError(f"crc_mismatch:{crc_actual}:{crc_expected}")
    if member_uncompressed_size and len(content) != member_uncompressed_size:
        raise MemberError(f"uncompressed_size_mismatch:{len(content)}:{member_uncompressed_size}")
    return content, {
        "member_name": name,
        "range_start": start,
        "range_end_requested": end,
        "downloaded_bytes": len(body),
        "compression_method": method,
        "compressed_size": len(compressed),
        "uncompressed_size": len(content),
        "crc32": crc_actual,
        "sha256": hashlib.sha256(content).hexdigest(),
        "archive_downloaded": False,
        "member_downloaded": True,
        "executed": False,
    }


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--url", required=True)
    parser.add_argument("--offset", type=int, required=True)
    parser.add_argument("--compressed-size", type=int, required=True)
    parser.add_argument("--name", required=True)
    parser.add_argument("--crc32", type=lambda value: int(value, 0), default=None)
    parser.add_argument("--uncompressed-size", type=int, default=None)
    parser.add_argument("--out", type=Path, default=None)
    args = parser.parse_args(argv)
    try:
        content, result = fetch_member(
            args.url,
            args.offset,
            args.compressed_size,
            args.name,
            args.crc32,
            args.uncompressed_size,
        )
        if args.out is not None:
            args.out.write_bytes(content)
            result["output_path"] = str(args.out)
    except (OSError, MemberError, UnicodeError) as exc:
        print(json.dumps({"verdict": "blocked", "error": str(exc)}, ensure_ascii=False))
        return 2
    result["verdict"] = "member_verified"
    print(json.dumps(result, ensure_ascii=False, indent=2))
    return 0


if __name__ == "__main__":
    sys.exit(main())
