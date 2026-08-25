#!/usr/bin/env python3
"""Check the Herald compiler against its frozen corpus.

A benchmark that lives in the same file as the code it grades can be tuned to
the cases it already passes. research/hsca_intent_corpus_v1.json is the frozen
answer key: every accepted sentence with the exact digest it must produce, and
every refused sentence with the exact reason it must be refused. This tool
rebuilds the compiler, replays the corpus, and fails on any drift.

A digest change here is a semantic change. It is allowed, but it has to be an
argued edit to the corpus, not a silent one.
"""
import hashlib
import json
import subprocess
import sys
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
CORE = ROOT / "firmware" / "core"
NET = ROOT / "firmware" / "net"
CORPUS = ROOT / "research" / "hsca_intent_corpus_v1.json"


def main() -> int:
    doc = json.loads(CORPUS.read_text(encoding="utf-8"))
    expected = doc["rows"]

    recomputed = hashlib.sha256(
        json.dumps(expected, ensure_ascii=False, sort_keys=True).encode()
    ).hexdigest()
    if recomputed != doc["corpus_digest"]:
        print("  FAIL  the corpus file no longer matches its own digest")
        return 1

    with tempfile.TemporaryDirectory() as td:
        binary = Path(td) / "herald_corpus"
        build = subprocess.run(
            ["cc", "-O2", "-std=c11", f"-I{CORE}", f"-I{NET}",
             str(NET / "crypto.c"), str(CORE / "hir.c"), str(CORE / "herald.c"),
             str(CORE / "test_hsca_herald.c"), "-o", str(binary)],
            capture_output=True, text=True)
        if build.returncode != 0:
            print("  FAIL  the compiler did not build")
            print(build.stderr[-2000:])
            return 1
        run = subprocess.run([str(binary), "--corpus"], capture_output=True, text=True)
        if run.returncode != 0:
            print("  FAIL  the corpus run did not complete")
            return 1

    actual = []
    for line in run.stdout.splitlines():
        if not line.startswith("CORPUS\t"):
            continue
        _, family, status, digest, text = line.split("\t", 4)
        actual.append({"family": family, "status": status, "digest": digest, "text": text})

    problems = []
    if len(actual) != len(expected):
        problems.append(f"corpus size changed: {len(expected)} frozen, {len(actual)} produced")
    for want, got in zip(expected, actual):
        if want == got:
            continue
        problems.append(
            f'"{want["text"]}": frozen {want["status"]}/{want["digest"][:8]} '
            f'-> now {got["status"]}/{got["digest"][:8]}')

    families = {}
    for row in actual:
        if row["family"] == "_refusal":
            continue
        families.setdefault(row["family"], set()).add(row["digest"])
    diverged = [f for f, d in families.items() if len(d) != 1]
    collisions = len(families) - len({next(iter(d)) for d in families.values()})

    print(f"  frozen corpus: {len(expected)} rows, {doc['accepted']} accepted, {doc['refused']} refused")
    print(f"  families: {len(families)}, each converging to one digest: {not diverged}")
    print(f"  cross-family digest collisions: {collisions}")

    if problems:
        print(f"  FAIL  {len(problems)} row(s) drifted from the frozen corpus")
        for p in problems[:20]:
            print(f"    - {p}")
        return 1
    if diverged:
        print(f"  FAIL  families that no longer converge: {diverged}")
        return 1
    if collisions:
        print("  FAIL  two families now share a digest")
        return 1
    print("HSCA CORPUS: frozen input/output pairs reproduced exactly")
    return 0


if __name__ == "__main__":
    sys.exit(main())
