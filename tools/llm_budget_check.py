#!/usr/bin/env python3
"""Check coarse local-model storage budgets for HERUS targets.

This is a sizing tool, not an inference benchmark. It compares declared model
bytes and optional working-set requirements with published board capacities. It
does not claim that a model fits at runtime, that a target is fast enough, or
that energy/thermal behaviour is acceptable.
"""
from __future__ import annotations

import argparse
import json
import sys
from dataclasses import asdict, dataclass


@dataclass(frozen=True)
class BoardBudget:
    name: str
    flash_bytes: int
    psram_bytes: int
    internal_sram_bytes: int
    source: str


BOARDS = {
    "lilygo-t3-s3-v1.3": BoardBudget(
        "lilygo-t3-s3-v1.3", 4_000_000, 2_000_000, 512_000,
        "https://lilygo.cc/en-us/products/t3-s3-v1-3",
    ),
    "esp32-s3-n16r8-article": BoardBudget(
        "esp32-s3-n16r8-article", 16_000_000, 8_000_000, 512_000,
        "https://github.com/slvDev/esp32-ai",
    ),
    "rp2350-pico2": BoardBudget(
        "rp2350-pico2", 4_000_000, 0, 520_000,
        "https://www.raspberrypi.com/products/rp2350/",
    ),
}


def model_bytes(params: int, bits_per_param: int) -> int:
    if params < 0 or bits_per_param <= 0:
        raise ValueError("params must be non-negative and bits_per_param positive")
    return (params * bits_per_param + 7) // 8


def check_budget(
    board: BoardBudget,
    params: int,
    bits_per_param: int,
    artifact_bytes: int | None,
    required_psram_bytes: int | None,
    required_internal_bytes: int | None,
) -> dict[str, object]:
    raw_weight_bytes = model_bytes(params, bits_per_param)
    stored_bytes = raw_weight_bytes if artifact_bytes is None else artifact_bytes
    result = {
        "board": asdict(board),
        "model": {
            "parameters": params,
            "bits_per_parameter": bits_per_param,
            "raw_weight_bytes": raw_weight_bytes,
            "stored_artifact_bytes": stored_bytes,
        },
        "fit": {
            "stored_artifact_in_flash": stored_bytes <= board.flash_bytes,
            "required_psram_in_capacity": (
                required_psram_bytes is None
                or required_psram_bytes <= board.psram_bytes
            ),
            "required_internal_in_capacity": (
                required_internal_bytes is None
                or required_internal_bytes <= board.internal_sram_bytes
            ),
        },
        "limitations": [
            "does not measure inference latency or tokens per second",
            "does not account for firmware, partitions, filesystem, radio, BLE, security, stacks or caches",
            "does not prove PSRAM bandwidth, thermal stability, energy or model quality",
        ],
    }
    result["fit"]["coarse_capacity_fit"] = all(result["fit"].values())
    return result


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--board", choices=sorted(BOARDS), default="lilygo-t3-s3-v1.3")
    parser.add_argument("--params", type=int, default=28_900_000)
    parser.add_argument("--bits", type=int, default=4)
    parser.add_argument("--artifact-bytes", type=int, default=14_912_332)
    parser.add_argument("--required-psram-bytes", type=int)
    parser.add_argument("--required-internal-bytes", type=int)
    parser.add_argument("--json", action="store_true", dest="as_json")
    return parser


def main(argv: list[str] | None = None) -> int:
    args = build_parser().parse_args(argv)
    if args.params < 0 or args.bits <= 0:
        print("params must be non-negative and bits must be positive", file=sys.stderr)
        return 2
    if args.artifact_bytes is not None and args.artifact_bytes < 0:
        print("artifact-bytes must be non-negative", file=sys.stderr)
        return 2
    board = BOARDS[args.board]
    result = check_budget(
        board,
        args.params,
        args.bits,
        args.artifact_bytes,
        args.required_psram_bytes,
        args.required_internal_bytes,
    )
    if args.as_json:
        print(json.dumps(result, sort_keys=True))
    else:
        fit = result["fit"]
        model = result["model"]
        print(f"board={board.name}")
        print(f"raw_weight_bytes={model['raw_weight_bytes']}")
        print(f"stored_artifact_bytes={model['stored_artifact_bytes']}")
        print(f"flash_capacity_bytes={board.flash_bytes}")
        print(f"stored_artifact_in_flash={fit['stored_artifact_in_flash']}")
        print(f"required_psram_in_capacity={fit['required_psram_in_capacity']}")
        print(f"required_internal_in_capacity={fit['required_internal_in_capacity']}")
        print(f"coarse_capacity_fit={fit['coarse_capacity_fit']}")
        print("NOTE: coarse sizing only; no runtime, energy, thermal or quality claim")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
