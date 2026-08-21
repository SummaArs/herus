"""GAN red-team for HERUS ambient presence.

Each mutant removes one control that makes the technology quiet, bounded or
non-authoritative. The existing property suite must fail for every mutant.
"""
from __future__ import annotations

import os
import pathlib
import subprocess
import sys
import tempfile
from dataclasses import dataclass

ROOT = pathlib.Path(__file__).resolve().parents[1]
FIRMWARE = ROOT / "firmware"
CC = os.environ.get("CC", "cc")
FLAGS = ["-O2", "-Wall", "-Wextra", "-std=c11", "-Icore"]
SOURCES = (
    "core/ambient_presence.c",
    "core/test_ambient_presence.c",
)


@dataclass(frozen=True)
class Mutation:
    name: str
    find: str
    replace: str


def run_mutation(directory: pathlib.Path, mutation: Mutation) -> bool:
    original_path = ROOT / "firmware/core/ambient_presence.c"
    original = original_path.read_text(encoding="utf-8")
    if original.count(mutation.find) != 1:
        print(f"  FAIL REDTEAM {mutation.name}: control text not unique")
        return False
    mutated_path = directory / "ambient_presence.c"
    mutated_path.write_text(original.replace(mutation.find, mutation.replace),
                            encoding="utf-8")
    binary = directory / f"{mutation.name}.bin"
    build = subprocess.run(
        [CC, *FLAGS, str(mutated_path), "core/test_ambient_presence.c",
         "-o", str(binary)],
        cwd=FIRMWARE, text=True, capture_output=True, check=False,
    )
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
            "remove-attention-gate",
            "    if (observation->attention_available != 1u)\n        reason |= AP_REASON_NO_ATTENTION;\n",
            "    /* REDTEAM: attention gate removed. */\n",
        ),
        Mutation(
            "remove-consent-gate",
            "    if (observation->proactive_consent != 1u)\n        reason |= AP_REASON_NO_CONSENT;\n",
            "    /* REDTEAM: consent gate removed. */\n",
        ),
        Mutation(
            "remove-single-offer-budget",
            "    if (presence->offered || presence->offer_budget == 0u) {\n",
            "    if (0) { /* REDTEAM: repeated offers allowed. */\n",
        ),
        Mutation(
            "remove-expiry-gate",
            "    if (presence->candidate_valid &&\n        now_generation > presence->expires_generation) {\n",
            "    if (0) { /* REDTEAM: expired candidates retained. */\n",
        ),
        Mutation(
            "remove-sensitive-gate",
            "    if (observation->privacy_class == AP_PRIVACY_SENSITIVE)\n        return reject_observation(presence, AP_REASON_SENSITIVE, AP_ABSTAIN);\n",
            "    /* REDTEAM: sensitive context admitted. */\n",
        ),
        Mutation(
            "remove-contact-gate",
            "    if (physical_contact != 1u) {\n",
            "    if (0) { /* REDTEAM: acknowledgement without contact. */\n",
        ),
    )

    print("\n== HERUS ambient presence red-team campaign ==")
    passed = True
    with tempfile.TemporaryDirectory(prefix="herus-ambient-redteam-") as raw:
        directory = pathlib.Path(raw)
        for mutation in mutations:
            passed = run_mutation(directory, mutation) and passed
    if not passed:
        print("AMBIENT PRESENCE REDTEAM FAILED — an intrusion or authority mutant survived")
        return 1
    print("AMBIENT PRESENCE REDTEAM: 6/6 critical invisibility mutants killed")
    return 0


if __name__ == "__main__":
    sys.exit(main())
