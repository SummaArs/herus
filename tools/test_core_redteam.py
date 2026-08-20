#!/usr/bin/env python3
"""GAN red-team for the Core feed and its durable anti-rollback cursor.

Each mutant removes one deny-by-default control from the real C implementation,
compiles the existing behavioral suite, and requires that suite to fail. A
zero-exit mutant is a surviving attack and fails this campaign.
"""
from __future__ import annotations

import os
import pathlib
import shutil
import subprocess
import sys
import tempfile
from dataclasses import dataclass

ROOT = pathlib.Path(__file__).resolve().parents[1]
FIRMWARE = ROOT / "firmware"
CORE = FIRMWARE / "core"
CC = os.environ.get("CC", "cc")
FLAGS = ["-O2", "-Wall", "-Wextra", "-std=c11", "-DHV_LUT_POPCOUNT", "-Icore", "-Inet"]

FEED_SOURCES = [
    "net/crypto.c", "core/hv.c", "core/resonator.c",
    "core/resonator_bridge.c", "core/symbol_registry.c",
    "core/symbolic_reasoner.c", "core/symbolic_dialogue.c",
    "core/knowledge_feed.c", "core/test_knowledge_feed.c",
]
CURSOR_SOURCES = [
    "net/crypto.c", "core/knowledge_feed_cursor.c",
    "core/test_knowledge_feed_cursor.c",
]


@dataclass(frozen=True)
class Mutation:
    name: str
    source: str
    test_sources: tuple[str, ...]
    find: str
    replace: str


def run_mutation(directory: pathlib.Path, mutation: Mutation) -> bool:
    original_path = ROOT / mutation.source
    original = original_path.read_text(encoding="utf-8")
    if original.count(mutation.find) != 1:
        print(f"  FAIL REDTEAM {mutation.name}: control text not unique")
        return False

    mutated_path = directory / pathlib.Path(mutation.source).name
    mutated_path.write_text(original.replace(mutation.find, mutation.replace),
                            encoding="utf-8")
    binary = directory / f"{mutation.name}.bin"
    target_name = pathlib.Path(mutation.source).name
    sources = [str(mutated_path) if pathlib.Path(source).name == target_name else source
               for source in mutation.test_sources]
    build = subprocess.run([CC, *FLAGS, *sources, "-o", str(binary)],
                           cwd=FIRMWARE, text=True, capture_output=True,
                           check=False)
    if build.returncode != 0:
        print(f"  FAIL REDTEAM {mutation.name}: mutant did not compile")
        print(build.stderr)
        return False
    result = subprocess.run([str(binary)], cwd=FIRMWARE, text=True,
                            capture_output=True, check=False)
    if result.returncode == 0:
        print(f"  FAIL REDTEAM {mutation.name}: sabotage survived")
        return False
    print(f"  PASS REDTEAM {mutation.name}: sabotage was detected")
    return True


def main() -> int:
    mutations = (
        Mutation(
            "feed-digest-gate",
            "firmware/core/knowledge_feed.c",
            tuple(FEED_SOURCES),
            "    if (!ct_eq(packet->payload_digest, digest, KF_DIGEST_LEN))\n        return KF_REJECTED_DIGEST;\n",
            "    /* REDTEAM: payload digest gate removed. */\n",
        ),
        Mutation(
            "feed-namespace-gate",
            "firmware/core/knowledge_feed.c",
            tuple(FEED_SOURCES),
            "    if (!valid_source(packet->source_kind) || !valid_namespace(packet->namespace_id))\n        return KF_REJECTED_NAMESPACE;\n",
            "    /* REDTEAM: source and namespace gate removed. */\n",
        ),
        Mutation(
            "feed-rollback-gate",
            "firmware/core/knowledge_feed.c",
            tuple(FEED_SOURCES),
            "    if (cursor && cursor->initialized && packet->sequence <= cursor->last_sequence)\n        return KF_REJECTED_VERSION;\n",
            "    /* REDTEAM: feed anti-rollback gate removed. */\n",
        ),
        Mutation(
            "core-absence-gate",
            "firmware/core/knowledge_feed.c",
            tuple(FEED_SOURCES),
            "    return core_link_present == 1u ? KF_CORE_AVAILABLE : KF_CORE_UNAVAILABLE;\n",
            "    return KF_CORE_AVAILABLE; /* REDTEAM: absence becomes authority. */\n",
        ),
        Mutation(
            "cursor-hmac-gate",
            "firmware/core/knowledge_feed_cursor.c",
            tuple(CURSOR_SOURCES),
            "    if (!ct_eq(expected, record + 38u, KF_AUTH_TAG_LEN)) {\n        secure_zero(expected, sizeof(expected));\n        return KFC_E_AUTH;\n    }\n",
            "    /* REDTEAM: durable cursor HMAC gate removed. */\n",
        ),
        Mutation(
            "cursor-rollback-gate",
            "firmware/core/knowledge_feed_cursor.c",
            tuple(CURSOR_SOURCES),
            "    if (state->initialized && sequence <= state->cursor.last_sequence)\n        return KFC_E_ROLLBACK;\n",
            "    /* REDTEAM: commit anti-rollback gate removed. */\n",
        ),
        Mutation(
            "cursor-readback-gate",
            "firmware/core/knowledge_feed_cursor.c",
            tuple(CURSOR_SOURCES),
            "    if (decode_record(key, registry_version, readback,\n                      &recovered_sequence, recovered_digest) != KFC_OK ||\n        recovered_sequence != sequence ||\n        memcmp(recovered_digest, payload_digest, KF_DIGEST_LEN) != 0) {\n",
            "    if (0) { /* REDTEAM: authenticated readback gate removed. */\n",
        ),
    )

    print("\n== HERUS Core/feed deterministic red-team campaign ==")
    passed = True
    with tempfile.TemporaryDirectory(prefix="herus-core-redteam-") as raw:
        directory = pathlib.Path(raw)
        for mutation in mutations:
            passed = run_mutation(directory, mutation) and passed
    if not passed:
        print("CORE REDTEAM FAILED — at least one critical Core mutant survived")
        return 1
    print("CORE REDTEAM: 7/7 critical Core mutants killed")
    return 0


if __name__ == "__main__":
    sys.exit(main())
