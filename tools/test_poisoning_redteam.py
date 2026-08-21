#!/usr/bin/env python3
"""Redteam for direct, compositional and dormant memory poisoning."""
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
    "authority_scenario.c", "authority_benchmark.c", "adaptive_change.c",
    "adaptive_change_scenario.c", "poisoning_guard.c", "poisoning_scenario.c",
    "main.c",
]
CORE_SOURCES = [
    "hv.c", "sbc.c", "lexicon.c", "hcp.c", "text.c",
    "transport_selector.c", "personal_telemetry.c", "ambient_presence.c",
    "memory_semantic_evidence.c",
]
NET_SOURCES = ["crypto.c", "session.c", "region.c", "weave.c", "beat.c", "link.c"]

MUTATIONS = [
    {
        "name": "bundle-epoch-bypass",
        "old": "    if (current_epoch != bundle->epoch || memory->epoch != bundle->epoch)\n        return PG_E_EPOCH;",
        "new": "    if (0) /* REDTEAM */\n        return PG_E_EPOCH;",
        "why": "replayed memory must not re-enter an old bundle",
    },
    {
        "name": "context-epoch-bypass",
        "old": "    if (bundle->epoch == 0u || current_epoch != bundle->epoch ||\n        generation < bundle->generation ||",
        "new": "    if (bundle->epoch == 0u || 0 ||\n        generation < bundle->generation ||",
        "why": "dormant bundles must not cross session epochs",
    },
    {
        "name": "context-token-bypass",
        "old": "    if (context_token == 0u || context_token != expected_context_token)\n        return PG_E_CONTEXT;",
        "new": "    if (0) /* REDTEAM */\n        return PG_E_CONTEXT;",
        "why": "unrelated context must not activate dormant memory",
    },
    {
        "name": "context-expiry-bypass",
        "old": "        (bundle->valid_until_generation != 0u &&\n         generation > bundle->valid_until_generation))",
        "new": "        (0 && bundle->valid_until_generation != 0u &&\n         generation > bundle->valid_until_generation))",
        "why": "expired dormant state must not activate",
    },
    {
        "name": "context-conflict-bypass",
        "old": "    if (bundle->conflict) return PG_E_CONFLICT;",
        "new": "    if (0) /* REDTEAM */ return PG_E_CONFLICT;",
        "why": "compositional conflict must abstain",
    },
    {
        "name": "authority-union-bypass",
        "old": "            : bundle->authority_intersection & memory->authority;",
        "new": "            : bundle->authority_intersection | memory->authority; /* REDTEAM */",
        "why": "composition must not add authority",
    },
    {
        "name": "offer-action-bypass",
        "old": "    out_offer->authority = bundle->authority_intersection;",
        "new": "    out_offer->authority = bundle->authority_intersection | AT_AUTH_ACTION; /* REDTEAM */",
        "why": "context offer must not become action",
    },
    {
        "name": "offer-confirmation-bypass",
        "old": "    out_offer->physically_confirmed = 0u;",
        "new": "    out_offer->physically_confirmed = 1u; /* REDTEAM */",
        "why": "dormant trigger must not manufacture physical confirmation",
    },
]


def build_command(mutated: pathlib.Path, binary: pathlib.Path) -> list[str]:
    return [
        "cc", "-O2", "-Wall", "-Wextra", "-std=c11", "-DHV_LUT_POPCOUNT",
        f"-I{ROOT / 'sim'}", f"-I{ROOT / 'firmware/core'}",
        f"-I{ROOT / 'firmware/net'}", f"-I{ROOT / 'firmware/port'}",
        *(str(mutated) if name == "poisoning_guard.c" else str(ROOT / "sim" / name)
          for name in SIM_SOURCES),
        *(str(ROOT / "firmware/core" / name) for name in CORE_SOURCES),
        *(str(ROOT / "firmware/net" / name) for name in NET_SOURCES),
        "-lm", "-o", str(binary),
    ]


def main() -> int:
    source_path = ROOT / "sim" / "poisoning_guard.c"
    original = source_path.read_text(encoding="utf-8")
    killed = 0
    with tempfile.TemporaryDirectory(prefix="herus-poisoning-redteam-") as raw:
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
                [str(binary), "poisoning"], cwd=ROOT, text=True,
                stdout=subprocess.PIPE, stderr=subprocess.STDOUT, check=False,
            )
            if result.returncode != 0:
                killed += 1
                print(f"PASS {spec['name']}: {spec['why']}")
            else:
                print(f"FAIL {spec['name']}: surviving mutant")
                print(result.stdout[-2500:])
    print(f"POISONING REDTEAM: {killed}/{len(MUTATIONS)} critical mutants killed")
    return 0 if killed == len(MUTATIONS) else 1


if __name__ == "__main__":
    raise SystemExit(main())
