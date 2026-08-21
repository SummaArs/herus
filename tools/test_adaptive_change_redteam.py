#!/usr/bin/env python3
"""GAN redteam for AGSC-D adaptive semantic continuity."""
from __future__ import annotations

import pathlib
import subprocess
import tempfile

ROOT = pathlib.Path(__file__).resolve().parents[1]
SIM_SOURCES = [
    "world.c", "channel.c", "render.c", "node.c", "scenarios.c",
    "stress.c", "compose.c", "study.c", "learn.c", "virtual_lab.c",
    "personal_scenario.c", "personal_sim.c", "semantic_scenario.c",
    "semantic_life.c", "physical_fault_scenario.c", "authority_transition.c",
    "authority_scenario.c", "authority_benchmark.c", "adaptive_change.c", "adaptive_change_scenario.c", "poisoning_guard.c", "poisoning_scenario.c", "attribution_guard.c", "attribution_scenario.c", "main.c",
]
CORE_SOURCES = [
    "hv.c", "sbc.c", "lexicon.c", "hcp.c", "text.c",
    "transport_selector.c", "personal_telemetry.c", "ambient_presence.c",
    "memory_semantic_evidence.c",
]
NET_SOURCES = ["crypto.c", "session.c", "region.c", "weave.c", "beat.c", "link.c"]

MUTATIONS = [
    {
        "name": "confidence-gate-bypass",
        "old": "if (observation->confidence < AC_MIN_CONFIDENCE) {\n        index->rejected++;\n        return AC_E_CONFIDENCE;\n    }",
        "new": "if (0) { /* REDTEAM */\n        index->rejected++;\n        return AC_E_CONFIDENCE;\n    }",
        "why": "low-confidence changes must not rewrite identity",
    },
    {
        "name": "change-confirmation-bypass",
        "old": "        observation->physical_confirmation != 1u ||\n        observation->explicit_change_confirmation != 1u ||",
        "new": "        observation->physical_confirmation != 1u ||\n        0 ||",
        "why": "explicit confirmation must gate semantic change",
    },
    {
        "name": "supersession-bypass",
        "old": "            entry->status = AC_ENTRY_SUPERSEDED;\n            entry->superseded_by_card_id = observation->card_id;",
        "new": "            entry->status = AC_ENTRY_ACTIVE; /* REDTEAM */\n            entry->superseded_by_card_id = observation->card_id;",
        "why": "new preference must remove old preference from current identity",
    },
    {
        "name": "derived-revocation-bypass",
        "old": "            if (entry->status != AC_ENTRY_REVOKED &&\n                (entry->card_id == card_id ||\n                 is_revoked(index, entry->derived_from_card_id))) {",
        "new": "            if (entry->status != AC_ENTRY_REVOKED &&\n                entry->card_id == card_id) { /* REDTEAM */",
        "why": "revocation must reach derived memories transitively",
    },
    {
        "name": "expiry-bypass",
        "old": "        if (entry->status == AC_ENTRY_ACTIVE &&\n            entry->valid_until_generation != 0u &&",
        "new": "        if (0 && entry->status == AC_ENTRY_ACTIVE &&\n            entry->valid_until_generation != 0u &&",
        "why": "expired preferences must not remain current",
    },
    {
        "name": "epoch-bypass",
        "old": "        observation->epoch != index->epoch ||",
        "new": "        0 ||",
        "why": "pre-reboot observations must not replay into a new epoch",
    },
    {
        "name": "revoked-derivation-bypass",
        "old": "    if (observation->derived_from_card_id != 0u &&\n        is_revoked(index, observation->derived_from_card_id))\n        return 0;",
        "new": "    if (0) /* REDTEAM */\n        return 0;",
        "why": "revoked lineage must not re-enter through a derived observation",
    },
    {
        "name": "physical-revocation-bypass",
        "old": "    if (!index || card_id == 0u || physical_confirmation != 1u)\n        return AC_E_AUTH;",
        "new": "    if (!index || card_id == 0u || 0) /* REDTEAM */\n        return AC_E_AUTH;",
        "why": "revocation must require physical confirmation",
    },
]


def build_command(mutated: pathlib.Path, binary: pathlib.Path) -> list[str]:
    return [
        "cc", "-O2", "-Wall", "-Wextra", "-std=c11", "-DHV_LUT_POPCOUNT",
        f"-I{ROOT / 'sim'}", f"-I{ROOT / 'firmware/core'}",
        f"-I{ROOT / 'firmware/net'}", f"-I{ROOT / 'firmware/port'}",
        *(str(mutated) if name == "adaptive_change.c" else str(ROOT / "sim" / name)
          for name in SIM_SOURCES),
        *(str(ROOT / "firmware/core" / name) for name in CORE_SOURCES),
        *(str(ROOT / "firmware/net" / name) for name in NET_SOURCES),
        "-lm", "-o", str(binary),
    ]


def main() -> int:
    source_path = ROOT / "sim" / "adaptive_change.c"
    original = source_path.read_text(encoding="utf-8")
    killed = 0
    with tempfile.TemporaryDirectory(prefix="herus-adaptive-change-redteam-") as raw:
        work = pathlib.Path(raw)
        for spec in MUTATIONS:
            if original.count(spec["old"]) != 1:
                print(f"FAIL {spec['name']}: control text is not unique")
                continue
            mutated = work / f"{spec['name']}.c"
            binary = work / f"{spec['name']}.bin"
            mutated.write_text(original.replace(spec["old"], spec["new"], 1),
                               encoding="utf-8")
            build = subprocess.run(
                build_command(mutated, binary), cwd=ROOT, text=True,
                stdout=subprocess.PIPE, stderr=subprocess.STDOUT, check=False,
            )
            if build.returncode != 0:
                print(f"FAIL {spec['name']}: mutant does not compile")
                print(build.stdout[-2500:])
                continue
            result = subprocess.run(
                [str(binary), "adaptive-change"], cwd=ROOT, text=True,
                stdout=subprocess.PIPE, stderr=subprocess.STDOUT, check=False,
            )
            if result.returncode != 0:
                killed += 1
                print(f"PASS {spec['name']}: {spec['why']}")
            else:
                print(f"FAIL {spec['name']}: surviving mutant")
                print(result.stdout[-2500:])
    print(f"ADAPTIVE-CHANGE REDTEAM: {killed}/{len(MUTATIONS)} critical mutants killed")
    return 0 if killed == len(MUTATIONS) else 1


if __name__ == "__main__":
    raise SystemExit(main())
