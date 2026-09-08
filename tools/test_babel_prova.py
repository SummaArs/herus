#!/usr/bin/env python3
"""test_babel_prova.py — prova de fogo do Babel: invariante + busca adversarial.

O portao do Loom enumera. Enumeracao prova o tamanho da imaginacao de quem
escreveu o enumerador. Aqui declaramos o que NUNCA pode acontecer e deixamos o
arnes passar centenas de milhares de tentativas tentando fazer acontecer, com
semente fixa para que toda falha seja reproduzivel exatamente.

Os invariantes fortes sao os tres ultimos: ruido, classe protegida e autoridade.
Eles dizem que Babel pode RECUSAR mais, nunca ENTENDER diferente. A forma exata:

    para toda perturbacao p de uma frase valida t:
        compile(p(t)) ∈ { mesmo significado de t, recusa tipada }

e nunca um terceiro significado. E essa a diferenca entre um compilador e um
adivinhador.

A cadeia de confianca fecha em tres elos:
  1. este arquivo prova as propriedades sobre a referencia, com busca hostil
  2. test_babel_cross.py prova que o C e a referencia sao a MESMA funcao
  3. o portao do Loom prova que os dados sustentam as duas
"""

from __future__ import annotations

import pathlib
import random
import subprocess
import sys

HERE = pathlib.Path(__file__).resolve().parent
ROOT = HERE.parent
sys.path.insert(0, str(HERE))

import babel_ref as B      # noqa: E402
import loom                # noqa: E402
from prova import provar, executar   # noqa: E402

PACK = B.Pack.load(ROOT / "research" / "loom" / "core")
assert not PACK.problems, PACK.problems
TAGS = PACK.tags
SHAPES = loom._op_role_shapes(PACK)
BY_ROLE: dict[int, list[int]] = {}
for _c in PACK.concepts:
    BY_ROLE.setdefault(_c["role"], []).append(_c["sym"])


def random_hir(rnd: random.Random, escala: float):
    """Significado alcancavel aleatorio. A escala controla quantos papeis
    entram: casos pequenos primeiro, saturados no fim."""
    for _ in range(24):
        op, req, opt, var = rnd.choice(SHAPES)
        k = min(len(opt), int(round(escala * len(opt))))
        chosen = rnd.sample(opt, k) if k else []
        fillers = {}
        ok = True
        for r in list(req) + list(chosen):
            if r == var:
                continue
            pool = BY_ROLE.get(r)
            if not pool:
                ok = False
                break
            fillers[r] = rnd.choice(pool)
        if not ok:
            continue
        urg = rnd.choice([0, 0, 1, 2])
        h = loom._make(PACK, op, fillers, var, rnd.choice([0, 0, 1]), urg)
        if h is not None:
            return h
    return None


def _sentence(rnd, escala):
    h = random_hir(rnd, escala)
    tag = rnd.choice(TAGS)
    return h, tag, (B.render(h, tag, PACK) if h else None)


# ----------------------------------------------------------------- geradores

def g_roundtrip(rnd, escala):
    h, tag, text = _sentence(rnd, escala)
    return {"tag": tag, "wire": h.wire().hex() if h else None, "texto": text}


def g_pair(rnd, escala):
    h = random_hir(rnd, escala)
    a, b = rnd.sample(TAGS, 2)
    return {"a": a, "b": b, "wire": h.wire().hex() if h else None,
            "texto_a": B.render(h, a, PACK) if h else None,
            "texto_b": B.render(h, b, PACK) if h else None}


def _lexeme_bounds(text: str, tag: str) -> list[int]:
    """Offsets onde um lexema termina e o proximo comeca, na leitura canonica.

    Existe por causa de um invariante que estava ERRADO. A primeira versao
    inseria ruido em posicao aleatoria e exigia que o significado nao mudasse.
    Num idioma com espaco isso e verdade. Num idioma sem espaco e falso e nao
    ha conserto: enfiar uma particula no meio de "15" produz "1" e "5分", que
    e outro numero de verdade. Nao e ruido, e corrupcao de lexema — e nenhum
    compilador do mundo pode desfazer isso, porque a informacao se foi.

    Entao o invariante correto e sobre FRONTEIRA de lexema, e a corrupcao
    dentro do lexema tem o seu proprio invariante, mais fraco e honesto
    (i_corruption): pode virar outro significado, mas nunca um HIR invalido e
    nunca um significado que nao sobrevive a propria ida-e-volta.
    """
    lang = PACK.lang_by_tag[tag]
    f = B.fold(text)
    if f.status != B.OK:
        return []
    edges, _ = B._edges(f.text, lang, PACK.lex[tag], PACK.lengths[tag])
    paths, _ = B._paths(edges, len(f.text))
    if not paths:
        return []
    at = 0
    out = [0]
    for e in paths[0]:
        idx = f.text.find(e.form, at)
        if idx < 0:
            return []
        at = idx + len(e.form)
        out.append(at)
    return out


def g_noise(rnd, escala):
    """Frase valida + palavras de RUIDO declarado, so em fronteira de lexema."""
    h, tag, text = _sentence(rnd, escala)
    if text is None:
        return {"tag": tag, "texto": None, "wire": None, "sujo": None}
    stops = [e.form for e in PACK.lex[tag].values() if e.cls == B.LX_STOP]
    bounds = _lexeme_bounds(text, tag)
    if not stops or not bounds:
        return {"tag": tag, "texto": text, "wire": h.wire().hex(), "sujo": text}
    sujo = B.fold(text).text
    for _ in range(1 + int(escala * 4)):
        bounds = _lexeme_bounds(sujo, tag)
        if not bounds:
            break
        at = rnd.choice(bounds)
        w = rnd.choice(stops)
        lang = PACK.lang_by_tag[tag]
        if lang.spaced:
            sujo = (sujo[:at] + (" " if at else "") + w +
                    (" " if at < len(sujo) else "") + sujo[at:])
        else:
            sujo = sujo[:at] + w + sujo[at:]
        sujo = B.fold(sujo).text
        if len(sujo.encode()) > B.TEXT_MAX:
            break
    return {"tag": tag, "texto": text, "wire": h.wire().hex(), "sujo": sujo}


def g_corruption(rnd, escala):
    """Corrupcao DENTRO de um lexema: insercao em offset arbitrario."""
    h, tag, text = _sentence(rnd, escala)
    if text is None:
        return {"tag": tag, "sujo": None}
    f = B.fold(text).text
    at = rnd.randrange(len(f) + 1) if f else 0
    junk = rnd.choice(["x", "0", "9", "q", " ", "z"])
    return {"tag": tag, "sujo": f[:at] + junk + f[at:]}


def _poison(rnd, escala, cls):
    h, tag, text = _sentence(rnd, escala)
    words = [e.form for e in PACK.lex[tag].values() if e.cls == cls]
    if text is None or not words:
        return {"tag": tag, "sujo": None}
    lang = PACK.lang_by_tag[tag]
    w = rnd.choice(words)
    if lang.spaced:
        parts = text.split()
        parts.insert(rnd.randrange(len(parts) + 1), w)
        sujo = " ".join(parts)
    else:
        at = rnd.randrange(len(text) + 1)
        sujo = text[:at] + w + text[at:]
    return {"tag": tag, "sujo": sujo, "palavra": w}


def g_sensitive(rnd, escala):
    return _poison(rnd, escala, B.LX_SENS)


def g_authority(rnd, escala):
    return _poison(rnd, escala, B.LX_AUTH)


def g_garbage(rnd, escala):
    """Lixo estruturado: pedacos de formas validas costurados com bytes soltos.
    E o pior caso para um casador guloso, e portanto o caso que interessa."""
    tag = rnd.choice(TAGS)
    forms = [e.form for e in PACK.lex[tag].values()]
    n = 1 + int(escala * 10)
    out = []
    for _ in range(n):
        f = rnd.choice(forms)
        mode = rnd.randrange(4)
        if mode == 0:
            out.append(f)
        elif mode == 1:
            out.append(f[: max(1, len(f) // 2)])
        elif mode == 2:
            out.append(f + rnd.choice("xyzq0"))
        else:
            out.append(rnd.choice("abcdefghij") * rnd.randrange(1, 4))
    join = " " if PACK.lang_by_tag[tag].spaced else ""
    return {"tag": tag, "texto": join.join(out)[:B.TEXT_MAX]}


def g_bytes(rnd, escala):
    tag = rnd.choice(TAGS)
    n = 1 + int(escala * 60)
    raw = bytes(rnd.randrange(256) for _ in range(n))
    return {"tag": tag, "raw": raw.hex()}


def g_fold(rnd, escala):
    alfabeto = "aeiouAEIOU cheguei CASA 123 .,!?-'ÁÉÍÓÚàéîõüç着いた到了　。"
    n = 1 + int(escala * 60)
    return {"texto": "".join(rnd.choice(alfabeto) for _ in range(n))}


# --------------------------------------------------------------- invariantes

def i_roundtrip(caso):
    if caso["wire"] is None:
        return True
    u = B.compile(caso["texto"], caso["tag"], PACK)
    return u.ok and u.hir.wire().hex() == caso["wire"]


def i_pair(caso):
    if caso["wire"] is None:
        return True
    ua = B.compile(caso["texto_a"], caso["a"], PACK)
    ub = B.compile(caso["texto_b"], caso["b"], PACK)
    return (ua.ok and ub.ok
            and ua.hir.wire().hex() == caso["wire"]
            and ub.hir.wire().hex() == caso["wire"])


def i_noise(caso):
    """Ruido pode fazer RECUSAR. Nunca pode fazer entender OUTRA coisa."""
    if caso["sujo"] is None:
        return True
    u = B.compile(caso["sujo"], caso["tag"], PACK)
    if not u.ok:
        return True
    return u.hir.wire().hex() == caso["wire"]


def i_sensitive(caso):
    if caso["sujo"] is None:
        return True
    return B.compile(caso["sujo"], caso["tag"], PACK).status == B.E_SENSITIVE


def i_authority(caso):
    if caso["sujo"] is None:
        return True
    return B.compile(caso["sujo"], caso["tag"], PACK).status == B.E_AUTHORITY


def i_corruption(caso):
    """Corromper um lexema PODE virar outro significado — a informacao original
    se foi. O que nunca pode e virar um HIR invalido ou um significado que nao
    sobrevive a propria ida-e-volta."""
    if caso["sujo"] is None:
        return True
    u = B.compile(caso["sujo"], caso["tag"], PACK)
    if not u.ok:
        return True
    if B.hir_validate(u.hir) != "OK":
        return False
    back = B.compile(B.render(u.hir, caso["tag"], PACK), caso["tag"], PACK)
    return back.ok and back.hir == u.hir


def i_never_guess(caso):
    """Ou recusa tipada, ou um significado que sobrevive a propria ida-e-volta.
    Nao existe saida 'quase certa'."""
    u = B.compile(caso["texto"], caso["tag"], PACK)
    if not u.ok:
        return u.status in (B.E_GAP, B.E_AMBIGUOUS, B.E_INCOMPLETE, B.E_EMPTY,
                            B.E_OVERFLOW, B.E_SENSITIVE, B.E_AUTHORITY, B.E_BYTE)
    back = B.compile(B.render(u.hir, caso["tag"], PACK), caso["tag"], PACK)
    return back.ok and back.hir == u.hir


def i_bytes(caso):
    raw = bytes.fromhex(caso["raw"])
    u = B.compile(raw, caso["tag"], PACK)
    if not u.ok:
        return True
    return B.hir_validate(u.hir) == "OK" and len(u.hir.wire()) == 24


def i_determinism(caso):
    if caso["wire"] is None:
        return True
    a = B.compile(caso["texto"], caso["tag"], PACK)
    b = B.compile(caso["texto"], caso["tag"], PACK)
    if a.status != b.status:
        return False
    return (a.hir == b.hir) if a.ok else True


def i_envelope(caso):
    if caso["wire"] is None:
        return True
    u = B.compile(caso["texto"], caso["tag"], PACK)
    if not u.ok:
        return False
    w = u.hir.wire()
    return len(w) == 24 and w.hex() == caso["wire"]


def i_fold(caso):
    a = B.fold(caso["texto"])
    if a.status != B.OK:
        return True
    b = B.fold(a.text)
    if b.status != B.OK or b.text != a.text:
        return False
    return "  " not in a.text and not a.text.endswith(" ")


# ---------------------------------------------------------- amostra cruzada

def cross_sample(n=20000, semente=7):
    """Fecha a cadeia sobre entrada ALEATORIA, nao apenas enumerada: n casos
    sorteados vao para o C real e as duas respostas tem de bater."""
    probe = ROOT / "firmware" / "build" / "babel_probe"
    if not probe.exists():
        return None, "sonda ausente"
    reqs, expect = [], []
    for i in range(n):
        rnd = random.Random((semente << 24) ^ i)
        escala = (i % 100 + 1) / 100.0
        pick = i % 3
        if pick == 0:
            c = g_roundtrip(rnd, escala)
            if c["texto"] is None:
                continue
            texto, tag = c["texto"], c["tag"]
        elif pick == 1:
            c = g_garbage(rnd, escala)
            texto, tag = c["texto"], c["tag"]
        else:
            c = g_noise(rnd, escala)
            if c["sujo"] is None:
                continue
            texto, tag = c["sujo"], c["tag"]
        if not texto or "\t" in texto or "\n" in texto:
            continue
        u = B.compile(texto, tag, PACK)
        expect.append(f"OK {u.hir.digest().hex()} {u.hir.wire().hex()}" if u.ok
                      else u.status)
        reqs.append(f"C\t{tag}\t{texto}")
    p = subprocess.Popen([str(probe)], stdin=subprocess.PIPE,
                         stdout=subprocess.PIPE)
    out, _ = p.communicate("".join(r + "\n" for r in reqs).encode("utf-8"))
    got = out.decode("utf-8").splitlines()
    bad = [(r, g, e) for r, g, e in zip(reqs, got, expect) if g != e]
    return (len(reqs), bad)


def main() -> int:
    relatorios = [
        provar("ida-e-volta: compile(render(h,L),L) == h",
               g_roundtrip, i_roundtrip, 60_000, semente=1),
        provar("convergencia: dois idiomas, um significado",
               g_pair, i_pair, 40_000, semente=2),
        provar("ruido em fronteira de lexema nunca muda o significado",
               g_noise, i_noise, 40_000, semente=3),
        provar("corrupcao dentro do lexema falha fechado, nunca HIR invalido",
               g_corruption, i_corruption, 30_000, semente=11),
        provar("classe protegida e inescapavel em qualquer posicao",
               g_sensitive, i_sensitive, 20_000, semente=4),
        provar("autoridade nao se lava em qualquer posicao",
               g_authority, i_authority, 20_000, semente=5),
        provar("nunca adivinha: recusa tipada ou significado estavel",
               g_garbage, i_never_guess, 40_000, semente=6),
        provar("byte arbitrario nunca produz HIR invalido",
               g_bytes, i_bytes, 20_000, semente=7),
        provar("compilacao e deterministica e sem estado",
               g_roundtrip, i_determinism, 20_000, semente=8),
        provar("todo significado cabe em exatamente 24 bytes",
               g_roundtrip, i_envelope, 20_000, semente=9),
        provar("dobramento e idempotente e sem espaco solto",
               g_fold, i_fold, 20_000, semente=10),
    ]
    code = executar(relatorios, "BABEL — PROVA DE FOGO")

    n, bad = cross_sample()
    if n is None:
        print(f"\n[AVISO] amostra cruzada pulada: {bad}")
    elif bad:
        print(f"\n[FALHA] amostra cruzada: {len(bad)} de {n:,} divergiram"
              .replace(",", "."))
        for r, g, e in bad[:5]:
            print(f"   {r!r}\n     C  {g}\n     PY {e}")
        code = 1
    else:
        print(f"\n[OK  ] amostra cruzada no C real: {n:,} casos aleatorios, "
              f"0 divergencias".replace(",", "."))

    total = sum(r["tentativas"] for r in relatorios) + (n or 0)
    print(f"\nBABEL PROVA: {'PASS' if code == 0 else 'FAIL'} "
          f"{len(relatorios)} invariantes, {total:,} tentativas".replace(",", "."))
    return code


if __name__ == "__main__":
    raise SystemExit(main())
