"""Run the real C voice parser over official SLURP JSONL sentences.

This wrapper is a corpus adapter, not an NLU system. It emits aggregate
counts only; source intent labels remain benchmark labels and are never
converted to HERUS commands.
"""
from __future__ import annotations

import argparse
import collections
import json
import subprocess
from pathlib import Path


MAX_JSONL_LINE_BYTES = 1024 * 1024
MAX_SENTENCE_BYTES = 4095


def run(c_runner: Path, inputs: list[Path]) -> dict[str, int]:
    rows = 0
    invalid_rows = 0
    split_rows: collections.Counter[str] = collections.Counter()
    for path in inputs:
        if not path.is_file():
            raise FileNotFoundError(path)
    if not c_runner.is_file():
        raise FileNotFoundError(c_runner)
    process = subprocess.Popen(
        [str(c_runner)],
        stdin=subprocess.PIPE,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
    )
    if process.stdin is None or process.stdout is None or process.stderr is None:
        process.kill()
        process.wait()
        raise RuntimeError("c_runner_pipe_setup_failed")
    try:
        for path in inputs:
            split = path.stem
            with path.open("r", encoding="utf-8") as handle:
                for line_number, line in enumerate(handle, start=1):
                    if len(line.encode("utf-8")) > MAX_JSONL_LINE_BYTES:
                        raise ValueError(f"jsonl_line_too_long:{path}:{line_number}")
                    if not line.strip():
                        invalid_rows += 1
                        continue
                    row = json.loads(line)
                    sentence = row.get("sentence") if isinstance(row, dict) else None
                    if (
                        not isinstance(sentence, str)
                        or "\n" in sentence
                        or "\r" in sentence
                        or len(sentence.encode("utf-8")) > MAX_SENTENCE_BYTES
                    ):
                        invalid_rows += 1
                        continue
                    process.stdin.write(sentence.encode("utf-8") + b"\n")
                    rows += 1
                    split_rows[split] += 1
        process.stdin.close()
        stdout = process.stdout.read()
        stderr = process.stderr.read()
    except Exception:
        process.kill()
        process.wait()
        raise
    returncode = process.wait()
    if returncode != 0:
        raise RuntimeError(f"c_runner_failed:{returncode}:{stderr.decode('utf-8', errors='replace')}")
    result: dict[str, int] = {
        "jsonl_rows_seen": rows + invalid_rows,
        "jsonl_rows_sent_to_c": rows,
        "jsonl_extraction_failures": invalid_rows,
        "herus_command_authority": 0,
        "automatic_label_mapping": 0,
    }
    for key, value in split_rows.items():
        result[f"jsonl_rows_{key}"] = value
    for raw_line in stdout.decode("utf-8", errors="strict").splitlines():
        key, value = raw_line.split("=", 1)
        result[key] = int(value)
    if result["parser_input_rows"] != rows:
        raise RuntimeError("c_runner_row_count_mismatch")
    return result


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--runner", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("jsonl", nargs=3, type=Path)
    args = parser.parse_args()
    result = run(args.runner, args.jsonl)
    args.output.write_text(json.dumps(result, indent=2) + "\n", encoding="utf-8")
    print(json.dumps(result, indent=2))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
