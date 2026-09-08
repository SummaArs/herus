#!/usr/bin/env python3
"""check_links.py — nenhum link de documento aponta para o vazio.

Cento e seis documentos numa hierarquia de oito secoes so continuam
navegaveis se os links continuarem certos, e link quebrado e o tipo de defeito
que ninguem ve enquanto nao precisa da informacao — normalmente as duas da
manha. Entao isso vira invariante.

Verifica, em todo .md do repositorio:
  - todo link relativo para arquivo local existe
  - toda ancora #secao existe no arquivo apontado
  - nenhum documento em docs/ ficou orfao (sem ninguem apontando para ele),
    porque um documento que ninguem alcanca e um documento que ninguem le
"""
from __future__ import annotations

import pathlib
import posixpath
import re
import sys

ROOT = pathlib.Path(__file__).resolve().parent.parent
LINK = re.compile(r"\[[^\]]*\]\(([^)\s]+)\)")
HEAD = re.compile(r"^#{1,6}\s+(.+?)\s*$", re.M)

SKIP_DIRS = {".git", "build", "__pycache__", "node_modules"}
# Documentos que existem para serem citados de fora do repositorio, ou que sao
# indices; nao precisam de ninguem apontando para eles.
ROOT_OK = {"README.md", "SECURITY.md", "CONTRIBUTING.md", "LICENSE",
           "docs/INDEX.md", "sim/README.md"}


def slug(text: str) -> str:
    s = text.lower()
    s = re.sub(r"[`*_\[\]()]", "", s)
    s = re.sub(r"[^a-z0-9À-ɏ\s-]", "", s)
    # Um traco por espaco, SEM colapsar: e assim que o GitHub gera a ancora.
    # Colapsar produzia "phase-0-the-weekend" onde a ancora real e
    # "phase-0--the-weekend" (o travessao sai e deixa dois espacos), e o
    # verificador acusava um link que estava certo.
    return re.sub(r"\s", "-", s.strip())


def main() -> int:
    files = [p for p in ROOT.rglob("*.md")
             if not (set(p.parts) & SKIP_DIRS)]
    anchors = {}
    for p in files:
        anchors[p] = {slug(m.group(1)) for m in HEAD.finditer(
            p.read_text(encoding="utf-8"))}

    broken, referenced = [], set()
    for p in files:
        text = p.read_text(encoding="utf-8")
        for m in LINK.finditer(text):
            target = m.group(1)
            if target.startswith(("http://", "https://", "mailto:", "#")):
                continue
            path_part, _, anchor = target.partition("#")
            if not path_part:
                continue
            resolved = (p.parent / path_part).resolve()
            try:
                rel = resolved.relative_to(ROOT)
            except ValueError:
                broken.append(f"{p.relative_to(ROOT)}: link sai do repositorio: {target}")
                continue
            if not resolved.exists():
                broken.append(f"{p.relative_to(ROOT)}: aponta para o vazio: {target}")
                continue
            referenced.add(str(rel))
            if anchor and resolved.suffix == ".md":
                have = anchors.get(resolved)
                if have is not None and slug(anchor) not in have:
                    broken.append(f"{p.relative_to(ROOT)}: ancora inexistente: {target}")

    # A regra de orfao vale para a HIERARQUIA (docs/), nao para o repositorio
    # todo: research/ guarda notas de investigacao que existem para serem
    # lidas de fora, e cobrar link para cada uma seria burocracia disfarcada de
    # invariante. Mas elas sao CONTADAS e mostradas, porque nota que ninguem
    # alcanca tambem e um custo — so nao e um erro.
    orphans, loose = [], []
    for p in files:
        rel = str(p.relative_to(ROOT))
        if rel in ROOT_OK or rel in referenced:
            continue
        (orphans if rel.startswith("docs/") else loose).append(rel)

    total_links = sum(len(LINK.findall(p.read_text(encoding="utf-8"))) for p in files)
    if broken or orphans:
        print(f"DOC LINKS: FAIL {len(broken)} quebrados, {len(orphans)} orfaos "
              f"em {len(files)} documentos")
        for b in broken[:20]:
            print("  " + b)
        for o in orphans[:20]:
            print(f"  orfao (ninguem aponta para ele): {o}")
        return 1
    print(f"DOC LINKS: PASS {len(files)} documentos, {total_links} links, "
          f"0 quebrados, 0 orfaos na hierarquia")
    if loose:
        print(f"  nota: {len(loose)} notas fora de docs/ sem ninguem apontando "
              f"para elas (permitido, mas contado)")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
