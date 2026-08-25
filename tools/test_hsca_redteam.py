#!/usr/bin/env python3
"""HSCA red team: remove one control, require the suite to notice.

A passing test suite proves that the code does what the code does. It says
nothing about whether the assertions are load bearing. This harness takes the
five HSCA modules apart one control at a time — coverage, refusal, ordering,
authentication, expiry, confirmation, revocation, the Core matrix — rebuilds,
and requires the corresponding suite to fail. A mutant that survives is a
control that was decoration.

Every mutation is a literal source edit applied to a throwaway copy of the
tree. Nothing is patched in place.
"""
import json
import shutil
import subprocess
import sys
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
CORE = ROOT / "firmware" / "core"
NET = ROOT / "firmware" / "net"

SUITES = {
    "herald": ["crypto.c", "hir.c", "herald.c", "test_hsca_herald.c"],
    "ladder": ["ladder.c", "test_hsca_ladder.c"],
    "drift":  ["crypto.c", "hir.c", "herald.c", "drift.c", "test_hsca_drift.c"],
    "aura":   ["crypto.c", "aura.c", "test_hsca_aura.c"],
    "keel":   ["crypto.c", "hir.c", "herald.c", "ladder.c", "drift.c", "aura.c",
               "keel.c", "test_hsca_keel.c"],
    "finale": ["crypto.c", "hir.c", "herald.c", "ladder.c", "drift.c", "aura.c",
               "keel.c", "test_hsca_finale.c"],
}

# (name, suite, file, needle, replacement)
MUTANTS = [
    ("coverage: an unknown token is silently skipped", "herald", "herald.c",
     """        if (!hit) {
            out->gap.present = 1;""",
     """        if (!hit) { i = (uint8_t)(i + 1u); continue; }
        if (0) {
            out->gap.present = 1;"""),

    ("refusal: the protected/authority pre-scan is removed", "herald", "herald.c",
     "    for (i = 0; i < ntok; i++) {\n        uint8_t w;\n        for (w = HERALD_PHRASE_MAX; w >= 1u; w--) {\n            const lex_t *probe;",
     "    for (i = 0; i < 0u; i++) {\n        uint8_t w;\n        for (w = HERALD_PHRASE_MAX; w >= 1u; w--) {\n            const lex_t *probe;"),

    ("ambiguity: a conflicting filler overwrites instead of refusing", "herald", "hir.c",
     "        if (h->slot[i].filler == filler) return HIR_OK;   /* idempotent */\n        return HIR_E_DUPLICATE_ROLE;                      /* conflicting fill */",
     "        if (h->slot[i].filler == filler) return HIR_OK;\n        h->slot[i].filler = filler; return HIR_OK;"),

    ("canonicality: slots are no longer sorted before hashing", "herald", "hir.c",
     "    memcpy(s, h->slot, sizeof(hir_slot_t) * h->slot_count);\n    sort_slots(s, h->slot_count);\n\n    out[n++] = 'H';",
     "    memcpy(s, h->slot, sizeof(hir_slot_t) * h->slot_count);\n\n    out[n++] = 'H';"),

    ("wire: reserved bytes may carry data", "herald", "hir.c",
     "    for (; off < HIR_WIRE_BYTES; off++) {\n        if (in[off] != 0u) return HIR_E_RANGE;\n    }",
     "    for (; off < HIR_WIRE_BYTES; off++) {\n        if (in[off] != 0u) { /* tolerated */ }\n    }"),

    ("encoding: a byte outside the accepted set is folded away", "herald", "herald.c",
     "        } else {\n            return 1;                                /* unknown ASCII byte */\n        }",
     "        } else {\n            folded = ' '; i++;\n        }"),

    ("ladder: a carrier may exceed its declared payload class", "ladder", "ladder.c",
     "    if (f->payload_class > p->max_class) return 0;", "    (void)0;"),

    ("ladder: reach is no longer compared against the distance", "ladder", "ladder.c",
     "    if (p->reach_m < f->peer_distance_m) return 0;", "    (void)0;"),

    ("ladder: commit stops looking at confirmation", "ladder", "ladder.c",
     "    if (!confirmed) return LDR_E_UNCONFIRMED;", "    (void)confirmed;"),

    ("ladder: the confirmed class is no longer compared", "ladder", "ladder.c",
     "    if (confirmed_class != plan->payload_class) return LDR_E_CLASS_DRIFT;",
     "    (void)confirmed_class;"),

    ("drift: the AEAD tag is no longer required to verify", "drift", "drift.c",
     "        if (rc == 0) return DRIFT_OK;\n        memset(wire, 0, HIR_WIRE_BYTES);\n        return DRIFT_E_AUTH;",
     "        (void)rc; return DRIFT_OK;"),

    ("drift: expired bundles are carried anyway", "drift", "drift.c",
     "    if (b->expires_at <= now) return DRIFT_E_EXPIRED;", "    (void)now;"),

    ("drift: hop count no longer decrements", "drift", "drift.c",
     "        copy.hops_left = (uint8_t)(copy.hops_left - 1u);", "        copy.hops_left = copy.hops_left;"),

    ("drift: fanout is unbounded", "drift", "drift.c",
     "        if (from->slot[i].fanout_left == 0u) continue;", "        (void)0;"),

    ("drift: a delivered meaning can be delivered again", "drift", "drift.c",
     "        mark_seen(s, id);\n        return DRIFT_OK;", "        return DRIFT_OK;"),

    ("drift: custody accepts any payload class", "drift", "drift.c",
     "    if (b->payload_class != 1u) return DRIFT_E_CLASS;\n    if (b->expires_at <= now) return DRIFT_E_EXPIRED;",
     "    if (b->expires_at <= now) return DRIFT_E_EXPIRED;"),

    ("aura: an epoch is not consumed, so a beacon replays", "aura", "aura.c",
     "                aura_step(probe);\n                memcpy(b->peer[i].key, probe, AURA_KEY_BYTES);",
     "                memcpy(b->peer[i].key, b->peer[i].key, AURA_KEY_BYTES);"),

    ("aura: revoked peers are still recognised", "aura", "aura.c",
     "        if (!b->peer[i].active) continue;", "        (void)0;"),

    ("aura: the lookahead window is unbounded in practice", "aura", "aura.c",
     "        for (w = 0; w < AURA_WINDOW; w++) {", "        for (w = 0; w < 250u; w++) {"),

    ("keel: the antenna role is allowed to do anything", "keel", "keel.c",
     "    case KEEL_CORE_ANTENNA:\n        return action == KEEL_ACT_RELAY_CIPHERTEXT || action == KEEL_ACT_SAT_UPLINK;",
     "    case KEEL_CORE_ANTENNA:\n        return 1;"),

    ("keel: an unknown role is allowed everything", "keel", "keel.c",
     "    case KEEL_CORE_KNOWLEDGE:\n        return action == KEEL_ACT_PROPOSE_KNOWLEDGE;\n    default:\n        return 0;",
     "    case KEEL_CORE_KNOWLEDGE:\n        return action == KEEL_ACT_PROPOSE_KNOWLEDGE;\n    default:\n        return 1;"),

    ("keel: an external package may land in the personal namespace", "keel", "keel.c",
     "    if (namespace_personal) return 0;", "    (void)namespace_personal;"),

    ("keel: a knowledge package no longer needs local confirmation", "keel", "keel.c",
     "    if (!locally_confirmed) return 0;", "    (void)locally_confirmed;"),

    ("finale: the chain commits without confirmation", "finale", "ladder.c",
     "    if (!confirmed) return LDR_E_UNCONFIRMED;", "    (void)confirmed;"),

    ("finale: the Core boundary is ignored when planning", "finale", "ladder.c",
     "    if (p->needs_core && !f->core_present) return 0;", "    (void)0;"),

    ("finale: a meaning may leave without knowing it needs a finger", "finale", "hir.c",
     "    case HIR_OP_PERGUNTAR: return 0;\n    default:               return 1;",
     "    case HIR_OP_PERGUNTAR: return 0;\n    default:               return 0;"),
]


ARGS = {"aura": ["--quick"]}


def build_and_run(workdir: Path, suite: str) -> int:
    src = [str(workdir / f) for f in SUITES[suite]]
    binary = workdir / ("t_" + suite)
    cc = subprocess.run(["cc", "-O2", "-std=c11", "-I", str(workdir),
                         *src, "-o", str(binary)],
                        capture_output=True, text=True)
    if cc.returncode != 0:
        return 2                       # refusing to compile is a detection too
    run = subprocess.run([str(binary), *ARGS.get(suite, [])], capture_output=True, text=True)
    if run.returncode != 0:
        return 1
    if "FAIL" in run.stdout:
        return 1
    return 0


def stage(tmp: Path) -> Path:
    work = tmp / "core"
    work.mkdir(parents=True, exist_ok=True)
    for f in CORE.glob("*.h"):
        shutil.copy(f, work / f.name)
    for name in {n for s in SUITES.values() for n in s}:
        source = NET / name if (NET / name).exists() else CORE / name
        shutil.copy(source, work / name)
    for f in NET.glob("*.h"):
        shutil.copy(f, work / f.name)
    return work


def main() -> int:
    with tempfile.TemporaryDirectory() as td:
        tmp = Path(td)
        baseline = stage(tmp)
        for suite in SUITES:
            if build_and_run(baseline, suite) != 0:
                print(f"  FAIL  baseline suite {suite} does not pass; nothing below means anything")
                return 1
        print(f"  baseline: {len(SUITES)} suites pass unmutated")

        killed, survived = 0, []
        for name, suite, filename, needle, replacement in MUTANTS:
            work = stage(tmp / f"m{killed + len(survived)}")
            target = work / filename
            text = target.read_text()
            if needle not in text:
                print(f"  FAIL  mutation is stale, its needle is gone: {name}")
                survived.append(name)
                continue
            target.write_text(text.replace(needle, replacement, 1))
            if build_and_run(work, suite) != 0:
                killed += 1
            else:
                survived.append(name)
                print(f"  SURVIVED  {name}  [{suite}]")

        total = len(MUTANTS)
        print(f"HSCA REDTEAM: {killed}/{total} mutants detected")
        if survived:
            print("  the following controls are not load bearing:")
            for s in survived:
                print(f"    - {s}")
            return 1
        return 0


if __name__ == "__main__":
    sys.exit(main())
