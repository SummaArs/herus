"""Adversarial campaign for non-delegable shares and cross-principal revocation."""
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
        "name": "derived-share-delegation-bypass",
        "old": "    if (node->source_share_id != 0u || node->secondary_share_id != 0u)\n        return AG_E_SHARE;",
        "new": "    if (0) /* REDTEAM */\n        return AG_E_SHARE;",
        "why": "a derived imported share must not become a new delegation",
    },
    {
        "name": "imported-composition-bypass",
        "old": "    if (left->source_share_id != 0u || right->source_share_id != 0u)\n        return AG_E_SHARE;",
        "new": "    if (0) /* REDTEAM */\n        return AG_E_SHARE;",
        "why": "imported authority must not enter a new composition chain",
    },
    {
        "name": "revoked-share-reentry-bypass",
        "old": "    if (share_id_is_revoked(index, share->share_id)) return AG_E_REVOKED;",
        "new": "    if (0) /* REDTEAM */ return AG_E_REVOKED;",
        "why": "a cached revocation must block a later import",
    },
    {
        "name": "preventive-revocation-bypass",
        "old": "        return record_revoked_share(index, revocation->share_id);",
        "new": "        return AG_E_PARENT; /* REDTEAM */",
        "why": "a recipient must be able to cache revocation before import",
    },
    {
        "name": "transitive-revocation-bypass",
        "old": "    status = ag_revoke(index, index->nodes[position].node_id, 1u, generation);",
        "new": "    status = AG_OK; /* REDTEAM */",
        "why": "issuer revocation must reach the imported root and descendants",
    },
    {
        "name": "revocation-recipient-bypass",
        "old": "    if (index->exported_share_recipient_ids[slot] != recipient_principal_id)\n        return AG_E_PRINCIPAL;",
        "new": "    if (0) /* REDTEAM */\n        return AG_E_PRINCIPAL;",
        "why": "a revocation cannot be redirected to another principal",
    },
    {
        "name": "revoked-share-id-reuse-bypass",
        "old": "    if (share_id_is_exported(index, share_id) ||\n        share_id_is_revoked(index, share_id))\n        return AG_E_REPLAY;",
        "new": "    if (0) /* REDTEAM */\n        return AG_E_REPLAY;",
        "why": "revoked capability identifiers must not be reused",
    },
    {
        "name": "share-lineage-propagation-bypass",
        "old": "    node->source_share_id = parent->source_share_id;",
        "new": "    node->source_share_id = 0u; /* REDTEAM */",
        "why": "derived records must retain the source share lineage",
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
    with tempfile.TemporaryDirectory(prefix="herus-transitive-redteam-") as raw:
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
                [str(binary), "attribution-transitive"], cwd=ROOT, text=True,
                stdout=subprocess.PIPE, stderr=subprocess.STDOUT, check=False,
            )
            if result.returncode != 0:
                killed += 1
                print(f"PASS {spec['name']}: {spec['why']}")
            else:
                print(f"FAIL {spec['name']}: surviving mutant")
                print(result.stdout[-2500:])
    print(f"TRANSITIVE REDTEAM: {killed}/{len(MUTATIONS)} critical mutants killed")
    return 0 if killed == len(MUTATIONS) else 1


if __name__ == "__main__":
    raise SystemExit(main())
