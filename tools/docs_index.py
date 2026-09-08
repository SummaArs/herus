#!/usr/bin/env python3
"""docs_index.py — reconstroi docs/INDEX.md a partir do que existe em disco.

Um indice escrito a mao envelhece na primeira vez que alguem acrescenta um
documento e esquece a linha. Este le a hierarquia, tira o titulo do proprio
arquivo, e escreve o indice. `--check` falha se o indice no disco nao for o que
seria gerado agora — e e essa forma que o prove.sh usa.
"""
from __future__ import annotations

import argparse
import pathlib
import re
import sys

ROOT = pathlib.Path(__file__).resolve().parent.parent
DOCS = ROOT / "docs"

SECTIONS = {
    "10-arquitetura":  "Arquitetura, protocolo e fronteira física",
    "20-produto":      "Produto, mercado e adoção",
    "25-significado":  "A camada do significado: interlíngua, compilador, portadores",
    "30-memoria":      "Memória pessoal soberana",
    "40-voz-haptica":  "Voz, háptica e interação",
    "50-inteligencia": "Inteligência local limitada",
    "60-hardware":     "Bancada, hardware e Fase 0",
    "70-validacao":    "Validação, ameaça e campanhas adversariais",
}

# Os documentos que definem o que o HERUS e agora. Vao no topo do indice
# porque um indice em ordem de numero e uma lista; um indice em ordem de
# importancia e um mapa.
FIRST = [
    "25-significado/200-HERUS-A-CAMADA-DO-SIGNIFICADO.md",
    "25-significado/201-HERUS-LOOM-O-TEAR.md",
    "25-significado/202-HERUS-BABEL-INTERLINGUA-FECHADA.md",
    "25-significado/203-HERUS-AETHER-SOM-E-LUZ.md",
    "10-arquitetura/00-HERUS-MASTER.md",
    "20-produto/04-PRODUCT.md",
    "60-hardware/03-BUILD-GUIDE.md",
]


def title_of(path: pathlib.Path) -> str:
    for line in path.read_text(encoding="utf-8").splitlines():
        s = line.strip()
        if s.startswith("# "):
            return re.sub(r"\s+", " ", s[2:].strip())
    return path.stem


def render() -> str:
    out = ["# Índice dos documentos", "",
           "Gerado por `tools/docs_index.py`. `tools/check_links.py` prova que",
           "nenhum link aqui aponta para o vazio e que nenhum documento da",
           "hierarquia ficou órfão.", ""]
    total = 0
    out.append("## Comece por aqui")
    out.append("")
    for rel in FIRST:
        p = DOCS / rel
        if p.exists():
            out.append(f"- [{title_of(p)}]({rel})")
    out.append("")
    for sec, desc in SECTIONS.items():
        d = DOCS / sec
        if not d.is_dir():
            continue
        items = sorted(d.glob("*.md"))
        if not items:
            continue
        total += len(items)
        out.append(f"## {desc}")
        out.append("")
        out.append(f"`docs/{sec}/` — {len(items)} documentos")
        out.append("")
        for p in items:
            out.append(f"- [{title_of(p)}]({sec}/{p.name})")
        out.append("")
    out.insert(5, f"São {total} documentos em {len(SECTIONS)} seções.")
    out.insert(6, "")
    return "\n".join(out) + "\n"


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--check", action="store_true")
    args = ap.parse_args()
    want = render()
    dest = DOCS / "INDEX.md"
    if args.check:
        have = dest.read_text(encoding="utf-8") if dest.exists() else ""
        if have != want:
            print("DOC INDEX: FAIL docs/INDEX.md nao e o que o gerador produz — "
                  "rode python3 tools/docs_index.py")
            return 1
        print(f"DOC INDEX: PASS o indice bate com a hierarquia em disco")
        return 0
    dest.write_text(want, encoding="utf-8")
    print(f"DOC INDEX: escrito ({len(want.splitlines())} linhas)")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
