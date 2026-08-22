"""Adversarial campaign for counterfactual attribution composition and sharing."""
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
        "name": "composition-union-authority",
        "old": "    authority = left->authority & right->authority;",
        "new": "    authority = left->authority | right->authority; /* REDTEAM */",
        "why": "composition must intersect authority instead of adding capabilities",
    },
    {
        "name": "composition-union-scope",
        "old": "    scope = left->scope & right->scope;",
        "new": "    scope = left->scope | right->scope; /* REDTEAM */",
        "why": "composition must not widen the usable scope",
    },
    {
        "name": "composition-single-parent",
        "old": "    node->secondary_parent_id = right->node_id;",
        "new": "    node->secondary_parent_id = 0u; /* REDTEAM */",
        "why": "a composite claim must retain both causal supports",
    },
    {
        "name": "composition-source-laundering",
        "old": "    node->source = source_for_mask(source_mask);",
        "new": "    node->source = AT_SOURCE_LOCAL_OBSERVATION; /* REDTEAM */",
        "why": "mixed local/Core provenance must not be relabeled as local",
    },
    {
        "name": "recomposition-revocation-bypass",
        "old": "    if (left->status == AG_NODE_REVOKED || right->status == AG_NODE_REVOKED)\n        return AG_E_REVOKED;",
        "new": "    if (0) /* REDTEAM */\n        return AG_E_REVOKED;",
        "why": "removing a causal support must block recomposition",
    },
    {
        "name": "share-recipient-bypass",
        "old": "        share->recipient_principal_id != index->principal_id ||",
        "new": "        0 || /* REDTEAM */",
        "why": "a share addressed to another principal must not be imported",
    },
    {
        "name": "share-source-bypass",
        "old": "        source_mask_for(share->source) != share->source_mask ||",
        "new": "        0 || /* REDTEAM */",
        "why": "the source label and source mask must remain bound",
    },
    {
        "name": "share-confirmation-bypass",
        "old": "    if (!index || !share || physical_confirmation != 1u)\n        return AG_E_SHARE;",
        "new": "    if (!index || !share || 0) /* REDTEAM */\n        return AG_E_SHARE;",
        "why": "import must require fresh local confirmation",
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
    original_path = ROOT / "sim" / "attribution_guard.c"
    original = original_path.read_text(encoding="utf-8")
    killed = 0
    with tempfile.TemporaryDirectory(prefix="herus-composition-redteam-") as raw:
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
                [str(binary), "attribution-composition"], cwd=ROOT, text=True,
                stdout=subprocess.PIPE, stderr=subprocess.STDOUT, check=False,
            )
            if result.returncode != 0:
                killed += 1
                print(f"PASS {spec['name']}: {spec['why']}")
            else:
                print(f"FAIL {spec['name']}: surviving mutant")
                print(result.stdout[-2500:])
    print(f"COMPOSITION REDTEAM: {killed}/{len(MUTATIONS)} critical mutants killed")
    return 0 if killed == len(MUTATIONS) else 1


if __name__ == "__main__":
    raise SystemExit(main())
