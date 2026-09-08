#!/usr/bin/env python3
"""test_loom_extension.py — a plataforma é extensível, e o portão morde.

`docs/25-significado/201` afirma que um terceiro estende o espaço semântico sem
tocar em firmware. Afirmação não é prova. Esta suíte prova as duas metades:

  POSITIVA  o pacote de domínio `research/loom/campo` passa no MESMO portão de
            dez invariantes que o núcleo passa, e exercita 29 mil lacunas
            tipadas por idioma — um caminho que existia no esquema e nunca
            tinha sido testado.

  NEGATIVA  uma extensão mal-comportada é RECUSADA, e por motivo tipado:
            redefinir namespace do núcleo, invadir a faixa de outro namespace,
            colidir forma com o núcleo, ou deixar de declarar a lacuna.

A metade negativa é a que importa. Um mecanismo de extensão que aceita tudo não
é plataforma, é buraco.
"""
from __future__ import annotations

import copy
import json
import pathlib
import sys
import tempfile

HERE = pathlib.Path(__file__).resolve().parent
ROOT = HERE.parent
sys.path.insert(0, str(HERE))

import babel_ref as B   # noqa: E402
import loom             # noqa: E402

CORE = ROOT / "research" / "loom" / "core"
CAMPO = ROOT / "research" / "loom" / "campo"

PASS, FAIL = 0, 0


def ok(cond, what):
    global PASS, FAIL
    if cond:
        PASS += 1
        print(f"  PASS  {what}")
    else:
        FAIL += 1
        print(f"  FAIL  {what}")


def load_parts(*dirs):
    parts = []
    for d in dirs:
        for f in sorted(pathlib.Path(d).glob("*.hlx.json")):
            parts.append(json.loads(f.read_text(encoding="utf-8")))
    return parts


def refuses(parts, needle):
    """A carga recusa com esse motivo, ou o portão acusa com esse motivo."""
    try:
        pack = B.Pack(copy.deepcopy(parts))
    except B.Collision as exc:
        return needle in str(exc)
    return any(needle in p for p in pack.problems)


def main() -> int:
    print("--- loom: extensão de pacote por terceiro ---")
    base = load_parts(CORE)
    ext = load_parts(CAMPO)

    # ---- POSITIVA -------------------------------------------------------
    pack = B.Pack(copy.deepcopy(base + ext))
    ok(not pack.problems, "núcleo + domínio carregam sem colisão")
    if pack.problems:
        for p in pack.problems[:5]:
            print("        " + p)
    ok(len(pack.concepts) == 157, f"157 conceitos ({len(pack.concepts)})")
    ok(pack.extensions == ["campo"], "a extensão se declara pelo nome")
    ok("CAMPO" in pack.namespaces and pack.namespaces["CAMPO"]["base"] == 1536,
       "o namespace de domínio entrou na faixa 1536-2047")

    sym = pack.sym_of["CAMPO.EVACUAR"]
    ok(sym == 1536, f"CAMPO.EVACUAR = base + 0 = 1536 ({sym})")
    ok(pack.render_form(sym, "pt") == "evacuar", "renderiza em português")
    ok(pack.render_form(sym, "ja") is None, "e não renderiza em japonês")
    ok(pack.gap_langs(sym) == {"it", "de", "ja", "zh"},
       "a lacuna dos quatro idiomas está DECLARADA, não descoberta")

    h = B.Hir()
    h.op = B.OP_COMUNICAR
    h.put(B.ROLE_QUEM, pack.sym_of["PER.EQUIPE"])
    h.put(B.ROLE_O_QUE, sym)
    ok(B.render(h, "pt", pack) == "avisa equipe evacuar",
       "um significado de domínio veste o português")
    ok(B.render(h, "ja", pack) is None and pack.render_gap(h, "ja") == sym,
       "e em japonês devolve lacuna, apontando QUAL símbolo")
    u = B.compile("avisa equipe evacuar", "pt", pack)
    ok(u.ok and u.hir == h, "e a ida-e-volta fecha no idioma coberto")

    # o núcleo sozinho não conhece o símbolo de domínio
    core_only = B.Pack(copy.deepcopy(base))
    ok(core_only.render_form(sym, "pt") is None,
       "um aparelho só com o núcleo não conhece o símbolo de domínio")
    ok(core_only.render_gap(h, "pt") == sym,
       "e trata isso como lacuna com endereço, não como erro genérico")

    # ---- NEGATIVA -------------------------------------------------------
    bad = copy.deepcopy(ext)
    bad[1]["namespaces"] = {"EV": {"base": 1, "role": "O_QUE", "cap": 255}}
    ok(refuses(base + bad, "redefine o namespace"),
       "extensão que redefine namespace do núcleo é recusada")

    bad = copy.deepcopy(ext)
    bad[1]["namespaces"] = {"CAMPO": {"base": 600, "role": "O_QUE", "cap": 255}}
    ok(refuses(base + bad, "colide com"),
       "extensão cuja faixa invade outro namespace é recusada")

    bad = copy.deepcopy(ext)
    bad[0]["concepts"][0]["lex"]["pt"] = ["cheguei"]
    ok(refuses(base + bad, "COLISAO"),
       "extensão que colide forma com o núcleo é recusada")

    bad = copy.deepcopy(ext)
    bad[0]["concepts"][0]["gap"] = ["ja"]      # esquece it, de, zh
    ok(refuses(base + bad, "sem lacuna declarada"),
       "extensão que esquece de declarar a lacuna é recusada")

    bad = copy.deepcopy(ext)
    bad[0]["concepts"][1]["n"] = 0             # dois conceitos no mesmo símbolo
    ok(refuses(base + bad, "reivindicado por"),
       "duas coisas no mesmo símbolo é recusado")

    bad = copy.deepcopy(ext)
    bad[0]["concepts"][0]["lex"]["pt"] = ["Evacuar"]   # não dobrada
    ok(refuses(base + bad, "nao esta na forma dobrada"),
       "forma fora da forma canônica é recusada")

    print(f"LOOM EXTENSION: {PASS} pass, {FAIL} fail")
    return 1 if FAIL else 0


if __name__ == "__main__":
    raise SystemExit(main())
