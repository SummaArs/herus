#!/usr/bin/env python3
"""loom.py — o tear: uma especificacao de significado, quatro artefatos.

O Loom existe porque a escala do HERUS deixou de ser um problema de codigo e
passou a ser um problema de dados. Um lexico escrito a mao em C serve para um
idioma e trava em dois. Aqui a fonte da verdade e um pacote .hlx.json e tudo o
que consome significado e GERADO dele:

    research/loom/<pacote>/*.hlx.json
              |
              +-- firmware/core/loom_<pacote>.{h,c}   tabelas C11 do aparelho
              +-- research/loom/<pacote>.compiled.json pacote do navegador
              +-- (tools/babel_ref.py le o pacote direto — nao precisa gerar)

E o Loom nao e so um gerador: e um PORTAO. Um pacote so compila se provar, para
os proprios dados, que:

  L1  nenhuma forma de superficie tem dois significados no mesmo idioma
  L2  toda forma esta na forma dobrada canonica (nada de acento perdido depois)
  L3  todo simbolo e unico e cabe no seu namespace de 11 bits
  L4  todo conceito existe em todo idioma declarado, ou declara a lacuna
  L5  IDA-E-VOLTA: para todo significado alcancavel h e todo idioma L,
         compile(render(h, L), L) == h
  L6  CONVERGENCIA TRANSLINGUE: renderizar em L1 e em L2 e compilar cada um
      devolve o MESMO h, byte a byte
  L7  os conceitos congelados guardam exatamente as formas pt do herald.c
  L8  o corpus HSCA v1 congelado sai identico — status e digest
  L9  nenhuma frase renderizada estoura o orcamento de leituras (PATH_MAX)
  L10 as tabelas cabem no envelope declarado

L5 e o teorema. Todo o resto existe para que L5 possa ser verdade.

Uso:
    python3 tools/loom.py --check              portao apenas
    python3 tools/loom.py --emit               portao + gera os artefatos
    python3 tools/loom.py --pack research/loom/outro --emit
"""

from __future__ import annotations

import argparse
import hashlib
import itertools
import json
import pathlib
import re
import sys

HERE = pathlib.Path(__file__).resolve().parent
ROOT = HERE.parent
sys.path.insert(0, str(HERE))

import babel_ref as B   # noqa: E402

SEG_MAX = 12
ENVELOPE = {
    "entries": 4096,          # formas de superficie somadas em todos os idiomas
    "table_bytes": 262144,    # bytes de string das tabelas geradas
    "form_bytes": 48,         # uma forma nunca passa disso
}


# --------------------------------------------------------------- enumeradores

def _op_role_shapes(pack: B.Pack):
    """As formas estruturais que o compilador pode produzir, por operacao.

    Devolve (op, papeis_obrigatorios, papeis_opcionais, var_role_ou_None).
    Isto e uma leitura do type check do compilador, nao um palpite: se o
    compilador mudar e isto nao, L5 quebra e o portao acusa.
    """
    R = B
    shapes = []
    opt_all = [R.ROLE_QUANDO, R.ROLE_ONDE, R.ROLE_QUANTO, R.ROLE_ESTADO]
    shapes.append((R.OP_COMUNICAR, [R.ROLE_QUEM, R.ROLE_O_QUE], opt_all, None))
    shapes.append((R.OP_LEMBRAR, [R.ROLE_O_QUE],
                   [R.ROLE_QUANDO, R.ROLE_ONDE, R.ROLE_QUANTO, R.ROLE_ESTADO,
                    R.ROLE_QUEM], None))
    shapes.append((R.OP_PLANEJAR, [R.ROLE_O_QUE],
                   [R.ROLE_QUANDO, R.ROLE_ONDE, R.ROLE_QUANTO, R.ROLE_ESTADO,
                    R.ROLE_QUEM], None))
    shapes.append((R.OP_CONFIRMAR, [R.ROLE_O_QUE], opt_all + [R.ROLE_QUEM], None))
    shapes.append((R.OP_CANCELAR, [R.ROLE_O_QUE], opt_all + [R.ROLE_QUEM], None))
    shapes.append((R.OP_SOCORRO, [R.ROLE_QUEM], [R.ROLE_O_QUE] + opt_all, None))
    for var in (R.ROLE_ONDE, R.ROLE_QUANDO, R.ROLE_QUEM, R.ROLE_QUANTO):
        others = [r for r in [R.ROLE_QUEM, R.ROLE_O_QUE, R.ROLE_QUANDO,
                              R.ROLE_ONDE, R.ROLE_QUANTO, R.ROLE_ESTADO]
                  if r != var]
        shapes.append((R.OP_PERGUNTAR, [var], others, var))
    return shapes


def _make(pack, op, fillers, var_role, polarity, urgency):
    h = B.Hir()
    h.op = op
    h.polarity = polarity
    h.urgency = urgency
    for role, sym in fillers.items():
        h.put(role, sym)
    if var_role is not None:
        h.put(var_role, B.HIR_FILLER_VAR)
    if op == B.OP_SOCORRO:
        h.urgency = B.URG_SOCORRO
        h.put(B.ROLE_QUEM, pack.sym_of["PER.TODOS"])
    if op == B.OP_LEMBRAR:
        h.persistence = B.PERSIST_LEMBRAR
    # As pos-condicoes do proprio compilador. Um significado que nao passa por
    # elas nao e alcancavel, e cobrar ida-e-volta dele seria cobrar do render
    # uma frase que nenhuma pessoa poderia ter dito.
    if op == B.OP_PERGUNTAR and len(h.slots) < 2:
        return None
    return h if B.hir_validate(h) == "OK" else None


def enumerate_singles(pack: B.Pack):
    """Cobertura exaustiva de SIMBOLO: todo conceito, em todo papel legal, em
    toda operacao que o aceita. Garante que nenhum simbolo do pacote fica sem
    renderizador nem sem forma de volta em nenhum idioma."""
    by_role = {}
    for c in pack.concepts:
        by_role.setdefault(c["role"], []).append(c["sym"])
    seen = set()
    for op, req, opt, var in _op_role_shapes(pack):
        # preenche os obrigatorios com o primeiro simbolo e varia UM papel
        for target in set(req) | set(opt):
            if target == var:
                continue
            for sym in by_role.get(target, []):
                fillers = {}
                ok = True
                for r in req:
                    if r == var:
                        continue
                    pick = sym if r == target else (by_role.get(r) or [None])[0]
                    if pick is None:
                        ok = False
                        break
                    fillers[r] = pick
                if not ok:
                    continue
                if target not in req:
                    fillers[target] = sym
                h = _make(pack, op, fillers, var, 0, 0)
                if h is not None and h.key() not in seen:
                    seen.add(h.key())
                    yield h


def enumerate_shapes(pack: B.Pack):
    """Cobertura exaustiva de ESTRUTURA: toda combinacao de papeis presentes,
    toda polaridade, toda urgencia — com o primeiro simbolo de cada namespace.
    Garante que nenhum template tem um segmento que se perde ou cola errado."""
    first = {}
    for c in pack.concepts:
        first.setdefault(c["role"], c["sym"])
    seen = set()
    for op, req, opt, var in _op_role_shapes(pack):
        for k in range(len(opt) + 1):
            for combo in itertools.combinations(opt, k):
                for polarity in (0, 1):
                    for urgency in (0, 1, 2):
                        fillers = {}
                        ok = True
                        for r in list(req) + list(combo):
                            if r == var:
                                continue
                            if r not in first:
                                ok = False
                                break
                            fillers[r] = first[r]
                        if not ok:
                            continue
                        h = _make(pack, op, fillers, var, polarity, urgency)
                        if h is not None and h.key() not in seen:
                            seen.add(h.key())
                            yield h


def _minimal_ops(ra, rb):
    """Operacoes que aceitam este par de papeis. A primeira e a mais permissiva
    possivel (exige o menos possivel), para que os papeis nao usados no par
    fiquem realmente vazios; a ultima e sempre COMUNICAR, a forma em que o par
    aparece numa mensagem real."""
    pair = {ra, rb}
    out = []
    if pair & {B.ROLE_O_QUE, B.ROLE_ONDE, B.ROLE_QUANDO}:
        out.append(B.OP_LEMBRAR)
    if B.ROLE_O_QUE in pair or B.ROLE_ONDE in pair:
        out.append(B.OP_PLANEJAR)
    if B.ROLE_O_QUE in pair:
        out.append(B.OP_CONFIRMAR)
    out.append(B.OP_SOCORRO)
    out.append(B.OP_COMUNICAR)
    return out


def enumerate_pairs(pack: B.Pack):
    """Cobertura exaustiva de PAR: todo par de papeis, toda combinacao de
    preenchimentos.

    A primeira versao cobria so QUEM x O_QUE e por isso deixou passar dois
    defeitos que a busca adversarial depois encontrou: em japones a forma
    "少し待って" de ESPERA continha a forma "少し" de POUCO, e em ingles a forma
    "not alone" de ACOMPANHADO colidia com negacao + SOZINHO. Os dois sao
    colisoes de PAR, e nenhum aparece variando um papel por vez.

    Enumerar todos os 15 pares custa ~62 mil idas-e-voltas e fecha a classe
    inteira. O criterio e simples: se a busca adversarial acha uma classe de
    defeito, a classe vira enumeracao — a busca fica livre para procurar a
    proxima em vez de reencontrar a mesma.
    """
    roles = [B.ROLE_QUEM, B.ROLE_O_QUE, B.ROLE_QUANDO, B.ROLE_ONDE,
             B.ROLE_QUANTO, B.ROLE_ESTADO]
    by_role = {r: [c["sym"] for c in pack.concepts if c["role"] == r]
               for r in roles}
    first = {r: (by_role[r][0] if by_role[r] else None) for r in roles}
    seen = set()
    for i, ra in enumerate(roles):
        for rb in roles[i + 1:]:
            for a in by_role[ra]:
                for b in by_role[rb]:
                    # Duas variantes do MESMO par, e as duas importam.
                    #
                    # (1) operacao MINIMA: a que aceita exatamente este par e
                    #     deixa todos os outros papeis VAZIOS. Um papel vazio e
                    #     um slot livre, e um slot livre e o que permite a
                    #     leitura rival existir. Foi assim que passou
                    #     "all good"+"morning" x "all"+"good morning": com QUEM
                    #     ja preenchido, a leitura rival colidia no papel e
                    #     morria; com QUEM livre, ela vive e sao dois
                    #     significados.
                    # (2) COMUNICAR completando o que falta, que e a forma em
                    #     que o par realmente aparece numa mensagem de verdade.
                    for op in _minimal_ops(ra, rb):
                        fillers = {ra: a, rb: b}
                        if op == B.OP_COMUNICAR:
                            ok = True
                            for need in (B.ROLE_QUEM, B.ROLE_O_QUE):
                                if need not in fillers:
                                    if first[need] is None:
                                        ok = False
                                        break
                                    fillers[need] = first[need]
                            if not ok:
                                continue
                        pols = (0, 1) if (ra, rb) == (B.ROLE_QUEM, B.ROLE_O_QUE) else (0,)
                        for polarity in pols:
                            h = _make(pack, op, fillers, None, polarity, 0)
                            if h is not None and h.key() not in seen:
                                seen.add(h.key())
                                yield h


# --------------------------------------------------------------------- portao

class Gate:
    def __init__(self, pack: B.Pack, pack_dir: pathlib.Path):
        self.pack = pack
        self.dir = pack_dir
        self.failures: list[str] = []
        self.notes: list[str] = []
        self.counts: dict[str, int] = {}

    def fail(self, tag, msg):
        self.failures.append(f"{tag}: {msg}")

    # ---- L1..L4 vem do proprio carregamento do pacote -------------------
    def l1_l4_load(self):
        for p in self.pack.problems:
            self.fail("L1-L4", p)
        self.counts["conceitos"] = len(self.pack.concepts)
        self.counts["idiomas"] = len(self.pack.langs)
        self.counts["formas"] = sum(len(v) for v in self.pack.lex.values())

    # ---- L5 / L6 / L9 ----------------------------------------------------
    def l5_roundtrip(self):
        pack = self.pack
        tags = pack.tags
        total = 0
        bad = 0
        for name, gen in (("simbolos", enumerate_singles),
                          ("estruturas", enumerate_shapes),
                          ("pares", enumerate_pairs)):
            n = 0
            for h in gen(pack):
                n += 1
                seen: dict[str, B.Hir] = {}
                for tag in tags:
                    # L11: um significado que contem simbolo sem forma neste
                    # idioma tem de virar LACUNA DECLARADA, nao ida-e-volta
                    # quebrada e nao aproximacao. E a lacuna tem de estar
                    # declarada no pacote: forma faltando que ninguem declarou
                    # e defeito de dado, nao decisao de produto.
                    gap = pack.render_gap(h, tag)
                    if gap is not None:
                        if tag not in pack.gap_langs(gap):
                            self.fail("L11", f"[{tag}] simbolo {gap} sem forma e "
                                             f"sem lacuna declarada em {h!r}")
                            bad += 1
                        elif B.render(h, tag, pack) is not None:
                            self.fail("L11", f"[{tag}] simbolo {gap} tem lacuna "
                                             f"declarada mas o render devolveu texto")
                            bad += 1
                        else:
                            self.counts["lacunas"] = self.counts.get("lacunas", 0) + 1
                        continue
                    text = B.render(h, tag, pack)
                    if text is None:
                        self.fail("L5", f"[{tag}] {h!r} nao renderiza")
                        bad += 1
                        continue
                    u = B.compile(text, tag, pack)
                    if not u.ok:
                        self.fail("L5", f"[{tag}] {h!r} -> {text!r} -> {u.status}")
                        bad += 1
                        continue
                    if u.hir != h:
                        self.fail("L5", f"[{tag}] {h!r} -> {text!r} -> {u.hir!r}")
                        bad += 1
                        continue
                    if u.paths >= B.PATH_MAX:
                        self.fail("L9", f"[{tag}] {text!r} tem {u.paths} leituras")
                        bad += 1
                    seen[tag] = u.hir
                    total += 1
                    if bad > 40:
                        return
                # L6: todos os idiomas convergiram no MESMO significado
                distinct = {v.key() for v in seen.values()}
                if len(distinct) > 1:
                    self.fail("L6", f"{h!r} divergiu entre idiomas: "
                                    f"{ {t: repr(v) for t, v in seen.items()} }")
                    bad += 1
            self.counts[f"significados_{name}"] = n
        self.counts["idas_e_voltas"] = total

    # ---- L7: os congelados nao andaram ----------------------------------
    def l7_frozen(self):
        src = (ROOT / "firmware" / "core" / "herald.c").read_text(encoding="utf-8")
        legacy: dict[str, set[str]] = {}
        rows = re.findall(r'\{"([^"]+)",\s*LX_FILL,\s*HIR_ROLE_(\w+),\s*(\w+)\}', src)
        for form, _role, symname in rows:
            legacy.setdefault(symname, set()).add(form)
        # mapa nome-do-macro -> id do conceito no pacote
        alias = {"EV_": "EV.", "PER_": "PER.", "LOC_": "LOC.", "T_": "TIME.",
                 "ST_": "ST."}
        rename = {"EV.TUDO_BEM": "EV.TUDO_BEM", "TIME.MIN_5": "TIME.MIN_5"}
        for symname, forms in legacy.items():
            full = None
            for prefix, ns in alias.items():
                if symname.startswith(prefix):
                    full = ns + symname[len(prefix):]
                    break
            if full is None:
                continue
            full = rename.get(full, full)
            c = None
            for cand in self.pack.concepts:
                if cand["full"] == full:
                    c = cand
                    break
            if c is None:
                self.fail("L7", f"herald.c tem {symname} ({full}) e o pacote nao")
                continue
            if not c.get("frozen"):
                self.fail("L7", f"{full} vem do herald.c e nao esta marcado frozen")
            have = set(c["lex"]["pt"])
            if have != forms:
                self.fail("L7", f"{full} pt: pacote={sorted(have)} "
                                f"herald.c={sorted(forms)}")
        self.counts["congelados"] = len(legacy)

    # ---- L8: o corpus congelado -----------------------------------------
    def l8_corpus(self):
        path = ROOT / "research" / "hsca_intent_corpus_v1.json"
        if not path.exists():
            self.notes.append("L8: corpus ausente, pulado")
            return
        corpus = json.loads(path.read_text(encoding="utf-8"))
        dpath = self.dir / "frozen_deltas.json"
        declared = {}
        if dpath.exists():
            for d in json.loads(dpath.read_text(encoding="utf-8"))["deltas"]:
                declared[d["text"]] = d
        used = set()
        ok = 0
        for row in corpus["rows"]:
            u = B.compile(row["text"], "pt", self.pack)
            if u.ok:
                got_status, got_digest = "OK", u.hir.digest().hex()
            else:
                got_status, got_digest = u.status, None
            if got_status != row["status"]:
                d = declared.get(row["text"])
                if d is None:
                    self.fail("L8", f"{row['text']!r}: corpus={row['status']} "
                                    f"babel={got_status} — divergencia NAO declarada")
                elif d["was"] != row["status"] or d["now"] != got_status:
                    self.fail("L8", f"{row['text']!r}: delta declarado "
                                    f"{d['was']}->{d['now']}, observado "
                                    f"{row['status']}->{got_status}")
                else:
                    used.add(row["text"])
                    ok += 1
                continue
            if row["status"] == "OK" and got_digest != row["digest"]:
                self.fail("L8", f"{row['text']!r}: digest corpus={row['digest']} "
                                f"babel={got_digest}")
                continue
            ok += 1
        for text in declared:
            if text not in used:
                self.fail("L8", f"{text!r}: delta declarado que nao acontece mais "
                                f"— entrada morta, remova o argumento")
        self.counts["corpus"] = ok
        self.counts["corpus_total"] = len(corpus["rows"])
        self.counts["corpus_deltas"] = len(used)

    # ---- L10: envelope ---------------------------------------------------
    def l10_envelope(self):
        entries = self.counts.get("formas", 0)
        if entries > ENVELOPE["entries"]:
            self.fail("L10", f"{entries} formas > {ENVELOPE['entries']}")
        for tag, lex in self.pack.lex.items():
            for form in lex:
                if len(form.encode()) > ENVELOPE["form_bytes"]:
                    self.fail("L10", f"[{tag}] forma longa demais: {form!r}")
        for tag, table in self.pack.templates.items():
            for op, segs in table.items():
                if len(segs) > SEG_MAX:
                    self.fail("L10", f"[{tag}] template {B.OP_NAME[op]} tem "
                                     f"{len(segs)} segmentos > {SEG_MAX}")

    def run(self):
        self.l1_l4_load()
        if not self.failures:
            self.l5_roundtrip()
        self.l7_frozen()
        self.l8_corpus()
        self.l10_envelope()
        return not self.failures


# ------------------------------------------------------------------- geracao

def _c_string(s: str) -> str:
    out = []
    for b in s.encode("utf-8"):
        if b == 0x22:
            out.append('\\"')
        elif b == 0x5C:
            out.append("\\\\")
        elif 0x20 <= b < 0x7F:
            out.append(chr(b))
        else:
            out.append(f"\\{b:03o}")
    return '"' + "".join(out) + '"'


def emit_c(pack: B.Pack, name: str, digest: str) -> tuple[str, str]:
    langs = pack.langs
    ordered: list[tuple[int, B.Entry]] = []
    lang_first, lang_count, max_form = [], [], []
    buckets: list[list[tuple[int, int]]] = []
    for l in langs:
        lex = pack.lex[l.tag]
        # Ordem: primeiro byte, depois forma mais longa. O casamento no pulso
        # nao pode varrer o lexico inteiro por posicao: ele consulta o balde do
        # byte atual e ja o encontra em ordem de maior para menor.
        rows = sorted(lex.values(),
                      key=lambda e: (e.form.encode()[0], -len(e.form.encode()), e.form))
        lang_first.append(len(ordered))
        lang_count.append(len(rows))
        max_form.append(max((len(e.form.encode()) for e in rows), default=0))
        base = len(ordered)
        table = [(0, 0)] * 256
        for e in rows:
            ordered.append((l.index, e))
        for b in range(256):
            first = None
            count = 0
            for k, e in enumerate(rows):
                if e.form.encode()[0] == b:
                    if first is None:
                        first = base + k
                    count += 1
            table[b] = (first if first is not None else 0, count)
        buckets.append(table)

    guard = f"HERUS_LOOM_{name.upper()}_H"
    h = []
    h.append(f"""/* loom_{name}.h — GERADO por tools/loom.py. NAO EDITE.
 *
 * Fonte: research/loom/{name}/  (todos os .hlx.json)
 * Digest da fonte: {digest}
 *
 * Editar este arquivo a mao desliga o portao do Loom: as garantias L1-L10
 * passam a valer para um pacote que nao existe mais. Mexa na especificacao e
 * rode `python3 tools/loom.py --emit`.
 */
#ifndef {guard}
#define {guard}

#include <stdint.h>

#define LOOM_PACK_NAME      {_c_string(name)}
#define LOOM_PACK_VERSION   {pack.grammar.get('version', 1)}
#define LOOM_PACK_DIGEST    {_c_string(digest)}
#define LOOM_LANG_COUNT     {len(langs)}u
#define LOOM_ENTRY_COUNT    {len(ordered)}u
#define LOOM_SYM_COUNT      {len(pack.concepts)}u
#define LOOM_OP_COUNT       8u
#define LOOM_ROLE_COUNT     7u
#define LOOM_SEG_MAX        {SEG_MAX}u
#define LOOM_MAX_FORM_BYTES {max(max_form)}u

/* Classes lexicais. Mesma numeracao de tools/babel_ref.py; a suite diferencial
 * falha se uma das duas mudar sozinha. */
enum {{
    LOOM_STOP = 0, LOOM_OP = 1, LOOM_FILL = 2, LOOM_NEG = 3,
    LOOM_URG = 4, LOOM_QVAR = 5, LOOM_SENS = 6, LOOM_AUTH = 7,
    /* Negacao LEXICAL: a forma carrega o preenchimento E a polaridade. Em
     * japones e chines a negacao e morfologica; um marcador solto produz frase
     * que ninguem escreveria. O dado fornece a forma, o codigo segue fechado. */
    LOOM_FILLNEG = 8
}};

typedef struct {{
    const char *form;   /* forma dobrada, UTF-8, sem NUL interno */
    uint8_t     len;    /* bytes, nao pontos de codigo */
    uint8_t     cls;
    uint8_t     role;
    uint16_t    val;    /* simbolo, codigo de operacao ou nivel de urgencia */
}} loom_entry_t;

typedef struct {{
    const char *tag;
    const char *endonym;
    uint8_t     spaced;      /* 1 = fronteira por espaco, 0 = por escrita */
    uint8_t     rtl;
    uint16_t    first;       /* indice em LOOM_ENTRY */
    uint16_t    count;
    uint16_t    max_form;    /* maior forma deste idioma, em bytes */
    uint16_t    speakers_m;
}} loom_lang_t;

typedef struct {{
    uint16_t    sym;
    uint8_t     role;
    const char *render[LOOM_LANG_COUNT];
    const char *neg[LOOM_LANG_COUNT];   /* NULL onde o marcador basta */
}} loom_sym_t;

typedef struct {{
    uint8_t     n;
    const char *seg[LOOM_SEG_MAX];
}} loom_tmpl_t;

extern const loom_entry_t LOOM_ENTRY[LOOM_ENTRY_COUNT];
extern const loom_lang_t  LOOM_LANG[LOOM_LANG_COUNT];
extern const loom_sym_t   LOOM_SYM[LOOM_SYM_COUNT];
extern const loom_tmpl_t  LOOM_TMPL[LOOM_LANG_COUNT][LOOM_OP_COUNT];
extern const char *const  LOOM_OP_RENDER[LOOM_LANG_COUNT][LOOM_OP_COUNT];
extern const char *const  LOOM_NEG_RENDER[LOOM_LANG_COUNT];
extern const char *const  LOOM_URG_RENDER[LOOM_LANG_COUNT][4];
extern const char *const  LOOM_QW_RENDER[LOOM_LANG_COUNT][LOOM_ROLE_COUNT];

/* Balde por primeiro byte: LOOM_BUCKET[idioma][byte] = {{indice, quantas}}, e as
 * entradas do balde ja vem da mais longa para a mais curta. */
typedef struct {{ uint16_t first; uint16_t count; }} loom_bucket_t;
extern const loom_bucket_t LOOM_BUCKET[LOOM_LANG_COUNT][256];

/* Indice do idioma pela etiqueta BCP-47 curta, ou -1. */
int loom_lang_index(const char *tag);
/* Forma de renderizacao de um simbolo naquele idioma, ou NULL. */
const char *loom_sym_render(uint16_t sym, uint8_t lang);
/* Forma NEGATIVA lexical, ou NULL quando o idioma nega por marcador. */
const char *loom_sym_neg(uint16_t sym, uint8_t lang);

#endif /* {guard} */
""")

    c = []
    c.append(f"""/* loom_{name}.c — GERADO por tools/loom.py. NAO EDITE.
 * Fonte: research/loom/{name}/  (todos os .hlx.json)   digest {digest}
 */
#include "loom_{name}.h"
#include <string.h>

const loom_entry_t LOOM_ENTRY[LOOM_ENTRY_COUNT] = {{""")
    for lang_index, e in ordered:
        c.append(f"    {{ {_c_string(e.form)}, {len(e.form.encode())}, "
                 f"{e.cls}, {e.role}, {e.val} }},"
                 f"  /* {pack.tags[lang_index]} {e.origin} */")
    c.append("};\n")

    c.append(f"const loom_lang_t LOOM_LANG[LOOM_LANG_COUNT] = {{")
    for i, l in enumerate(langs):
        c.append(f"    {{ {_c_string(l.tag)}, {_c_string(l.endonym)}, "
                 f"{1 if l.spaced else 0}, {1 if l.dir == 'rtl' else 0}, "
                 f"{lang_first[i]}, {lang_count[i]}, {max_form[i]}, "
                 f"{l.speakers_m} }},")
    c.append("};\n")

    c.append(f"const loom_sym_t LOOM_SYM[LOOM_SYM_COUNT] = {{")
    for cc in pack.concepts:
        renders = ", ".join(_c_string(cc["lex"][t][0]) if cc["lex"].get(t) else "NULL"
                            for t in pack.tags)
        negs = ", ".join(_c_string((cc.get("neg") or {})[t])
                         if (cc.get("neg") or {}).get(t) else "NULL"
                         for t in pack.tags)
        c.append(f"    {{ {cc['sym']}, {cc['role']}, {{ {renders} }}, "
                 f"{{ {negs} }} }},  /* {cc['full']} */")
    c.append("};\n")

    c.append("const loom_tmpl_t LOOM_TMPL[LOOM_LANG_COUNT][LOOM_OP_COUNT] = {")
    for l in langs:
        c.append(f"    {{ /* {l.tag} */")
        for op in range(8):
            segs = pack.templates[l.tag].get(op)
            if not segs:
                c.append("        { 0, { 0 } },")
                continue
            body = ", ".join(_c_string(s) for s in segs)
            c.append(f"        {{ {len(segs)}, {{ {body} }} }},")
        c.append("    },")
    c.append("};\n")

    c.append("const char *const LOOM_OP_RENDER[LOOM_LANG_COUNT][LOOM_OP_COUNT] = {")
    for l in langs:
        row = ", ".join(_c_string(pack.op_render[l.tag][op])
                        if pack.op_render.get(l.tag, {}).get(op) else "NULL"
                        for op in range(8))
        c.append(f"    {{ {row} }},  /* {l.tag} */")
    c.append("};\n")

    c.append("const char *const LOOM_NEG_RENDER[LOOM_LANG_COUNT] = {")
    c.append("    " + ", ".join(_c_string(pack.neg_render.get(l.tag, ""))
                                 for l in langs))
    c.append("};\n")

    c.append("const char *const LOOM_URG_RENDER[LOOM_LANG_COUNT][4] = {")
    for l in langs:
        row = ", ".join(_c_string(pack.urg_render.get(l.tag, {}).get(k, ""))
                        for k in range(4))
        c.append(f"    {{ {row} }},  /* {l.tag} */")
    c.append("};\n")

    c.append("const char *const LOOM_QW_RENDER[LOOM_LANG_COUNT][LOOM_ROLE_COUNT] = {")
    for l in langs:
        row = ", ".join(_c_string(pack.qw_render.get(l.tag, {}).get(r, ""))
                        for r in range(7))
        c.append(f"    {{ {row} }},  /* {l.tag} */")
    c.append("};\n")

    c.append("const loom_bucket_t LOOM_BUCKET[LOOM_LANG_COUNT][256] = {")
    for i, l in enumerate(langs):
        c.append(f"    {{ /* {l.tag} */")
        row = []
        for b in range(256):
            first, count = buckets[i][b]
            row.append(f"{{{first},{count}}}")
        for k in range(0, 256, 8):
            c.append("        " + ", ".join(row[k:k + 8]) + ",")
        c.append("    },")
    c.append("};\n")

    c.append("""int loom_lang_index(const char *tag)
{
    uint8_t i;
    if (!tag) return -1;
    for (i = 0; i < LOOM_LANG_COUNT; i++) {
        if (strcmp(LOOM_LANG[i].tag, tag) == 0) return (int)i;
    }
    return -1;
}

static const loom_sym_t *loom_find(uint16_t sym)
{
    uint16_t lo = 0, hi = LOOM_SYM_COUNT;
    while (lo < hi) {                      /* LOOM_SYM esta ordenado por sym */
        uint16_t mid = (uint16_t)(lo + (hi - lo) / 2u);
        if (LOOM_SYM[mid].sym == sym) return &LOOM_SYM[mid];
        if (LOOM_SYM[mid].sym < sym) lo = (uint16_t)(mid + 1u);
        else hi = mid;
    }
    return NULL;
}

const char *loom_sym_render(uint16_t sym, uint8_t lang)
{
    const loom_sym_t *s;
    if (lang >= LOOM_LANG_COUNT) return NULL;
    s = loom_find(sym);
    return s ? s->render[lang] : NULL;
}

const char *loom_sym_neg(uint16_t sym, uint8_t lang)
{
    const loom_sym_t *s;
    if (lang >= LOOM_LANG_COUNT) return NULL;
    s = loom_find(sym);
    return s ? s->neg[lang] : NULL;
}
""")
    return "\n".join(h), "\n".join(c)


def emit_json(pack: B.Pack, name: str, digest: str) -> str:
    """Pacote para o navegador: as mesmas tabelas, forma compacta."""
    out = {
        "pack": name,
        "version": pack.grammar.get("version", 1),
        "digest": digest,
        "langs": [{"tag": l.tag, "name": l.name, "endonym": l.endonym,
                   "spaced": l.spaced, "join": l.join, "dir": l.dir,
                   "speakers_m": l.speakers_m} for l in pack.langs],
        "roles": {v: k for k, v in B.ROLE_NAME.items()},
        "ops": {B.OP_NAME[k]: k for k in B.OP_NAME},
        "lex": {t: [[e.form, e.cls, e.role, e.val]
                    for e in sorted(pack.lex[t].values(),
                                    key=lambda e: (-len(e.form), e.form))]
                for t in pack.tags},
        "syms": [{"sym": c["sym"], "role": c["role"], "id": c["full"],
                  "gloss": c.get("gloss", ""),
                  "render": {t: (c["lex"][t][0] if c["lex"].get(t) else None)
                             for t in pack.tags},
                  "neg": {t: v for t, v in (c.get("neg") or {}).items()}}
                 for c in pack.concepts],
        "templates": {t: {B.OP_NAME[op]: segs
                          for op, segs in pack.templates[t].items()}
                      for t in pack.tags},
        "op_render": {t: {str(k): v for k, v in pack.op_render.get(t, {}).items()}
                      for t in pack.tags},
        "neg_render": pack.neg_render,
        "urg_render": {t: {str(k): v for k, v in pack.urg_render.get(t, {}).items()}
                       for t in pack.tags},
        "qw_render": {t: {str(k): v for k, v in pack.qw_render.get(t, {}).items()}
                      for t in pack.tags},
    }
    return json.dumps(out, ensure_ascii=False, separators=(",", ":"))


def pack_digest(*pack_dirs: pathlib.Path) -> str:
    hh = hashlib.sha256()
    for pack_dir in pack_dirs:
        hh.update(pack_dir.name.encode())
        for f in sorted(pack_dir.glob("*.hlx.json")):
            hh.update(f.name.encode())
            hh.update(f.read_bytes())
    return hh.hexdigest()[:32]


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("--pack", action="append", default=None,
                    help="pode repetir: --pack core --pack campo")
    ap.add_argument("--check", action="store_true",
                    help="portao apenas (padrao)")
    ap.add_argument("--emit", action="store_true")
    ap.add_argument("--quiet", action="store_true")
    args = ap.parse_args()

    specs = args.pack or ["research/loom/core"]
    pack_dirs = [(ROOT / s) if not pathlib.Path(s).is_absolute() else pathlib.Path(s)
                 for s in specs]
    name = "+".join(d.name for d in pack_dirs)
    try:
        pack = B.Pack.load(*pack_dirs)
    except B.Collision as exc:
        print(f"LOOM {name}: FAIL pacote nao carrega — {exc}")
        return 1

    digest = pack_digest(*pack_dirs)
    gate = Gate(pack, pack_dirs[0])
    ok = gate.run()

    if not args.quiet:
        print(f"--- loom: pacote {name} v{pack.grammar.get('version',1)} "
              f"digest {digest} ---")
        if pack.extensions:
            print(f"  extensoes                  {', '.join(pack.extensions)}")
        for k in ("conceitos", "idiomas", "formas", "congelados",
                  "significados_simbolos", "significados_estruturas",
                  "significados_pares", "idas_e_voltas", "lacunas", "corpus",
                  "corpus_total"):
            if k in gate.counts:
                print(f"  {k:26s} {gate.counts[k]:,}".replace(",", "."))
        for n in gate.notes:
            print(f"  nota: {n}")

    if not ok:
        print(f"LOOM {name}: FAIL {len(gate.failures)} violacoes")
        for f in gate.failures[:40]:
            print(f"  {f}")
        if len(gate.failures) > 40:
            print(f"  ... e mais {len(gate.failures) - 40}")
        return 1

    speakers = sum(l.speakers_m for l in pack.langs)
    print(f"LOOM {name}: PASS L1-L10 — {len(pack.concepts)} conceitos, "
          f"{len(pack.langs)} idiomas (~{speakers/1000:.1f} bi de falantes), "
          f"{gate.counts['idas_e_voltas']:,} idas-e-voltas, "
          f"{gate.counts.get('corpus',0)}/{gate.counts.get('corpus_total',0)} "
          f"corpus".replace(",", "."))

    if args.emit:
        # O nome de exibicao pode ser "core+campo"; o nome de ARQUIVO nao pode,
        # porque vira identificador de C e caminho em disco.
        stem = name.replace("+", "_")
        h, c = emit_c(pack, stem, digest)
        (ROOT / "firmware" / "core" / f"loom_{stem}.h").write_text(h, encoding="utf-8")
        (ROOT / "firmware" / "core" / f"loom_{stem}.c").write_text(c, encoding="utf-8")
        blob = emit_json(pack, stem, digest)
        (pack_dirs[0].parent / f"{stem}.compiled.json").write_text(blob, encoding="utf-8")
        print(f"LOOM {name}: EMIT firmware/core/loom_{stem}.{{h,c}} "
              f"({len(c):,} bytes) e research/loom/{stem}.compiled.json "
              f"({len(blob):,} bytes)".replace(",", "."))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
