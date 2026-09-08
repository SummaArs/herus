#!/usr/bin/env python3
"""test_babel_cross.py — o C e a referencia dizem a MESMA coisa, ou nada vale.

O portao do Loom prova as propriedades sobre tools/babel_ref.py. Isso e uma
referencia, nao o artefato: um espelho pode concordar consigo mesmo e discordar
do firmware. Esta suite dirige o C real (firmware/build/babel_probe) com todo o
espaco enumerado pelo Loom e compara caso a caso:

    para cada significado alcancavel h e cada idioma L:
        render_C(h, L)   == render_PY(h, L)      byte a byte
        compile_C(t, L)  == compile_PY(t, L)     status, digest e 24 bytes

Uma divergencia aqui nao e "ajuste o teste": e um dos dois esta errado, e o
contraexemplo esta impresso.
"""

from __future__ import annotations

import pathlib
import subprocess
import sys

HERE = pathlib.Path(__file__).resolve().parent
ROOT = HERE.parent
sys.path.insert(0, str(HERE))

import babel_ref as B      # noqa: E402
import loom                # noqa: E402

PROBE = ROOT / "firmware" / "build" / "babel_probe"


class Probe:
    """Conversa com o C por linhas. Escreve em lote e le em lote: uma ida e
    volta por caso seria dominada pelo custo do pipe, nao pela computacao."""

    def __init__(self, path):
        self.p = subprocess.Popen([str(path)], stdin=subprocess.PIPE,
                                  stdout=subprocess.PIPE)

    def ask(self, requests: list[str]) -> list[str]:
        payload = "".join(r + "\n" for r in requests).encode("utf-8")
        self.p.stdin.write(payload)
        self.p.stdin.flush()
        out = []
        while len(out) < len(requests):
            line = self.p.stdout.readline()
            if not line:
                raise RuntimeError("a sonda morreu")
            out.append(line.decode("utf-8").rstrip("\n"))
        return out

    def close(self):
        self.p.stdin.close()
        self.p.wait()


def main() -> int:
    if not PROBE.exists():
        print(f"BABEL CROSS: FAIL sonda ausente ({PROBE}) — rode `make babel`")
        return 1

    pack = B.Pack.load(ROOT / "research" / "loom" / "core")
    if pack.problems:
        print("BABEL CROSS: FAIL o pacote nao passa no portao do Loom")
        for p in pack.problems[:5]:
            print("  ", p)
        return 1

    probe = Probe(PROBE)
    failures: list[str] = []
    compared = 0

    # ---- 1. idiomas declarados batem -----------------------------------
    langs = probe.ask(["L"])[0]
    want = "OK " + " ".join(f"{l.tag}:{l.endonym}" for l in pack.langs)
    if langs != want:
        failures.append(f"tabela de idiomas divergiu:\n  C  {langs}\n  PY {want}")

    # ---- 2. render e compile, todo o espaco enumerado -------------------
    BATCH = 2000
    pending: list[tuple[str, B.Hir, str, str]] = []   # (kind, h, tag, py)

    def flush():
        nonlocal compared
        if not pending:
            return
        reqs = []
        for kind, h, tag, py in pending:
            if kind == "R":
                reqs.append(f"R\t{tag}\t{h.wire().hex()}")
            else:
                reqs.append(f"C\t{tag}\t{py}")
        got = probe.ask(reqs)
        for (kind, h, tag, py), g in zip(pending, got):
            compared += 1
            if kind == "R":
                exp = "OK " + py
                if g != exp:
                    failures.append(f"render [{tag}] {h!r}\n  C  {g}\n  PY {exp}")
            else:
                u = B.compile(py, tag, pack)
                if u.ok:
                    exp = f"OK {u.hir.digest().hex()} {u.hir.wire().hex()}"
                else:
                    exp = u.status
                if g != exp:
                    failures.append(f"compile [{tag}] {py!r}\n  C  {g}\n  PY {exp}")
        pending.clear()

    for gen in (loom.enumerate_singles, loom.enumerate_shapes,
                loom.enumerate_pairs):
        for h in gen(pack):
            for l in pack.langs:
                text = B.render(h, l.tag, pack)
                if text is None:
                    failures.append(f"referencia nao renderiza [{l.tag}] {h!r}")
                    continue
                pending.append(("R", h, l.tag, text))
                pending.append(("C", h, l.tag, text))
                if len(pending) >= BATCH:
                    flush()
            if len(failures) > 20:
                break
        if len(failures) > 20:
            break
    flush()

    # ---- 3. o corpus congelado, pelo C ---------------------------------
    import json
    corpus = json.loads((ROOT / "research" / "hsca_intent_corpus_v1.json")
                        .read_text(encoding="utf-8"))
    reqs = [f"C\tpt\t{row['text']}" for row in corpus["rows"]]
    got = probe.ask(reqs)
    for row, g in zip(corpus["rows"], got):
        compared += 1
        u = B.compile(row["text"], "pt", pack)
        exp = (f"OK {u.hir.digest().hex()} {u.hir.wire().hex()}" if u.ok
               else u.status)
        if g != exp:
            failures.append(f"corpus {row['text']!r}\n  C  {g}\n  PY {exp}")

    # ---- 4. dobramento sobre entrada hostil ----------------------------
    hostile = [
        "CHEGUEI", "Cheguei!!!", "  cheguei   ", "che\tguei", "chegou-me",
        "ÀÉÎÕÜ", "着いた。", "ＡＢＣ", "cheguei　agora", "a" * 200,
        "", "?", "@", "cheguei" + chr(0x00A9), "１２３",
    ]
    reqs = [f"F\t-\t{t}" for t in hostile]
    got = probe.ask(reqs)
    for t, g in zip(hostile, got):
        compared += 1
        if t == "":
            continue
        f = B.fold(t)
        exp = f.status if f.status != B.OK else f"OK {f.text}"
        if g != exp:
            failures.append(f"fold {t!r}\n  C  {g}\n  PY {exp}")

    probe.close()

    if failures:
        print(f"BABEL CROSS: FAIL {len(failures)} divergencias em "
              f"{compared:,} comparacoes".replace(",", "."))
        for f in failures[:12]:
            print("  " + f.replace("\n", "\n  "))
        return 1
    print(f"BABEL CROSS: PASS {compared:,} comparacoes C x referencia, "
          f"0 divergencias".replace(",", "."))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
