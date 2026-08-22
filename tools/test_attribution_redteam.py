"""Redteam for typed attribution, implicit authority and reintroduction barriers."""
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
    "attribution_guard.c", "attribution_scenario.c", "main.c",
]
CORE_SOURCES = [
    "hv.c", "sbc.c", "lexicon.c", "hcp.c", "text.c",
    "transport_selector.c", "personal_telemetry.c", "ambient_presence.c",
    "memory_semantic_evidence.c",
]
NET_SOURCES = ["crypto.c", "session.c", "region.c", "weave.c", "beat.c", "link.c"]

MUTATIONS = [
    {
        "name": "role-laundering",
        "old": "    if (!valid_derived_role(parent, edge, role)) return AG_E_ROLE;",
        "new": "    if (0) /* REDTEAM */ return AG_E_ROLE;",
        "why": "an observation-derived record must not become policy or action context",
    },
    {
        "name": "source-laundering",
        "old": "    node->source = parent->source;\n    node->role = role;",
        "new": "    node->source = AT_SOURCE_CORE_KNOWLEDGE; /* REDTEAM */\n    node->role = role;",
        "why": "derived lineage must preserve the original source principal",
    },
    {
        "name": "authority-amplification",
        "old": "    if ((authority & ~parent->authority) != 0u ||\n        (scope & ~parent->scope) != 0u)",
        "new": "    if ((0) || /* REDTEAM */\n        (scope & ~parent->scope) != 0u)",
        "why": "derivation must not add memory or action authority",
    },
    {
        "name": "scope-amplification",
        "old": "    if ((authority & ~parent->authority) != 0u ||\n        (scope & ~parent->scope) != 0u)",
        "new": "    if ((authority & ~parent->authority) != 0u ||\n        (0)) /* REDTEAM */",
        "why": "derivation must not add an ungranted local scope",
    },
    {
        "name": "purpose-bypass",
        "old": "    if (purpose_token == 0u || purpose_token != expected_purpose_token)\n        return AG_E_PURPOSE;",
        "new": "    if (0) /* REDTEAM */\n        return AG_E_PURPOSE;",
        "why": "semantic relevance cannot replace purpose-bound admission",
    },
    {
        "name": "quarantine-bypass",
        "old": "    if (node->status == AG_NODE_QUARANTINED) return AG_E_EPOCH;",
        "new": "    if (0) /* REDTEAM */ return AG_E_EPOCH;",
        "why": "post-reboot quarantined records must not be admitted",
    },
    {
        "name": "transitive-revocation-bypass",
        "old": "            if (node->status != AG_NODE_REVOKED &&\n                (node->node_id == node_id || is_revoked(index, node->parent_id))) {",
        "new": "            if (node->status != AG_NODE_REVOKED &&\n                (node->node_id == node_id || 0)) { /* REDTEAM */",
        "why": "revoking a root must revoke every derived descendant",
    },
    {
        "name": "confirmation-forwarding-bypass",
        "old": "    if (physical_confirmation != 1u) return AG_E_AUTH;\n    if (!valid_time(generation, offer->generation,",
        "new": "    physical_confirmation = 1u; /* REDTEAM */\n    if (!valid_time(generation, offer->generation,",
        "why": "the attribution layer must not manufacture physical confirmation",
    },
]


def build_command(mutated: pathlib.Path, binary: pathlib.Path) -> list[str]:
    return [
        "cc", "-O2", "-Wall", "-Wextra", "-std=c11", "-DHV_LUT_POPCOUNT",
        f"-I{ROOT / 'sim'}", f"-I{ROOT / 'firmware/core'}",
        f"-I{ROOT / 'firmware/net'}", f"-I{ROOT / 'firmware/port'}",
        *(str(mutated) if name == "attribution_guard.c" else str(ROOT / "sim" / name)
          for name in SIM_SOURCES),
        *(str(ROOT / "firmware/core" / name) for name in CORE_SOURCES),
        *(str(ROOT / "firmware/net" / name) for name in NET_SOURCES),
        "-lm", "-o", str(binary),
    ]


def main() -> int:
    source_path = ROOT / "sim" / "attribution_guard.c"
    original = source_path.read_text(encoding="utf-8")
    killed = 0
    with tempfile.TemporaryDirectory(prefix="herus-attribution-redteam-") as raw:
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
                [str(binary), "attribution"], cwd=ROOT, text=True,
                stdout=subprocess.PIPE, stderr=subprocess.STDOUT, check=False,
            )
            if result.returncode != 0:
                killed += 1
                print(f"PASS {spec['name']}: {spec['why']}")
            else:
                print(f"FAIL {spec['name']}: surviving mutant")
                print(result.stdout[-2500:])
    print(f"ATTRIBUTION REDTEAM: {killed}/{len(MUTATIONS)} critical mutants killed")
    return 0 if killed == len(MUTATIONS) else 1


if __name__ == "__main__":
    raise SystemExit(main())
