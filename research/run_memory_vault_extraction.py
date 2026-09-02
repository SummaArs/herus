"""Run the conservative memory_vault.c structural extraction."""
from __future__ import annotations

import argparse
import json
from pathlib import Path

from memory_vault_structural_extractor import compare_source


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("source", type=Path)
    parser.add_argument("case", type=Path)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()
    result = compare_source(args.source, args.case)
    args.output.write_text(json.dumps(result.to_dict(), indent=2, sort_keys=True) + "\n", encoding="utf-8")
    return 0 if result.verdict.value == "EXTRACTED_MATCH" else 1


if __name__ == "__main__":
    raise SystemExit(main())
