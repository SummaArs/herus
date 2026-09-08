#!/usr/bin/env python3
"""Red team do Babel: tira um controle, exige que a suite perceba.

Uma suite que passa prova que o codigo faz o que o codigo faz. Nao diz nada
sobre se as afirmacoes sao CARREGADORAS. Esta campanha desmonta o Babel um
controle por vez — cobertura, fronteira de palavra, atomicidade do numeral,
pre-varredura de classe protegida, contradicao de papel, canonicidade,
dominancia, negacao lexical, orcamento de leituras — reconstroi, e exige que
test_babel.c falhe. Mutante que sobrevive e controle que era decoracao.

Toda mutacao e uma edicao literal aplicada a uma copia descartavel da arvore.
Nada e alterado no lugar.
"""
import shutil
import subprocess
import sys
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
CORE = ROOT / "firmware" / "core"
NET = ROOT / "firmware" / "net"
SOURCES = ["crypto.c", "hir.c", "loom_core.c", "babel.c", "test_babel.c"]

# EXCLUSAO DECLARADA. Um mutante foi tirado desta lista de proposito, e o
# motivo fica escrito para que ninguem pense que ele foi escondido:
#
#   "dominancia: dois maximos passam a eleger o primeiro"
#   (o `if (best != NULL) return NULL;` em dominant())
#
# Aquele ramo e INALCANCAVEL por argumento, nao por falta de teste: os
# candidatos passam por hir_equal() antes de entrar no vetor, logo sao dois a
# dois diferentes; se h1 ≼ h2 e h2 ≼ h1 entao h1 == h2, o que contradiz a
# deduplicacao. E guarda defensiva de um caso impossivel. Cobra-lo aqui seria
# exigir que a suite matasse um mutante que nenhuma entrada alcanca — e
# ajustar a suite para "matar" isso seria teatro.

# (nome, arquivo, agulha, substituicao)
MUTANTS = [
    ("cobertura: um lexema desconhecido e pulado em silencio", "babel.c",
     """        if (found >= LOOM_BUCKET[lang][(uint8_t)folded[pos]].count) {
            if (depth == 0) break;
            depth--; continue;                /* retrocede */
        }""",
     """        if (found >= LOOM_BUCKET[lang][(uint8_t)folded[pos]].count) {
            if (pos + 1u < n) {
                stack[depth].pos = (uint16_t)(pos + 1u);
                stack[depth].off = 0; continue;
            }
            if (depth == 0) break;
            depth--; continue;
        }"""),

    ("recusa: a pre-varredura de classe protegida sai", "babel.c",
     "            if (e->cls == LOOM_SENS) { out->status = BABEL_E_SENSITIVE; return out->status; }",
     "            if (0) { out->status = BABEL_E_SENSITIVE; return out->status; }"),

    ("recusa: a pre-varredura de autoridade sai", "babel.c",
     "            if (e->cls == LOOM_AUTH) { out->status = BABEL_E_AUTHORITY; return out->status; }",
     "            if (0) { out->status = BABEL_E_AUTHORITY; return out->status; }"),

    ("fronteira: casamento no meio da palavra passa a valer", "babel.c",
     """        if (i > 0 && s[i - 1] != ' ') return 0;
        if (j < n && s[j] != ' ') return 0;
        return 1;""",
     """        (void)i; (void)j; (void)n; (void)s;
        return 1;"""),

    ("fronteira: o numeral latino deixa de ser atomico em escrita sem espaco",
     "babel.c",
     """    if (i > 0 && is_alnum(s[i - 1]) && is_alnum(s[i])) return 0;
    if (j < n && is_alnum(s[j - 1]) && is_alnum(s[j])) return 0;
    return 1;""",
     """    (void)i; (void)j; (void)n; (void)s;
    return 1;"""),

    ("render: o separador fino entre numerais sai", "babel.c",
     "        if (n > 0 && (sep || (is_alnum(buf[n - 1]) && is_alnum(piece[0])))) {",
     "        if (n > 0 && sep) {"),

    ("dominancia: preenchimento em conflito deixa de impedir a dominancia",
     "babel.c",
     "            if (b->slot[j].filler != a->slot[i].filler) return 0;",
     "            if (0) return 0;"),

    ("orcamento: estourar as leituras passa a ser aceito", "babel.c",
     "        if (truncated) { out->status = BABEL_E_AMBIGUOUS; return out->status; }",
     "        if (0) { out->status = BABEL_E_AMBIGUOUS; return out->status; }"),

    ("negacao: a forma lexical e ignorada no render", "babel.c",
     """            if (h->polarity && ROLES[k].role == HIR_ROLE_O_QUE) {
                const char *neg = loom_sym_neg(h->slot[i].filler, lang);
                if (neg) return neg;
            }""",
     "            if (0) { }"),

    ("negacao: a forma lexical deixa de carregar a polaridade", "babel.c",
     "            if (e->cls == LOOM_FILLNEG) h->polarity = 1u;",
     "            if (0) h->polarity = 1u;"),

    ("render: simbolo sem forma vira erro generico em vez de lacuna tipada",
     "babel.c",
     "                if (!val) return BABEL_E_GAP;   /* simbolo sem forma aqui */",
     "                if (!val) return BABEL_E_ARG;"),

    ("tipo: COMUNICAR deixa de exigir destinatario", "babel.c",
     "        if (!has_quem) { *missing_role = HIR_ROLE_QUEM;  return BABEL_E_INCOMPLETE; }",
     "        if (0) { *missing_role = HIR_ROLE_QUEM;  return BABEL_E_INCOMPLETE; }"),

    ("codificacao: byte fora do conjunto aceito e descartado em silencio",
     "babel.c",
     """            else if (c >= 0xf0u && c <= 0xf4u) width = 4;
            else return 1;""",
     """            else if (c >= 0xf0u && c <= 0xf4u) width = 4;
            else { i++; continue; }"""),

    ("papel: preenchimento em conflito sobrescreve em vez de recusar", "hir.c",
     "        return HIR_E_DUPLICATE_ROLE;                      /* conflicting fill */",
     "        return HIR_OK;"),

    ("papel: repetir o mesmo preenchimento passa a ser conflito", "hir.c",
     "        if (h->slot[i].filler == filler) return HIR_OK;   /* idempotent */",
     "        if (h->slot[i].filler == filler) return HIR_E_DUPLICATE_ROLE;"),

    ("canonicidade: os slots deixam de ser ordenados antes do digest", "hir.c",
     """    sort_slots(s, h->slot_count);

    out[n++] = 'H';""",
     "    out[n++] = 'H';"),
]


def build_and_run(work: Path) -> int:
    src = [str(work / f) for f in SOURCES]
    binary = work / "t_babel"
    cc = subprocess.run(["cc", "-Os", "-std=c11", "-I", str(work), *src,
                         "-o", str(binary)], capture_output=True, text=True)
    if cc.returncode != 0:
        return 2            # recusar compilar tambem e deteccao
    run = subprocess.run([str(binary)], capture_output=True, text=True)
    if run.returncode != 0 or "FAIL" in run.stdout:
        return 1
    return 0


def stage(tmp: Path) -> Path:
    work = tmp / "core"
    work.mkdir(parents=True, exist_ok=True)
    for f in CORE.glob("*.h"):
        shutil.copy(f, work / f.name)
    for f in NET.glob("*.h"):
        shutil.copy(f, work / f.name)
    for name in SOURCES:
        source = NET / name if (NET / name).exists() else CORE / name
        shutil.copy(source, work / name)
    return work


def main() -> int:
    with tempfile.TemporaryDirectory() as td:
        tmp = Path(td)
        if build_and_run(stage(tmp)) != 0:
            print("  FAIL  a suite base nao passa sem mutacao; nada abaixo significa nada")
            return 1
        print("  base: test_babel passa sem mutacao")

        killed, survived = 0, []
        for i, (name, filename, needle, replacement) in enumerate(MUTANTS):
            work = stage(tmp / f"m{i}")
            target = work / filename
            text = target.read_text()
            if needle not in text:
                print(f"  FAIL  mutacao obsoleta, a agulha nao existe mais: {name}")
                survived.append(name)
                continue
            target.write_text(text.replace(needle, replacement, 1))
            if build_and_run(work) != 0:
                killed += 1
            else:
                survived.append(name)
                print(f"  SOBREVIVEU  {name}")

        print(f"BABEL REDTEAM: {killed}/{len(MUTANTS)} mutantes detectados")
        if survived:
            print("  os controles a seguir nao sao carregadores:")
            for s in survived:
                print(f"    - {s}")
            return 1
        return 0


if __name__ == "__main__":
    raise SystemExit(main())
