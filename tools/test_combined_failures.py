#!/usr/bin/env python3
"""Run the composed Core, reboot, consent and bounded-memory failure matrix."""
from __future__ import annotations

import os
import pathlib
import subprocess
import sys
import tempfile

ROOT = pathlib.Path(__file__).resolve().parents[1]
FIRMWARE = ROOT / "firmware"
CC = os.environ.get("CC", "cc")
FLAGS = ["-O2", "-Wall", "-Wextra", "-std=c11", "-DHV_LUT_POPCOUNT", "-Icore", "-Inet"]
SOURCES = [
    "net/crypto.c", "core/hv.c", "core/resonator.c",
    "core/resonator_bridge.c", "core/symbol_registry.c",
    "core/symbolic_reasoner.c", "core/symbolic_dialogue.c",
    "core/knowledge_feed.c", "core/knowledge_feed_cursor.c",
    "core/memory_semantic_evidence.c", "core/memory_reasoning_bridge.c",
    "core/magic_anticipation.c", "core/magic_trigger.c",
    "core/test_core_resilience_matrix.c",
]


def main() -> int:
    with tempfile.TemporaryDirectory(prefix="herus-combined-failures-") as raw:
        binary = pathlib.Path(raw) / "core-resilience-matrix"
        build = subprocess.run([CC, *FLAGS, *SOURCES, "-o", str(binary)],
                               cwd=FIRMWARE, text=True, capture_output=True,
                               check=False)
        if build.returncode != 0:
            print("COMBINED FAILURES FAILED — matrix did not compile")
            print(build.stderr)
            return 1
        result = subprocess.run([str(binary)], cwd=FIRMWARE, text=True,
                                capture_output=True, check=False)
        print(result.stdout, end="")
        if result.returncode != 0 or "FAIL" in result.stdout:
            print("COMBINED FAILURES FAILED — a composed failure escaped")
            return 1
    print("COMBINED FAILURES: Core absence, reboot corruption, consent revocation, authority and exhaustion all fail closed")
    return 0


if __name__ == "__main__":
    sys.exit(main())
