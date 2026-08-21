#!/usr/bin/env python3
"""GAN redteam for Authority-Governed Semantic Continuity.

Every mutant removes one authority or provenance gate from the real host
contract. The full simulator is rebuilt and the `authority` scenario must fail.
A compile failure is a harness failure, not a killed mutant.
"""
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
        "name": "promotion-without-contact",
        "old": "/* A source is never allowed to promote itself. Physical confirmation is\n     * separate evidence, and the source remains attached to the capsule. */\n    if (physical_confirmation != 1u)\n        return fail(machine, AT_E_AUTH);",
        "new": "/* A source is never allowed to promote itself. Physical confirmation is\n     * separate evidence, and the source remains attached to the capsule. */\n    if (0)\n        return fail(machine, AT_E_AUTH);",
        "why": "memory promotion must require physical confirmation",
        "occurrence": 1,
    },
    {
        "name": "conflict-promotion",
        "old": "if (candidate->conflict) return fail(machine, AT_E_CONFLICT);",
        "new": "if (0) return fail(machine, AT_E_CONFLICT);",
        "why": "conflict must not become personal memory",
        "occurrence": 1,
    },
    {
        "name": "expired-promotion",
        "old": "if (generation < candidate->generation ||\n        (candidate->valid_until_generation != 0u &&\n         generation > candidate->valid_until_generation))",
        "new": "if (0)",
        "why": "expired candidates must not become current memory",
        "occurrence": 1,
    },
    {
        "name": "retrieval-adds-action-authority",
        "old": "out->authority = memory->authority;",
        "new": "out->authority = memory->authority | AT_AUTH_ACTION;",
        "why": "retrieval must not add action authority",
        "occurrence": 1,
    },
    {
        "name": "offer-skips-physical-grant",
        "old": "if (physical_confirmation != 1u)\n        return fail(machine, AT_E_AUTH);\n    memset(out, 0, sizeof(*out));\n    *out = *offer;",
        "new": "if (0)\n        return fail(machine, AT_E_AUTH);\n    memset(out, 0, sizeof(*out));\n    *out = *offer;",
        "why": "local action grant must require a physical confirmation",
        "occurrence": 1,
    },
    {
        "name": "core-scope-allowed",
        "old": "const uint32_t allowed = AT_SCOPE_LOCAL_HAPTIC |\n                             AT_SCOPE_LOCAL_DIALOGUE |\n                             AT_SCOPE_LOCAL_RADIO;",
        "new": "const uint32_t allowed = AT_SCOPE_LOCAL_HAPTIC |\n                             AT_SCOPE_LOCAL_DIALOGUE |\n                             AT_SCOPE_LOCAL_RADIO | AT_SCOPE_CORE_EXECUTE;",
        "why": "Core execution scope must never be grantable",
        "occurrence": 1,
    },
    {
        "name": "pre-reboot-offer-revival",
        "old": "offer->epoch != machine->epoch)",
        "new": "0)",
        "why": "pre-reboot offers must not regain authority",
        "occurrence": 1,
    },
    {
        "name": "conflict-action",
        "old": "if (offer->conflict) return fail(machine, AT_E_CONFLICT);",
        "new": "if (0) return fail(machine, AT_E_CONFLICT);",
        "why": "conflicting offers must not reach local action",
        "occurrence": 1,
    },
]


def mutate(source: str, spec: dict[str, object]) -> str:
    old = str(spec["old"])
    new = str(spec["new"])
    expected = int(spec.get("occurrence", 1))
    count = source.count(old)
    if count != expected:
        raise RuntimeError(
            f"{spec['name']} expected {expected} occurrence(s), found {count}"
        )
    return source.replace(old, new, expected)


def build_command(mutated: pathlib.Path, binary: pathlib.Path) -> list[str]:
    return [
        "cc", "-O2", "-Wall", "-Wextra", "-std=c11", "-DHV_LUT_POPCOUNT",
        f"-I{ROOT / 'firmware/core'}", f"-I{ROOT / 'firmware/net'}",
        f"-I{ROOT / 'firmware/port'}", f"-I{ROOT / 'sim'}",
        *(str(ROOT / "sim" / name) if name != "authority_transition.c"
          else str(mutated) for name in SIM_SOURCES),
        *(str(ROOT / "firmware/core" / name) for name in CORE_SOURCES),
        *(str(ROOT / "firmware/net" / name) for name in NET_SOURCES),
        "-lm", "-o", str(binary),
    ]


def main() -> int:
    source_path = ROOT / "sim" / "authority_transition.c"
    original = source_path.read_text(encoding="utf-8")
    killed = 0
    with tempfile.TemporaryDirectory(prefix="herus-authority-redteam-") as directory:
        work = pathlib.Path(directory)
        for spec in MUTATIONS:
            mutated = work / f"{spec['name']}.c"
            binary = work / f"{spec['name']}.bin"
            try:
                mutated.write_text(mutate(original, spec), encoding="utf-8")
            except RuntimeError as error:
                print(f"FAIL {spec['name']}: {error}")
                continue
            compiled = subprocess.run(
                build_command(mutated, binary), cwd=ROOT,
                text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                check=False,
            )
            if compiled.returncode != 0:
                print(f"FAIL {spec['name']}: mutant does not compile")
                print(compiled.stdout[-2500:])
                continue
            result = subprocess.run(
                [str(binary), "authority"], cwd=ROOT,
                text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                check=False,
            )
            if result.returncode != 0:
                killed += 1
                print(f"PASS {spec['name']}: {spec['why']}")
            else:
                print(f"FAIL {spec['name']}: surviving mutant")
                print(result.stdout[-2500:])
    print(f"AUTHORITY REDTEAM: {killed}/{len(MUTATIONS)} critical mutants killed")
    return 0 if killed == len(MUTATIONS) else 1


if __name__ == "__main__":
    raise SystemExit(main())
