#!/usr/bin/env python3
"""Deterministic mutation gate for the HERUS pre-hardware virtual lab.

Each mutation is a deliberately unsafe one-line change in a real host contract.
The script recompiles the same simulator with that source substituted and expects
`herus-sim virtual` to fail. A surviving mutant is a false-positive alarm: either
the virtual oracle is weak or the contract lacks a necessary invariant.

This is not fuzzing and does not claim hardware coverage. Inputs, mutations and
build commands are fixed so a failure is reproducible byte-for-byte.
"""
from __future__ import annotations

import argparse
import pathlib
import subprocess
import sys
import tempfile

ROOT = pathlib.Path(__file__).resolve().parents[1]

SIM_SOURCES = [
    "world.c", "channel.c", "render.c", "node.c", "scenarios.c",
    "stress.c", "compose.c", "study.c", "learn.c", "virtual_lab.c",
    "personal_scenario.c", "personal_sim.c", "main.c",
]
CORE_SOURCES = [
    "hv.c", "sbc.c", "lexicon.c", "hcp.c", "text.c",
    "transport_selector.c", "personal_telemetry.c", "ambient_presence.c",
]
NET_SOURCES = ["crypto.c", "session.c", "region.c", "weave.c", "beat.c", "link.c"]

MUTATIONS = [
    {
        "name": "transport-authority-bypass",
        "source": "firmware/core/transport_selector.c",
        "old": "if (input->physical_authorized != 1u) {",
        "new": "if (0) {",
        "why": "a route must never survive physical revocation",
    },
    {
        "name": "transport-privacy-bypass",
        "source": "firmware/core/transport_selector.c",
        "old": "if (input->payload_class == TRANSPORT_PAYLOAD_AUDIO ||\n        input->payload_class == TRANSPORT_PAYLOAD_LOCATION) {",
        "new": "if (0) {",
        "why": "audio and location must not become shareable payloads",
    },
    {
        "name": "telemetry-share-bypass",
        "source": "firmware/core/transport_selector.c",
        "old": "if (input->payload_class == TRANSPORT_PAYLOAD_PERSONAL_TELEMETRY &&\n        input->share_confirmed != 1u) {",
        "new": "if (0) {",
        "why": "local retention must not imply network sharing",
    },
    {
        "name": "telemetry-quality-bypass",
        "source": "firmware/core/personal_telemetry.c",
        "old": "if (s->quality != TELEMETRY_QUALITY_USABLE) {",
        "new": "if (0) {",
        "why": "low-quality observations must abstain rather than become memory",
    },
    {
        "name": "telemetry-session-bypass",
        "source": "firmware/core/personal_telemetry.c",
        "old": "if (s->capture_session_id == 0u ||\n        s->capture_session_id != t->capture_session_id ||\n        s->capture_authorized != 1u) {",
        "new": "if (0) {",
        "why": "a sample from another session must never enter the candidate",
    },
    {
        "name": "telemetry-confirmation-bypass",
        "source": "firmware/core/personal_telemetry.c",
        "old": "t->pending.persist_authorized = 1u;",
        "new": "t->pending.persist_authorized = 0u;",
        "why": "physical confirmation must be the only persistence authority",
    },
    {
        "name": "telemetry-wraparound-regression",
        "source": "firmware/core/personal_telemetry.c",
        "old": "return (int32_t)(s->now_ms - s->window_end_ms) >= 0;",
        "new": "return s->window_end_ms <= s->now_ms;",
        "why": "short valid windows may cross the uint32 clock wrap",
    },
]


def mutate_source(spec: dict[str, str], destination: pathlib.Path) -> None:
    original = (ROOT / spec["source"]).read_text(encoding="utf-8")
    count = original.count(spec["old"])
    if count != 1:
        raise RuntimeError(
            f"mutation {spec['name']} expected one match, found {count}"
        )
    destination.write_text(original.replace(spec["old"], spec["new"], 1), encoding="utf-8")


def build_command(mutated_source: pathlib.Path, mutated_relative: str, binary: pathlib.Path) -> list[str]:
    core = []
    for name in CORE_SOURCES:
        relative = f"firmware/core/{name}"
        core.append(str(mutated_source if relative == mutated_relative else ROOT / relative))
    return [
        "cc", "-O2", "-Wall", "-Wextra", "-std=c11", "-DHV_LUT_POPCOUNT",
        f"-I{ROOT / 'firmware/core'}",
        f"-I{ROOT / 'firmware/net'}",
        f"-I{ROOT / 'firmware/port'}",
        *(str(ROOT / "sim" / name) for name in SIM_SOURCES),
        *core,
        *(str(ROOT / "firmware/net" / name) for name in NET_SOURCES),
        "-lm", "-o", str(binary),
    ]


def run_one(spec: dict[str, str], work: pathlib.Path, verbose: bool) -> bool:
    work.mkdir(parents=True, exist_ok=True)
    mutated_relative = spec["source"]
    mutated_source = work / pathlib.Path(mutated_relative).name
    binary = work / "herus-sim"
    mutate_source(spec, mutated_source)
    compile_result = subprocess.run(
        build_command(mutated_source, mutated_relative, binary),
        cwd=ROOT,
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        check=False,
    )
    if compile_result.returncode != 0:
        print(f"FAIL {spec['name']}: mutant does not compile")
        print(compile_result.stdout[-4000:])
        return False

    result = subprocess.run(
        [str(binary), "virtual"],
        cwd=ROOT,
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        check=False,
    )
    killed = result.returncode != 0
    print(f"{'PASS' if killed else 'FAIL'} {spec['name']}: {spec['why']}")
    if verbose or not killed:
        print(result.stdout[-4000:])
    return killed


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--verbose", action="store_true", help="print each mutant output")
    parser.add_argument("--only", help="run one mutation by name")
    args = parser.parse_args()

    selected = [m for m in MUTATIONS if not args.only or m["name"] == args.only]
    if not selected:
        print(f"unknown mutation: {args.only}", file=sys.stderr)
        return 2

    with tempfile.TemporaryDirectory(prefix="herus-mutation-") as directory:
        work = pathlib.Path(directory)
        results = [run_one(spec, work / spec["name"], args.verbose)
                   for spec in selected]

    passed = sum(results)
    print(f"MUTATION GATE: {passed}/{len(results)} unsafe mutants killed")
    return 0 if all(results) else 1


if __name__ == "__main__":
    raise SystemExit(main())
