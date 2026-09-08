#!/usr/bin/env python3
"""Demonstração mínima do HERUS: significado atravessa um canal e não aceita adulteração."""
from __future__ import annotations

import argparse
import pathlib
import subprocess
import sys

ROOT = pathlib.Path(__file__).resolve().parents[1]
PIPE = ROOT / "firmware" / "build" / "meaning_pipeline_probe"
AETHER = ROOT / "firmware" / "build" / "aether_probe"


def ask(path: pathlib.Path, lines: list[str]) -> list[str]:
    if not path.exists():
        raise RuntimeError(f"probe ausente: {path}; rode make -C firmware demo-meaning")
    result = subprocess.run(
        [str(path)],
        input=("\n".join(lines) + "\n").encode("utf-8"),
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        check=False,
    )
    if result.returncode != 0:
        raise RuntimeError(f"probe falhou ({result.returncode}): {result.stderr.decode()}")
    return result.stdout.decode("utf-8").splitlines()


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--check", action="store_true", help="falha se a demonstração não cumprir suas invariantes")
    args = parser.parse_args()

    text = "emergencia estou aqui"
    pipeline = ask(PIPE, [f"P\tpt\tja\t7\t{text}"])[0]
    fields = pipeline.split(" ", 3)
    if len(fields) != 4 or fields[0] != "OK":
        raise RuntimeError(f"pipeline recusou o vetor demonstrável: {pipeline}")
    _, frame, wire, rendered = fields

    damaged = bytearray.fromhex(frame)
    for offset in (1, 2, 3, 4, 5):
        damaged[offset] ^= 0xA5
    corrupted_frame = damaged.hex()
    received, corrupted = ask(AETHER, [f"U {frame}", f"U {corrupted_frame}"])
    if not received.startswith("OK "):
        raise RuntimeError(f"frame íntegro não foi recebido: {received}")
    if corrupted.startswith("OK "):
        raise RuntimeError("frame adulterado foi aceito como sucesso")

    print("HERUS — demonstração de significado verificável")
    print(f"1. Intenção humana:       {text!r}")
    print("2. Significado canônico:  HIR de 24 bytes, sem autoridade implícita")
    print(f"   HIR wire:              {wire}")
    print("3. Transporte limitado:   quadro Aether de 33 bytes")
    print(f"   frame:                 {frame}")
    print(f"4. Outro dispositivo:     {rendered}")
    print("5. Adulteração deliberada:")
    print(f"   resultado:             REJEITADO ({corrupted})")
    print("\nNota: esta demonstração usa probes host-only; não é rádio nem validação física.")

    if args.check:
        assert len(wire) == 48
        assert len(frame) == 66
        assert corrupted.startswith("UNCORRECTED") or corrupted.startswith("ARG")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (AssertionError, RuntimeError) as error:
        print(f"DEMO: FAIL — {error}", file=sys.stderr)
        raise SystemExit(1)
