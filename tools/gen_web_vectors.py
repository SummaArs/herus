#!/usr/bin/env python3
"""gen_web_vectors.py — vetores de paridade para o pacote do navegador.

O HERUS Aberto (web/herus-aberto.html) reimplementa Babel, Reed-Solomon,
CRC-32 e o modem 16-FSK em JavaScript, porque um navegador nao roda C11. Dizer
"e o mesmo algoritmo" sem prova seria exatamente a alegacao que este
repositorio nao aceita de ninguem.

Entao o C exporta o proprio comportamento em forma de vetores e a pagina os
reproduz NA CARGA, na frente de quem estiver olhando. Se um unico vetor
divergir, a pagina diz isso em vermelho em vez de funcionar quase certo.
"""
from __future__ import annotations

import json
import pathlib
import random
import subprocess
import sys

ROOT = pathlib.Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT / "tools"))

import babel_ref as B      # noqa: E402
import loom                # noqa: E402

BABEL = ROOT / "firmware" / "build" / "babel_probe"
AETHER = ROOT / "firmware" / "build" / "aether_probe"
OUT = ROOT / "research" / "loom" / "web_vectors.json"


def ask(binary, lines):
    p = subprocess.Popen([str(binary)], stdin=subprocess.PIPE, stdout=subprocess.PIPE)
    out, _ = p.communicate("".join(l + "\n" for l in lines).encode("utf-8"))
    return out.decode("utf-8").splitlines()


def main() -> int:
    for b in (BABEL, AETHER):
        if not b.exists():
            print(f"gen_web_vectors: FAIL sonda ausente {b}")
            return 1

    pack = B.Pack.load(ROOT / "research" / "loom" / "core")
    assert not pack.problems, pack.problems
    rnd = random.Random(20260907)

    # --- Babel: uma amostra ampla, todos os idiomas, todas as operacoes ----
    hirs = []
    seen = set()
    pool = list(loom.enumerate_singles(pack)) + list(loom.enumerate_shapes(pack))
    rnd.shuffle(pool)
    for h in pool:
        if h.key() in seen:
            continue
        seen.add(h.key())
        hirs.append(h)
        if len(hirs) >= 96:
            break

    reqs, meta = [], []
    for h in hirs:
        for l in pack.langs:
            text = B.render(h, l.tag, pack)
            reqs.append(f"R\t{l.tag}\t{h.wire().hex()}")
            meta.append(("R", h, l.tag, text))
            reqs.append(f"C\t{l.tag}\t{text}")
            meta.append(("C", h, l.tag, text))
    got = ask(BABEL, reqs)

    babel_vectors = []
    for (kind, h, tag, text), g in zip(meta, got):
        if kind == "R":
            if g != "OK " + text:
                print(f"gen_web_vectors: FAIL o C discorda da referencia em render "
                      f"[{tag}] {h!r}: {g}")
                return 1
            babel_vectors.append({"op": "render", "lang": tag,
                                  "wire": h.wire().hex(), "text": text})
        else:
            parts = g.split(" ")
            if parts[0] != "OK":
                print(f"gen_web_vectors: FAIL compile [{tag}] {text!r} -> {g}")
                return 1
            babel_vectors.append({"op": "compile", "lang": tag, "text": text,
                                  "digest": parts[1], "wire": parts[2]})

    # --- recusas tipadas: o navegador tem de recusar igual ----------------
    refusals = [
        ("pt", "avisa joao pizza"), ("pt", "avisa a equipe a senha"),
        ("pt", "guarda isso automaticamente"), ("pt", "cheguei"),
        ("pt", "avisa joao maria cheguei"), ("pt", "avisa joao em casaco"),
        ("en", "tell team the password"), ("de", "abbrechen komme spater"),
        ("pt", "avisa joao to aqui to aqui to aqui to aqui to aqui to aqui"),
    ]
    got = ask(BABEL, [f"C\t{t}\t{x}" for t, x in refusals])
    for (tag, text), g in zip(refusals, got):
        babel_vectors.append({"op": "refuse", "lang": tag, "text": text,
                              "status": g.split(" ")[0]})

    # --- Aether: quadro, correcao, tons, selo ------------------------------
    payloads = []
    for h in hirs[:64]:
        payloads.append(h.wire().hex())
    frames = ask(AETHER, [f"P {p}" for p in payloads])
    aether_vectors = []
    for p, g in zip(payloads, frames):
        if not g.startswith("OK "):
            print(f"gen_web_vectors: FAIL pack {p} -> {g}")
            return 1
        aether_vectors.append({"op": "pack", "payload": p, "frame": g[3:]})

    # correcao de um e de dois bytes, e um padrao alem do corrigivel
    fr0 = aether_vectors[0]["frame"]
    pl0 = aether_vectors[0]["payload"]
    corrupt = []
    for spec in [(3, 0x01), (0, 0xff), (32, 0x80), (17, 0x5a)]:
        b = bytearray(bytes.fromhex(fr0))
        b[spec[0]] ^= spec[1]
        corrupt.append((bytes(b).hex(), 1))
    for spec in [((1, 0x0f), (30, 0xf0)), ((5, 0xc3), (12, 0x3c))]:
        b = bytearray(bytes.fromhex(fr0))
        for pos, v in spec:
            b[pos] ^= v
        corrupt.append((bytes(b).hex(), 2))
    b = bytearray(bytes.fromhex(fr0))
    for pos in (2, 9, 20, 28, 31):
        b[pos] ^= 0x77
    corrupt.append((bytes(b).hex(), -1))

    got = ask(AETHER, [f"U {c}" for c, _ in corrupt])
    for (frame, want), g in zip(corrupt, got):
        if want < 0:
            aether_vectors.append({"op": "unpack_fail", "frame": frame,
                                   "status": g.split(" ")[0]})
        else:
            parts = g.split(" ")
            if parts[0] != "OK" or parts[1] != pl0 or int(parts[2]) != want:
                print(f"gen_web_vectors: FAIL correcao esperada {want}: {g}")
                return 1
            aether_vectors.append({"op": "unpack", "frame": frame,
                                   "payload": pl0, "corrected": want})

    tones = ask(AETHER, [f"S {v['frame']}" for v in aether_vectors[:8]
                         if v["op"] == "pack"])
    for v, g in zip([x for x in aether_vectors[:8] if x["op"] == "pack"], tones):
        aether_vectors.append({"op": "tones", "frame": v["frame"], "tones": g[3:]})

    glyphs = ask(AETHER, [f"G {v['frame']}" for v in aether_vectors[:4]
                          if v["op"] == "pack"])
    for v, g in zip([x for x in aether_vectors[:4] if x["op"] == "pack"], glyphs):
        aether_vectors.append({"op": "glyph", "frame": v["frame"], "cells": g[3:]})

    crc_in = ["", "00", "ff", "cheguei".encode().hex(), fr0[:50]]
    got = ask(AETHER, [f"C {x}" for x in crc_in])
    for x, g in zip(crc_in, got):
        if g.startswith("OK"):
            aether_vectors.append({"op": "crc32", "data": x, "crc": g.split(" ")[1]})

    blob = {
        "schema": "herus.web-vectors/1",
        "source": "firmware/build/{babel_probe,aether_probe} — C11 do firmware",
        "pack_digest": pack.grammar.get("version", 1),
        "babel": babel_vectors,
        "aether": aether_vectors,
    }
    OUT.write_text(json.dumps(blob, ensure_ascii=False, separators=(",", ":")),
                   encoding="utf-8")
    print(f"gen_web_vectors: OK {len(babel_vectors)} vetores de Babel + "
          f"{len(aether_vectors)} de Aether -> {OUT.relative_to(ROOT)} "
          f"({OUT.stat().st_size:,} bytes)".replace(",", "."))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
