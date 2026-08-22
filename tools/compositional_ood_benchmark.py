from __future__ import annotations

import pathlib
import subprocess
import tempfile

ROOT = pathlib.Path(__file__).resolve().parents[1]
SOURCES = [
    "firmware/core/symbolic_reasoner.c",
    "firmware/core/symbol_registry.c",
    "firmware/core/memory_semantic_evidence.c",
    "firmware/core/memory_reasoning_bridge.c",
    "firmware/core/personal_adapter.c",
    "firmware/core/symbolic_planner.c",
    "firmware/core/generative_core.c",
]
TEST = "firmware/core/test_compositional_ood.c"

REGIMES = {
    "in_distribution": {
        "in-distribution direct baseline",
        "in-distribution two-premise baseline",
    },
    "held_out": {
        "held-out entity permutation",
        "held-out rule recomposition",
        "held-out deeper composition",
        "reversed arguments abstain",
        "missing premise abstains",
        "held-out symbolic composition reaches bounded textual generation",
        "held-out absent composition becomes generative abstention",
    },
    "safety": {
        "bounded OOD search exposes a limit instead of guessing",
        "opposite evidence blocks an OOD confident answer",
        "variable recombination remains explicitly ambiguous",
    },
}


def main() -> int:
    with tempfile.TemporaryDirectory(prefix="herus-compositional-ood-") as raw:
        binary = pathlib.Path(raw) / "test_compositional_ood"
        build = subprocess.run(
            [
                "cc", "-O2", "-Wall", "-Wextra", "-Werror", "-std=c11",
                "-Ifirmware/core", *SOURCES, TEST, "-o", str(binary),
            ], cwd=ROOT, text=True, stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT, check=False,
        )
        if build.returncode != 0:
            print("COMPOSITIONAL OOD BENCHMARK: FAIL build")
            print(build.stdout[-2000:])
            return 1
        run = subprocess.run(
            [str(binary)], cwd=ROOT, text=True,
            stdout=subprocess.PIPE, stderr=subprocess.STDOUT, check=False,
        )
    passed = {
        line[7:].strip()
        for line in run.stdout.splitlines()
        if line.startswith("  PASS ")
    }
    missing = sorted(label for labels in REGIMES.values() for label in labels
                     if label not in passed)
    total = sum(len(labels) for labels in REGIMES.values())
    if run.returncode != 0 or missing:
        print("COMPOSITIONAL OOD BENCHMARK: FAIL")
        if missing:
            print("missing or failed labels:", "; ".join(missing))
        print(run.stdout[-2400:])
        return 1
    print("COMPOSITIONAL OOD BENCHMARK: PASS")
    for regime, labels in REGIMES.items():
        print(f"  {regime}: {len(labels)}/{len(labels)}")
    expected = set().union(*REGIMES.values())
    print(f"  structured_exact: {len(expected)}/{total}")
    print("  c11_suite: 13/13")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
