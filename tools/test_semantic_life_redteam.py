#!/usr/bin/env python3
"""GAN red-team for the continuous personal semantic-life simulator.

Every mutation removes one control from the real host composition. A mutant is
killed only when the semantic-life scenario exits non-zero. A source that does
not compile is a coverage failure, not a killed mutant.
"""
from __future__ import annotations

import os
import pathlib
import subprocess
import sys
import tempfile
from dataclasses import dataclass

ROOT = pathlib.Path(__file__).resolve().parents[1]
CC = os.environ.get("CC", "cc")
FLAGS = ["-O2", "-Wall", "-Wextra", "-std=c11", "-DHV_LUT_POPCOUNT"]
INCLUDES = [
    f"-I{ROOT / 'sim'}",
    f"-I{ROOT / 'firmware/core'}",
    f"-I{ROOT / 'firmware/net'}",
    f"-I{ROOT / 'firmware/port'}",
]
SIM_SOURCES = [
    "world.c", "channel.c", "render.c", "node.c", "scenarios.c",
    "stress.c", "compose.c", "study.c", "learn.c", "virtual_lab.c",
    "personal_scenario.c", "personal_sim.c", "semantic_scenario.c",
    "semantic_life.c", "physical_fault_scenario.c", "authority_transition.c", "authority_scenario.c", "authority_benchmark.c", "main.c",
]
CORE_SOURCES = [
    "hv.c", "sbc.c", "lexicon.c", "hcp.c", "text.c",
    "transport_selector.c", "personal_telemetry.c", "ambient_presence.c",
    "memory_semantic_evidence.c",
]
NET_SOURCES = ["crypto.c", "session.c", "region.c", "weave.c", "beat.c", "link.c"]


@dataclass(frozen=True)
class Mutation:
    name: str
    source: str
    find: str
    replace: str


MUTATIONS = (
    Mutation(
        "memory-physical-authority-bypass",
        "sim/semantic_life.c",
        "    if (event->explicit_memory_confirmation != 1u ||\n        event->presence.physical_contact != 1u) {\n",
        "    if (0) { /* REDTEAM: unconfirmed candidate becomes durable */\n",
    ),
    Mutation(
        "functional-conflict-bypass",
        "sim/semantic_life.c",
        "    return predicate == SL_PRED_GOAL || predicate == SL_PRED_CONTEXT;\n",
        "    return 0; /* REDTEAM: incompatible goals become alternatives */\n",
    ),
    Mutation(
        "reboot-semantic-scrub-bypass",
        "sim/semantic_life.c",
        "    life->physical.haptic_available = 0u;\n    mse_init(&life->semantic_index, sl_is_functional, NULL);\n",
        "    life->physical.haptic_available = 0u;\n    /* REDTEAM: prior semantic evidence survives reboot */\n",
    ),
    Mutation(
        "reboot-floor-import-bypass",
        "sim/semantic_life.c",
        "    (void)mse_set_generation_floor(&life->semantic_index, floor);\n",
        "    /* REDTEAM: recovered semantic floor is not installed */\n",
    ),
    Mutation(
        "divergent-floor-gate-bypass",
        "sim/semantic_life.c",
        "        if (event->recovered_semantic_floor < life->durable_semantic_floor)\n            return SL_E_FLOOR;\n",
        "        /* REDTEAM: lower floor silently replaces durable floor */\n",
    ),
    Mutation(
        "context-expiry-bypass",
        "sim/semantic_life.c",
        "    trace->expired_count = mse_expire(&life->semantic_index,\n                                      event->presence.generation);\n",
        "    trace->expired_count = 0u; /* REDTEAM: expiry becomes invisible */\n",
    ),
    Mutation(
        "reboot-quarantine-bypass",
        "sim/semantic_life.c",
        "    life->quarantined = 1u;\n",
        "    life->quarantined = 0u; /* REDTEAM: reboot exits quarantine */\n",
    ),
    Mutation(
        "conflict-disposition-bypass",
        "sim/semantic_life.c",
        "            trace->memory_disposition = SL_MEMORY_CONFLICTED;\n",
        "            trace->memory_disposition = SL_MEMORY_RETAINED; /* REDTEAM */\n",
    ),
)


def build_command(mutated_source: pathlib.Path, mutated_relative: str,
                   binary: pathlib.Path) -> list[str]:
    def source_path(directory: str, name: str) -> str:
        relative = f"{directory}/{name}"
        if relative == mutated_relative:
            return str(mutated_source)
        return str(ROOT / relative)

    return [
        CC, *FLAGS, *INCLUDES,
        *(source_path("sim", name) for name in SIM_SOURCES),
        *(source_path("firmware/core", name) for name in CORE_SOURCES),
        *(source_path("firmware/net", name) for name in NET_SOURCES),
        "-lm", "-o", str(binary),
    ]


def run_mutation(mutation: Mutation, directory: pathlib.Path) -> bool:
    original_path = ROOT / mutation.source
    original = original_path.read_text(encoding="utf-8")
    if original.count(mutation.find) != 1:
        print(f"FAIL REDTEAM {mutation.name}: control text is not unique")
        return False
    mutated = directory / pathlib.Path(mutation.source).name
    mutated.write_text(original.replace(mutation.find, mutation.replace, 1),
                       encoding="utf-8")
    binary = directory / "herus-semantic-life"
    build = subprocess.run(
        build_command(mutated, mutation.source, binary),
        cwd=ROOT, text=True, stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT, check=False,
    )
    if build.returncode != 0:
        print(f"FAIL REDTEAM {mutation.name}: mutant did not compile")
        print(build.stdout[-3000:])
        return False
    result = subprocess.run(
        [str(binary), "semantic-life"], cwd=ROOT, text=True,
        stdout=subprocess.PIPE, stderr=subprocess.STDOUT, check=False,
    )
    killed = result.returncode != 0
    print(f"{'PASS' if killed else 'FAIL'} REDTEAM {mutation.name}: "
          f"{'sabotage detected' if killed else 'sabotage survived'}")
    if not killed:
        print(result.stdout[-3000:])
    return killed


def main() -> int:
    print("== HERUS continuous semantic-life red-team campaign ==")
    with tempfile.TemporaryDirectory(prefix="herus-semantic-life-redteam-") as raw:
        directory = pathlib.Path(raw)
        results = [run_mutation(mutation, directory) for mutation in MUTATIONS]
    killed = sum(results)
    print(f"SEMANTIC-LIFE REDTEAM: {killed}/{len(results)} critical mutants killed")
    return 0 if killed == len(results) else 1


if __name__ == "__main__":
    sys.exit(main())
