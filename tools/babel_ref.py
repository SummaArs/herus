"""babel_ref.py — semantica de REFERENCIA do Babel, em Python, stdlib only.

Este arquivo nao e um utilitario: e a definicao executavel do que Babel faz.
O C em firmware/core/babel.c tem de concordar com ele byte a byte (a suite
diferencial prova isso). Quando os dois discordam, um dos dois esta errado e
a divergencia e o achado — nunca "arredonda-se" para o C.

Tres operacoes:

  fold(texto)                 -> forma canonica de superficie
  compile(texto, idioma)      -> significado tipado (HIR) ou recusa tipada
  render(hir, idioma)         -> frase telegrafica naquele idioma

O teorema que o portao do Loom prova sobre elas:

    para todo significado h alcancavel e todo par de idiomas L1, L2:
        compile(render(h, L1), L1) == h == compile(render(h, L2), L2)

ou seja: o significado sobrevive a viagem por qualquer lingua. E por isso que
24 bytes atravessam a fronteira do idioma sem tradutor, sem nuvem e sem conta.
"""

from __future__ import annotations

# ---------------------------------------------------------------- constantes

HIR_VERSION = 1
HIR_MAX_SLOTS = 9
HIR_WIRE_BYTES = 24
HIR_FILLER_VAR = 0
HIR_FILLER_MAX = (1 << 11) - 1

OP_NONE, OP_COMUNICAR, OP_PERGUNTAR, OP_LEMBRAR = 0, 1, 2, 3
OP_PLANEJAR, OP_SOCORRO, OP_CONFIRMAR, OP_CANCELAR = 4, 5, 6, 7
OP_NAME = {1: "COMUNICAR", 2: "PERGUNTAR", 3: "LEMBRAR", 4: "PLANEJAR",
           5: "SOCORRO", 6: "CONFIRMAR", 7: "CANCELAR"}

ROLE_NONE, ROLE_QUEM, ROLE_O_QUE, ROLE_QUANDO = 0, 1, 2, 3
ROLE_ONDE, ROLE_QUANTO, ROLE_ESTADO = 4, 5, 6
ROLE_NAME = {1: "QUEM", 2: "O_QUE", 3: "QUANDO", 4: "ONDE", 5: "QUANTO", 6: "ESTADO"}
ROLE_CODE = {v: k for k, v in ROLE_NAME.items()}

URG_ROTINA, URG_ATENCAO, URG_URGENTE, URG_SOCORRO = 0, 1, 2, 3
PERSIST_EFEMERO, PERSIST_LEMBRAR = 0, 1

# classes lexicais
LX_STOP, LX_OP, LX_FILL, LX_NEG, LX_URG, LX_QVAR, LX_SENS, LX_AUTH = range(8)
# Negacao LEXICAL: uma forma que carrega o preenchimento E a polaridade.
#
# Existe porque em japones e chines a negacao e morfologica, nao um marcador
# solto: 着いた + ない nao produz "nao cheguei", produz uma frase que nenhum
# japones escreveria. Um marcador funciona em portugues ("nao cheguei") e
# quebra em japones. A saida nao e forcar a lingua a caber no template — e
# deixar o DADO fornecer a forma negativa e manter o codigo fechado.
#
# Onde a forma negativa nao existe no pacote, o marcador continua valendo. O
# ida-e-volta e provado nos dois caminhos, entao a lacuna e visivel e honesta
# em vez de silenciosa.
LX_FILLNEG = 8
LX_NAME = {LX_STOP: "STOP", LX_OP: "OP", LX_FILL: "FILL", LX_NEG: "NEG",
           LX_URG: "URG", LX_QVAR: "QVAR", LX_SENS: "SENS", LX_AUTH: "AUTH",
           LX_FILLNEG: "FILLNEG"}

# recusas tipadas — mesmos codigos do C
OK, E_ARG, E_EMPTY, E_OVERFLOW, E_GAP = "OK", "ARG", "EMPTY", "OVERFLOW", "GAP"
E_AMBIGUOUS, E_SENSITIVE, E_AUTHORITY, E_INCOMPLETE, E_BYTE = \
    "AMBIGUOUS", "SENSITIVE", "AUTHORITY", "INCOMPLETE", "BYTE"

# severidade para escolher deterministicamente entre recusas concorrentes.
# Falhar fechado: a recusa mais restritiva ganha, nunca a mais conveniente.
SEVERITY = {E_SENSITIVE: 90, E_AUTHORITY: 80, E_AMBIGUOUS: 70, E_OVERFLOW: 60,
            E_INCOMPLETE: 50, E_GAP: 40, E_EMPTY: 30, E_BYTE: 20, E_ARG: 10}

TEXT_MAX = 160          # bytes de entrada
TOKEN_MAX = 24          # tokens depois do dobramento
PATH_MAX = 64           # leituras distintas exploradas antes de falhar fechado
MEANING_MAX = 4         # significados distintos guardados; acima disso recusa
EDGE_MAX = 512          # arestas de segmentacao consideradas

# ------------------------------------------------------------------ dobramento

_ACCENT = {}
for _b in range(0xA0, 0xA6): _ACCENT[_b] = "a"
for _b in range(0x80, 0x86): _ACCENT[_b] = "a"
_ACCENT[0xA7] = _ACCENT[0x87] = "c"
for _b in range(0xA8, 0xAC): _ACCENT[_b] = "e"
for _b in range(0x88, 0x8C): _ACCENT[_b] = "e"
for _b in range(0xAC, 0xB0): _ACCENT[_b] = "i"
for _b in range(0x8C, 0x90): _ACCENT[_b] = "i"
_ACCENT[0xB1] = _ACCENT[0x91] = "n"
for _b in range(0xB2, 0xB7): _ACCENT[_b] = "o"
for _b in range(0x92, 0x97): _ACCENT[_b] = "o"
for _b in range(0xB9, 0xBD): _ACCENT[_b] = "u"
for _b in range(0x99, 0x9D): _ACCENT[_b] = "u"

_ASCII_SPACE = set(" \t,.!?;:\n\r-'")

# pontuacao de escrita CJK e largura plena que vira espaco
_WIDE_SPACE = {"　", "、", "。", "，", "！", "？",
               "：", "；", "・", "…"}


class Folded:
    """Resultado do dobramento: texto canonico, ou um erro tipado."""
    __slots__ = ("text", "status")

    def __init__(self, text: str, status: str = OK):
        self.text = text
        self.status = status


def fold(raw: bytes | str, cap: int = TEXT_MAX + 1) -> Folded:
    """Forma canonica de superficie.

    ASCII vira minusculo; acento latino cai para a letra base; pontuacao vira
    espaco; qualquer outro ponto de codigo UTF-8 valido passa INTACTO — e isso
    que deixa japones e chines entrarem sem romper o contrato de bytes do
    Herald original. Byte invalido, NUL ou ASCII fora do conjunto aceito e
    recusa, nunca substituicao silenciosa.
    """
    data = raw.encode("utf-8") if isinstance(raw, str) else raw
    # Contrato de capacidade, igual ao do C: um buffer que nao cabe e recusa
    # explicita, nao truncamento silencioso.
    if len(data) + 1 > cap:
        return Folded("", E_OVERFLOW)
    out: list[str] = []
    i, n = 0, len(data)
    last_space = True
    while i < n:
        c = data[i]
        if c == 0:
            return Folded("", E_BYTE)
        if c < 0x80:
            if 0x41 <= c <= 0x5A:
                ch = chr(c + 32); i += 1
            elif (0x61 <= c <= 0x7A) or (0x30 <= c <= 0x39):
                ch = chr(c); i += 1
            elif chr(c) in _ASCII_SPACE:
                ch = " "; i += 1
            else:
                return Folded("", E_BYTE)
        elif c == 0xC3:
            if i + 1 >= n:
                return Folded("", E_BYTE)
            mapped = _ACCENT.get(data[i + 1])
            if mapped is None:
                return Folded("", E_BYTE)
            ch = mapped; i += 2
        else:
            # sequencia UTF-8 multibyte: valida o comprimento e copia intacta
            if 0xC2 <= c <= 0xDF:
                width = 2
            elif 0xE0 <= c <= 0xEF:
                width = 3
            elif 0xF0 <= c <= 0xF4:
                width = 4
            else:
                return Folded("", E_BYTE)
            if i + width > n:
                return Folded("", E_BYTE)
            for k in range(1, width):
                if not (0x80 <= data[i + k] <= 0xBF):
                    return Folded("", E_BYTE)
            try:
                ch = data[i:i + width].decode("utf-8")
            except UnicodeDecodeError:
                return Folded("", E_BYTE)
            if ch in _WIDE_SPACE:
                ch = " "
            i += width

        if ch == " ":
            if last_space:
                continue
            last_space = True
        else:
            last_space = False
        out.append(ch)
    text = "".join(out).rstrip(" ")
    return Folded(text, OK)


# ------------------------------------------------------------------------ HIR

class Hir:
    """Significado canonico. Slots ordenados por (papel, preenchimento)."""
    __slots__ = ("op", "polarity", "urgency", "persistence", "slots")

    def __init__(self):
        self.op = OP_NONE
        self.polarity = 0
        self.urgency = URG_ROTINA
        self.persistence = PERSIST_EFEMERO
        self.slots: list[tuple[int, int]] = []

    # -- construcao -------------------------------------------------------
    def put(self, role: int, filler: int) -> str:
        if role == ROLE_NONE or role > ROLE_ESTADO:
            return "RANGE"
        if filler > HIR_FILLER_MAX:
            return "RANGE"
        for r, f in self.slots:
            if r == role:
                # Espelha hir.c: repetir o MESMO preenchimento no mesmo papel
                # nao acrescenta informacao e nao e ambiguidade. "cheguei
                # cheguei" nao tem duas leituras. Preenchimento diferente no
                # mesmo papel, sim: isso e conflito e recusa.
                return "OK" if f == filler else "DUPLICATE_ROLE"
        if len(self.slots) >= HIR_MAX_SLOTS:
            return "FULL"
        self.slots.append((role, filler))
        self.slots.sort()
        return "OK"

    def get(self, role: int):
        for r, f in self.slots:
            if r == role:
                return f
        return None

    # -- forma canonica ---------------------------------------------------
    # Espelha hir.c byte a byte. Qualquer divergencia aqui e um bug NESTE
    # arquivo, nao no C: o C e o artefato que embarca.
    def canon(self) -> bytes:
        out = bytearray()
        out.append(ord("H"))
        out.append(HIR_VERSION)
        out.append(self.op)
        out.append(self.polarity | (self.urgency << 1) | (self.persistence << 3))
        out.append(len(self.slots))
        out.append(0x00)
        for role, filler in self.slots:
            out.append(role)
            out.append((filler >> 8) & 0xFF)
            out.append(filler & 0xFF)
        return bytes(out)

    def digest(self) -> bytes:
        import hashlib
        return hashlib.sha256(self.canon()).digest()[:8]

    def wire(self) -> bytes:
        """24 bytes, bit-compativel com HCP Tier 1."""
        out = bytearray(HIR_WIRE_BYTES)
        out[0] = ((HIR_VERSION & 0x0F) << 4) | (self.op & 0x0F)
        out[1] = self.polarity | (self.urgency << 1) | (self.persistence << 3)
        out[2] = len(self.slots)
        off = 3
        for role, filler in self.slots:
            packed = ((role & 0x1F) << 11) | (filler & 0x7FF)
            out[off] = (packed >> 8) & 0xFF
            out[off + 1] = packed & 0xFF
            off += 2
        return bytes(out)

    def key(self):
        return (self.op, self.polarity, self.urgency, self.persistence,
                tuple(self.slots))

    def __eq__(self, other):
        return isinstance(other, Hir) and self.key() == other.key()

    def __hash__(self):
        return hash(self.key())

    def __repr__(self):
        parts = [f"{ROLE_NAME[r]}={f}" for r, f in self.slots]
        return (f"Hir({OP_NAME.get(self.op, self.op)} pol={self.polarity} "
                f"urg={self.urgency} per={self.persistence} {' '.join(parts)})")


def hir_requires_confirmation(h: Hir) -> bool:
    return h.op in (OP_COMUNICAR, OP_LEMBRAR, OP_SOCORRO, OP_CANCELAR)


def hir_validate(h: Hir) -> str:
    if h.op == OP_NONE or h.op >= 8:
        return "RANGE"
    if h.polarity > 1 or h.urgency > URG_SOCORRO or h.persistence > 2:
        return "RANGE"
    if len(h.slots) > HIR_MAX_SLOTS:
        return "RANGE"
    if not h.slots:
        return "EMPTY"
    seen = set()
    for role, filler in h.slots:
        if role == ROLE_NONE or role > ROLE_ESTADO or filler > HIR_FILLER_MAX:
            return "RANGE"
        if role in seen:
            return "DUPLICATE_ROLE"
        seen.add(role)
    return "OK"


# ------------------------------------------------------------------- o pacote

class Lang:
    __slots__ = ("tag", "name", "endonym", "script", "spaced", "join", "dir",
                 "speakers_m", "index")

    def __init__(self, d, index):
        self.tag = d["tag"]
        self.name = d["name"]
        self.endonym = d.get("endonym", d["name"])
        self.script = d["script"]
        self.spaced = d["segment"] == "space"
        self.join = d["join"]
        self.dir = d.get("dir", "ltr")
        self.speakers_m = d.get("speakers_m", 0)
        self.index = index


class Entry:
    __slots__ = ("form", "cls", "role", "val", "origin")

    def __init__(self, form, cls, role, val, origin):
        self.form = form
        self.cls = cls
        self.role = role
        self.val = val
        self.origin = origin   # de onde veio, para a mensagem de colisao

    def __repr__(self):
        return f"Entry({self.form!r} {LX_NAME[self.cls]} role={self.role} val={self.val})"


class Collision(Exception):
    pass


class Pack:
    """O espaco semantico compilado: um pacote, N idiomas, um portao."""

    def __init__(self, parts: list[dict]):
        grammar = None
        concept_parts = []
        extensions = []
        for part in parts:
            if part.get("part") == "grammar":
                if grammar is not None:
                    raise Collision("mais de uma gramatica no pacote")
                grammar = part
            elif part.get("part") == "concepts":
                concept_parts.append(part)
            elif part.get("part") == "extension":
                extensions.append(part)
            else:
                raise Collision(f"parte desconhecida: {part.get('part')!r}")
        if grammar is None:
            raise Collision("pacote sem gramatica")

        # Um pacote de DOMINIO nao traz gramatica propria: ele ESTENDE a do
        # nucleo com namespaces novos e, se precisar, mais palavras de ruido,
        # classe protegida ou autoridade. Nao pode redefinir namespace que ja
        # existe — isso mudaria o significado de simbolos ja no ar.
        self.extensions = [e.get("pack", "?") for e in extensions]
        for ext in extensions:
            for ns, spec in (ext.get("namespaces") or {}).items():
                if ns in grammar["namespaces"]:
                    raise Collision(
                        f"extensao {ext.get('pack')!r} redefine o namespace {ns!r}")
                base = spec["base"]
                for other, ospec in grammar["namespaces"].items():
                    lo, hi = ospec["base"], ospec["base"] + ospec["cap"]
                    if lo <= base <= hi:
                        raise Collision(
                            f"extensao {ext.get('pack')!r}: namespace {ns!r} em "
                            f"{base} colide com {other!r} ({lo}..{hi})")
                grammar["namespaces"][ns] = spec
            for kind in ("stop", "sensitive", "authority"):
                for tag, forms in (ext.get(kind) or {}).items():
                    grammar[kind].setdefault(tag, [])
                    grammar[kind][tag] = list(grammar[kind][tag]) + list(forms)

        self.grammar = grammar
        self.namespaces = grammar["namespaces"]
        self.langs = [Lang(d, i) for i, d in enumerate(grammar["languages"])]
        self.lang_by_tag = {l.tag: l for l in self.langs}
        self.tags = [l.tag for l in self.langs]

        self.problems: list[str] = []

        # ---- conceitos ---------------------------------------------------
        self.concepts: list[dict] = []
        self.sym_of: dict[str, int] = {}
        self.concept_by_sym: dict[int, dict] = {}
        for part in concept_parts:
            ns = part["ns"]
            if ns not in self.namespaces:
                self.problems.append(f"namespace desconhecido {ns!r}")
                continue
            base = self.namespaces[ns]["base"]
            cap = self.namespaces[ns]["cap"]
            role = ROLE_CODE[self.namespaces[ns]["role"]]
            for c in part["concepts"]:
                if not 0 <= c["n"] < cap:
                    self.problems.append(f"{ns}.{c['id']}: n={c['n']} fora do namespace")
                    continue
                sym = base + c["n"]
                full = f"{ns}.{c['id']}"
                if sym in self.concept_by_sym:
                    self.problems.append(
                        f"simbolo {sym} reivindicado por {full} e "
                        f"{self.concept_by_sym[sym]['full']}")
                    continue
                rec = dict(c, ns=ns, sym=sym, role=role, full=full)
                self.concepts.append(rec)
                self.concept_by_sym[sym] = rec
                self.sym_of[full] = sym
        self.concepts.sort(key=lambda r: r["sym"])

        # ---- lexico por idioma ------------------------------------------
        self.lex: dict[str, dict[str, Entry]] = {t: {} for t in self.tags}
        self.lengths: dict[str, list[int]] = {t: [] for t in self.tags}

        def add(tag, form, cls, role, val, origin):
            if tag not in self.lex:
                self.problems.append(f"{origin}: idioma {tag!r} nao declarado")
                return
            f = fold(form)
            if f.status != OK:
                self.problems.append(f"{origin}: forma {form!r} nao dobra ({f.status})")
                return
            if f.text != form:
                self.problems.append(
                    f"{origin}: forma {form!r} nao esta na forma dobrada "
                    f"(seria {f.text!r})")
                return
            if not form:
                self.problems.append(f"{origin}: forma vazia")
                return
            prev = self.lex[tag].get(form)
            if prev is not None:
                if (prev.cls, prev.role, prev.val) == (cls, role, val):
                    self.problems.append(
                        f"[{tag}] forma {form!r} declarada duas vezes "
                        f"({prev.origin} e {origin})")
                else:
                    self.problems.append(
                        f"[{tag}] COLISAO: {form!r} e {LX_NAME[prev.cls]}"
                        f"/{prev.val} por {prev.origin} e {LX_NAME[cls]}/{val} "
                        f"por {origin}")
                return
            self.lex[tag][form] = Entry(form, cls, role, val, origin)

        for c in self.concepts:
            gaps = set(c.get("gap", []))
            for tag in self.tags:
                if tag in gaps:
                    continue
                forms = c["lex"].get(tag)
                if not forms:
                    self.problems.append(
                        f"{c['full']}: sem forma em {tag!r} e sem lacuna declarada")
                    continue
                for form in forms:
                    add(tag, form, LX_FILL, c["role"], c["sym"], c["full"])
            for tag, form in (c.get("neg") or {}).items():
                if c["role"] != ROLE_O_QUE:
                    self.problems.append(
                        f"{c['full']}: forma negativa so faz sentido em O_QUE")
                    continue
                add(tag, form, LX_FILLNEG, c["role"], c["sym"],
                    c["full"] + "(neg)")

        self.op_render: dict[str, dict[int, str]] = {t: {} for t in self.tags}
        for op in grammar["ops"]:
            for tag, forms in op["lex"].items():
                for form in forms:
                    add(tag, form, LX_OP, 0, op["code"], f"op {op['id']}")
                if forms:
                    self.op_render.setdefault(tag, {})[op["code"]] = forms[0]

        self.qw_render: dict[str, dict[int, str]] = {t: {} for t in self.tags}
        for qw in grammar["qwords"]:
            role = ROLE_CODE[qw["role"]]
            for tag, forms in qw["lex"].items():
                for form in forms:
                    add(tag, form, LX_QVAR, role, 0, f"qword {qw['role']}")
                if forms:
                    self.qw_render.setdefault(tag, {})[role] = forms[0]

        self.urg_render: dict[str, dict[int, str]] = {t: {} for t in self.tags}
        for u in grammar["urgency"]:
            for tag, forms in u["lex"].items():
                for form in forms:
                    add(tag, form, LX_URG, 0, u["level"], f"urgency {u['id']}")
                if forms:
                    self.urg_render.setdefault(tag, {})[u["level"]] = forms[0]

        self.neg_render: dict[str, str] = {}
        for tag, forms in grammar["negation"].items():
            for form in forms:
                add(tag, form, LX_NEG, 0, 0, "negation")
            if forms:
                self.neg_render[tag] = forms[0]

        for kind, cls in (("sensitive", LX_SENS), ("authority", LX_AUTH),
                          ("stop", LX_STOP)):
            for tag, forms in grammar[kind].items():
                for form in forms:
                    add(tag, form, cls, 0, 0, kind)

        for tag in self.tags:
            self.lengths[tag] = sorted({len(f) for f in self.lex[tag]}, reverse=True)

        # ---- templates ---------------------------------------------------
        self.templates: dict[str, dict[int, list[str]]] = {}
        for tag, per_op in grammar["templates"].items():
            if tag not in self.lang_by_tag:
                self.problems.append(f"template para idioma nao declarado {tag!r}")
                continue
            table = {}
            for name, segs in per_op.items():
                code = next((k for k, v in OP_NAME.items() if v == name), None)
                if code is None:
                    self.problems.append(f"[{tag}] template para op inexistente {name!r}")
                    continue
                table[code] = list(segs)
            missing = [OP_NAME[c] for c in OP_NAME if c not in table]
            if missing:
                self.problems.append(f"[{tag}] sem template para {', '.join(missing)}")
            self.templates[tag] = table
        for tag in self.tags:
            if tag not in self.templates:
                self.problems.append(f"[{tag}] sem tabela de templates")

    # -- carga ------------------------------------------------------------
    @classmethod
    def load(cls, *directories) -> "Pack":
        """Um ou mais diretorios. Varios = nucleo + dominio, na ordem dada."""
        import json
        import pathlib
        parts = []
        for directory in directories:
            d = pathlib.Path(directory)
            found = sorted(d.glob("*.hlx.json"))
            if not found:
                raise Collision(f"nenhum .hlx.json em {d}")
            for f in found:
                parts.append(json.loads(f.read_text(encoding="utf-8")))
        if not parts:
            raise Collision("nenhum pacote informado")
        return cls(parts)

    # -- utilitarios ------------------------------------------------------
    def render_form(self, sym: int, tag: str) -> str | None:
        c = self.concept_by_sym.get(sym)
        if c is None:
            return None
        forms = c["lex"].get(tag)
        return forms[0] if forms else None

    def gap_langs(self, sym: int) -> set[str]:
        """Idiomas em que este simbolo DECLARA nao ter forma."""
        c = self.concept_by_sym.get(sym)
        return set(c.get("gap", [])) if c else set()

    def render_gap(self, h, tag: str) -> int | None:
        """O primeiro simbolo do significado que nao veste este idioma, ou None.

        Existe porque "nao consigo dizer isto na sua lingua" e uma resposta
        legitima e tem de ser TIPADA. Um pacote de dominio pode existir em
        quatro idiomas e nao em oito, e a alternativa a lacuna seria inventar
        vocabulario tecnico que ninguem revisou — exatamente a aproximacao que
        este projeto recusa em todo lugar.
        """
        for _role, filler in h.slots:
            if filler == HIR_FILLER_VAR:
                continue
            if self.render_form(filler, tag) is None:
                return filler
        return None

    def neg_form(self, sym: int, tag: str) -> str | None:
        c = self.concept_by_sym.get(sym)
        if c is None:
            return None
        return (c.get("neg") or {}).get(tag)

    def symbols_for_role(self, role: int) -> list[int]:
        return [c["sym"] for c in self.concepts if c["role"] == role]


# ------------------------------------------------------------------ compilar

class Unit:
    """Resultado de uma compilacao: significado ou recusa, nunca as duas."""
    __slots__ = ("status", "hir", "gap_at", "gap_len", "missing_role",
                 "readings", "paths")

    def __init__(self, status, hir=None, gap_at=0, gap_len=0, missing_role=0,
                 readings=0, paths=0):
        self.status = status
        self.hir = hir
        self.gap_at = gap_at
        self.gap_len = gap_len
        self.missing_role = missing_role
        self.readings = readings     # leituras distintas COM significado
        self.paths = paths           # caminhos completos explorados

    @property
    def ok(self):
        return self.status == OK

    def __repr__(self):
        if self.ok:
            return f"Unit(OK {self.hir!r})"
        return f"Unit({self.status} gap@{self.gap_at})"


def _alnum(ch: str) -> bool:
    return ("a" <= ch <= "z") or ("0" <= ch <= "9")


def _edges(text: str, lang: Lang, lex: dict, lengths: list[int]):
    """Arestas de segmentacao a partir de cada posicao.

    Duas fronteiras, uma por escrita:
      - idioma com espaco: um casamento tem de comecar e terminar em fronteira
        de palavra. 'casa' nunca casa dentro de 'casaco'.
      - idioma sem espaco (ja, zh): a fronteira e a transicao de escrita. Um
        numeral ou palavra latina embutida e ATOMICO — '10' nunca se le como
        '1' seguido de '0'.

    A mesma funcao serve os dois casos, e e por isso que japones nao precisa de
    um analisador proprio: precisa de um predicado de fronteira proprio.
    """
    n = len(text)
    out: list[list[tuple[int, Entry | None]]] = [[] for _ in range(n + 1)]
    total = 0
    for i in range(n):
        if text[i] == " ":
            out[i].append((i + 1, None))          # espaco: aresta de salto
            continue
        if lang.spaced and i > 0 and text[i - 1] != " ":
            continue                               # nao e inicio de palavra
        for ln in lengths:
            j = i + ln
            if j > n:
                continue
            e = lex.get(text[i:j])
            if e is None:
                continue
            if lang.spaced:
                if j < n and text[j] != " ":
                    continue
            else:
                if i > 0 and _alnum(text[i - 1]) and _alnum(text[i]):
                    continue
                if j < n and _alnum(text[j - 1]) and _alnum(text[j]):
                    continue
            out[i].append((j, e))
            total += 1
            if total > EDGE_MAX:
                return out, total
    return out, total


def _reachable_end(edges, n: int) -> int:
    """Posicao mais distante alcancavel do inicio — onde a lacuna comeca."""
    seen = [False] * (n + 1)
    seen[0] = True
    best = 0
    for i in range(n + 1):
        if not seen[i]:
            continue
        best = max(best, i)
        for j, _e in edges[i]:
            if not seen[j]:
                seen[j] = True
    return best


def _paths(edges, n: int):
    """Todos os caminhos completos 0 -> n, no maximo PATH_MAX.

    Ordem canonica: em cada posicao, a aresta mais longa primeiro. Isso faz o
    primeiro caminho coincidir com o casamento guloso do Herald original, o que
    e o que permite provar que Babel generaliza o Herald em vez de substituir.
    """
    result: list[list[Entry]] = []
    truncated = False
    stack: list[tuple[int, list[Entry]]] = [(0, [])]
    order = [sorted(edges[i], key=lambda t: -(t[0] - i)) for i in range(n + 1)]

    def walk(pos, acc):
        nonlocal truncated
        if len(result) >= PATH_MAX:
            truncated = True
            return
        if pos == n:
            result.append(list(acc))
            return
        for j, e in order[pos]:
            acc.append(e) if e is not None else None
            walk(j, acc)
            if e is not None:
                acc.pop()
            if len(result) >= PATH_MAX:
                truncated = True
                return

    walk(0, [])
    return result, truncated


def _build(reading: list[Entry], pack: Pack) -> tuple[str, Hir | None, int]:
    """Uma leitura -> significado tipado, ou recusa. Espelha herald.c."""
    h = Hir()
    op = OP_NONE
    qcount = 0
    for e in reading:
        if e.cls == LX_SENS:
            return E_SENSITIVE, None, 0
        if e.cls == LX_AUTH:
            return E_AUTHORITY, None, 0
        if e.cls == LX_STOP:
            continue
        if e.cls == LX_NEG:
            h.polarity = 1
        elif e.cls == LX_URG:
            if h.urgency != 0 and h.urgency != e.val:
                return E_AMBIGUOUS, None, 0
            h.urgency = e.val
        elif e.cls == LX_OP:
            if op != OP_NONE and op != e.val:
                return E_AMBIGUOUS, None, 0
            op = e.val
        elif e.cls == LX_QVAR:
            st = h.put(e.role, HIR_FILLER_VAR)
            if st == "DUPLICATE_ROLE":
                return E_AMBIGUOUS, None, 0
            if st == "FULL":
                return E_OVERFLOW, None, 0
            if st != "OK":
                return E_ARG, None, 0
            qcount += 1
        elif e.cls in (LX_FILL, LX_FILLNEG):
            st = h.put(e.role, e.val)
            if st == "DUPLICATE_ROLE":
                return E_AMBIGUOUS, None, 0
            if st == "FULL":
                return E_OVERFLOW, None, 0
            if st != "OK":
                return E_ARG, None, 0
            if e.cls == LX_FILLNEG:
                h.polarity = 1
        else:
            return E_ARG, None, 0

    # inferencia estrutural: nunca inventa conteudo, so a operacao
    if op == OP_NONE:
        if qcount > 0:
            op = OP_PERGUNTAR
        elif h.slots:
            op = OP_COMUNICAR
        else:
            return E_INCOMPLETE, None, 0
    if qcount > 0 and op != OP_PERGUNTAR:
        return E_AMBIGUOUS, None, 0
    h.op = op

    if op == OP_SOCORRO:
        h.urgency = URG_SOCORRO
        h.put(ROLE_QUEM, pack.sym_of["PER.TODOS"])   # destinatario explicito ganha
    if op == OP_LEMBRAR:
        h.persistence = PERSIST_LEMBRAR

    roles = {r for r, _f in h.slots}
    has_var = any(f == HIR_FILLER_VAR for _r, f in h.slots)
    if op == OP_COMUNICAR:
        if ROLE_QUEM not in roles:
            return E_INCOMPLETE, None, ROLE_QUEM
        if ROLE_O_QUE not in roles:
            return E_INCOMPLETE, None, ROLE_O_QUE
    elif op == OP_PERGUNTAR:
        if qcount > 1:
            return E_AMBIGUOUS, None, 0
        if not has_var or qcount != 1 or len(h.slots) < 2:
            return E_INCOMPLETE, None, 0
    elif op == OP_LEMBRAR:
        if not (roles & {ROLE_O_QUE, ROLE_ONDE, ROLE_QUANDO}):
            return E_INCOMPLETE, None, ROLE_O_QUE
    elif op in (OP_CONFIRMAR, OP_CANCELAR):
        if ROLE_O_QUE not in roles:
            return E_INCOMPLETE, None, ROLE_O_QUE
    elif op == OP_PLANEJAR:
        if not (roles & {ROLE_O_QUE, ROLE_ONDE}):
            return E_INCOMPLETE, None, ROLE_O_QUE
    elif op == OP_SOCORRO:
        pass
    else:
        return E_INCOMPLETE, None, 0
    if has_var and op != OP_PERGUNTAR:
        return E_AMBIGUOUS, None, 0

    if hir_validate(h) != "OK":
        return E_INCOMPLETE, None, 0
    return OK, h, 0


def _dominates(a: Hir, b: Hir) -> bool:
    """a ≼ b: mesma moldura, e todo slot de `a` esta em `b` com o mesmo
    preenchimento. Ou seja, `b` diz tudo o que `a` diz e mais."""
    if (a.op, a.polarity, a.urgency, a.persistence) != \
       (b.op, b.polarity, b.urgency, b.persistence):
        return False
    bs = dict(b.slots)
    for role, filler in a.slots:
        if bs.get(role) != filler:
            return False
    return True


def _dominant(meanings: list[Hir]):
    """O maximo unico da ordem ≼, ou None.

    Aqui esta a unica regra de PREFERENCIA de todo o compilador, e ela nao e
    heuristica — e um teorema pequeno:

        ruido nunca acrescenta significado.

    Uma palavra classificada como ruido nao contribui papel nem preenchimento.
    Logo, se duas leituras diferem apenas porque uma delas explicou como RUIDO
    o que a outra explicou como SIGNIFICADO, a segunda contem a primeira e a
    primeira nao contem nada que a segunda perca. Nao ha o que escolher: uma
    domina.

    "emergencia estou aqui" tem duas leituras — SOCORRO sozinho (lendo "estou
    aqui" como duas palavras de ruido) e SOCORRO + CHEGUEI (lendo a frase como
    o evento). A segunda domina, e e obviamente o que a pessoa disse.

    Quando as leituras se CONTRADIZEM — mesmo papel com preenchimento
    diferente, ou operacao diferente — nao ha dominancia e as duas sao
    recusadas. "avisa joao maria cheguei" continua AMBIGUOUS, e o chines
    "取消过来了" tambem: "过来了" e "过来" + particula sao dois eventos
    diferentes, e nenhum contem o outro.
    """
    best = None
    for cand in meanings:
        if all(_dominates(other, cand) for other in meanings):
            if best is not None:
                return None      # dois maximos: nao ha unico
            best = cand
    return best


def compile(text, tag: str, pack: Pack) -> Unit:
    """Texto de superficie -> significado tipado, ou recusa tipada.

    Nao ha terceiro resultado. Nunca ha aproximacao.
    """
    lang = pack.lang_by_tag.get(tag)
    if lang is None:
        return Unit(E_ARG)
    raw = text.encode("utf-8") if isinstance(text, str) else text
    if raw is None:
        return Unit(E_ARG)
    if len(raw) == 0:
        return Unit(E_EMPTY)
    if len(raw) > TEXT_MAX:
        return Unit(E_OVERFLOW)
    f = fold(raw)
    if f.status != OK:
        return Unit(E_BYTE)
    s = f.text.strip(" ")
    if not s:
        return Unit(E_EMPTY)
    if lang.spaced and len(s.split()) > TOKEN_MAX:
        return Unit(E_OVERFLOW)

    lex = pack.lex[tag]
    lengths = pack.lengths[tag]

    # Passo um: classe protegida e lavagem de autoridade sao recusadas onde
    # quer que aparecam, ANTES de qualquer significado ser construido. A recusa
    # nao pode depender da sorte da ordem da esquerda para a direita.
    n = len(s)
    for i in range(n):
        if lang.spaced and i > 0 and s[i - 1] != " ":
            continue
        for ln in lengths:
            j = i + ln
            if j > n:
                continue
            e = lex.get(s[i:j])
            if e is None or e.cls not in (LX_SENS, LX_AUTH):
                continue
            if lang.spaced and j < n and s[j] != " ":
                continue
            return Unit(E_SENSITIVE if e.cls == LX_SENS else E_AUTHORITY)

    edges, total = _edges(s, lang, lex, lengths)
    if total > EDGE_MAX:
        return Unit(E_OVERFLOW)

    readings, truncated = _paths(edges, n)
    if not readings:
        at = _reachable_end(edges, n)
        while at < n and s[at] == " ":
            at += 1
        end = at
        while end < n and s[end] != " ":
            end += 1
        return Unit(E_GAP, gap_at=at, gap_len=max(1, end - at))

    meanings: list[Hir] = []
    refusals: list[tuple[str, int]] = []
    for r in readings:
        st, h, missing = _build(r, pack)
        if st == OK:
            if h not in meanings:
                if len(meanings) >= MEANING_MAX:
                    return Unit(E_AMBIGUOUS, readings=len(meanings) + 1,
                                paths=len(readings))
                meanings.append(h)
        else:
            refusals.append((st, missing))

    if len(meanings) > 1:
        top = _dominant(meanings)
        if top is None:
            return Unit(E_AMBIGUOUS, readings=len(meanings), paths=len(readings))
        if truncated:
            return Unit(E_AMBIGUOUS, readings=len(meanings), paths=len(readings))
        return Unit(OK, top, readings=len(meanings), paths=len(readings))
    if len(meanings) == 1:
        if truncated:
            # nao da para provar unicidade dentro do orcamento: falha fechado
            return Unit(E_AMBIGUOUS, readings=1, paths=len(readings))
        return Unit(OK, meanings[0], readings=1, paths=len(readings))

    # nenhuma leitura produziu significado: devolve a recusa mais restritiva
    st, missing = max(refusals, key=lambda t: SEVERITY.get(t[0], 0))
    return Unit(st, missing_role=missing, paths=len(readings))


# ---------------------------------------------------------------- renderizar

def render(h: Hir, tag: str, pack: Pack) -> str | None:
    """Significado -> frase telegrafica no idioma pedido.

    Telegrafico e uma escolha, nao uma limitacao: e o registro em que a frase
    e natural para um humano E reconhecivel de volta pelo proprio compilador.
    Prosa seria mais bonita e nao fecharia o ciclo.
    """
    lang = pack.lang_by_tag.get(tag)
    if lang is None:
        return None
    tmpl = pack.templates.get(tag, {}).get(h.op)
    if tmpl is None:
        return None

    var_role = next((r for r, f in h.slots if f == HIR_FILLER_VAR), None)
    lexical_neg = False
    values: dict[str, str] = {}
    for role_name, code in ROLE_CODE.items():
        f = h.get(code)
        if f is None or code == var_role:
            values[role_name] = ""
            continue
        form = None
        if h.polarity and code == ROLE_O_QUE:
            form = pack.neg_form(f, tag)
            if form is not None:
                lexical_neg = True
        if form is None:
            form = pack.render_form(f, tag)
        if form is None:
            # LACUNA POR IDIOMA. Nao ha forma para este simbolo aqui, e a
            # resposta certa e nao dizer nada em vez de dizer algo parecido.
            # Quem chama descobre QUAL simbolo com pack.render_gap().
            return None
        values[role_name] = form
    values["OP"] = pack.op_render.get(tag, {}).get(h.op, "")
    # O marcador de negacao so aparece quando a polaridade NAO foi absorvida
    # pela forma lexical. Os dois juntos seriam negacao dupla.
    values["NEG"] = ("" if (not h.polarity or lexical_neg)
                     else pack.neg_render.get(tag, ""))
    if h.urgency in (URG_ATENCAO, URG_URGENTE):
        values["URG"] = pack.urg_render.get(tag, {}).get(h.urgency, "")
    elif h.urgency == URG_SOCORRO and h.op != OP_SOCORRO:
        values["URG"] = pack.urg_render.get(tag, {}).get(URG_URGENTE, "")
    else:
        values["URG"] = ""
    values["QVAR"] = (pack.qw_render.get(tag, {}).get(var_role, "")
                      if var_role is not None else "")

    pieces: list[str] = []
    for seg in tmpl:
        if "%" not in seg:
            pieces.append(seg)
            continue
        out = seg
        drop = False
        for name, val in values.items():
            token = f"%{name}%"
            if token in out:
                if not val:
                    drop = True
                    break
                out = out.replace(token, val)
        if not drop and out:
            pieces.append(out)
    # Juncao. Num idioma sem espaco a cola e vazia — MENOS quando o fim de um
    # pedaco e o comeco do seguinte sao os dois alfanumericos latinos. Sem esse
    # cuidado, "grupo 1" seguido de "1 hora" renderiza "小组11小时" e o "11"
    # deixa de ser dois numeros: vira um numeral que o proprio compilador nao
    # sabe cortar, porque a regra de fronteira trata numeral latino como
    # atomico — de proposito, para que "10" nunca se leia como "1" e "0".
    out = ""
    for piece in (p for p in pieces if p):
        if out:
            if lang.spaced:
                out += " "
            elif _alnum(out[-1]) and _alnum(piece[0]):
                out += " "
        out += piece
    return " ".join(out.split()) if lang.spaced else out.strip()
