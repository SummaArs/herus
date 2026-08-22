from __future__ import annotations

import pathlib
import re
import subprocess
import tempfile

ROOT = pathlib.Path(__file__).resolve().parents[1]
INCLUDE = ROOT / "firmware" / "core"

# Host-side guardrails only. They are deliberately not presented as ESP32 limits.
BUDGETS = {
    "generative_core": {"text": 4096, "data": 0, "bss": 0},
    "personal_adapter": {"text": 2048, "data": 0, "bss": 0},
    "composed_dialogue": {"text": 4096, "data": 0, "bss": 0},
}


def compile_object(source: pathlib.Path, output: pathlib.Path) -> None:
    subprocess.run([
        "cc", "-O2", "-Wall", "-Wextra", "-Werror", "-std=c11",
        f"-I{INCLUDE}", "-c", str(source), "-o", str(output),
    ], cwd=ROOT, check=True)


def read_size(object_path: pathlib.Path) -> tuple[int, int, int]:
    result = subprocess.run(["size", str(object_path)], cwd=ROOT,
                            check=True, text=True, stdout=subprocess.PIPE)
    rows = result.stdout.strip().splitlines()
    if len(rows) < 2:
        raise RuntimeError(f"unexpected size output for {object_path}")
    fields = re.split(r"\s+", rows[-1].strip())
    if len(fields) < 4:
        raise RuntimeError(f"unparseable size output: {rows[-1]}")
    return tuple(int(fields[i]) for i in range(3))


def main() -> int:
    sources = {
        "generative_core": ROOT / "firmware" / "core" / "generative_core.c",
        "personal_adapter": ROOT / "firmware" / "core" / "personal_adapter.c",
        "composed_dialogue": ROOT / "firmware" / "core" / "composed_dialogue.c",
    }
    failures: list[str] = []
    with tempfile.TemporaryDirectory(prefix="herus-generative-budget-") as raw:
        work = pathlib.Path(raw)
        for name, source in sources.items():
            output = work / f"{name}.o"
            compile_object(source, output)
            text, data, bss = read_size(output)
            limits = BUDGETS[name]
            print(f"{name}: text={text} data={data} bss={bss} "
                  f"limits={limits['text']}/{limits['data']}/{limits['bss']}")
            for field, actual in (("text", text), ("data", data), ("bss", bss)):
                if actual > limits[field]:
                    failures.append(f"{name}.{field}={actual}>{limits[field]}")
    if failures:
        print("GENERATIVE CORE BUDGET: FAIL " + ", ".join(failures))
        return 1
    print("GENERATIVE CORE BUDGET: PASS host-side object limits")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
