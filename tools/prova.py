"""prova.py — arnes adversarial. VENDORIZADO de ~/Python/kit_ferramentas/prova.py.

Copia deliberada: prove.sh tem de rodar num clone limpo do repositorio, sem
depender de nada fora dele. Se o kit evoluir, reimporte e diga isso no commit.
"""

from __future__ import annotations

import json
import time
import random
import traceback

__all__ = ["provar", "executar", "Relatorio"]


class Relatorio(dict):
    """dict com os campos: nome, tentativas, violacoes, caso_minimo, motivo,
    semente, segundos, por_segundo."""

    @property
    def ok(self) -> bool:
        return self["violacoes"] == 0

    def __str__(self) -> str:
        marca = "OK  " if self.ok else "FALHA"
        linha = (f"[{marca}] {self['nome']}  "
                 f"{self['tentativas']:,} tentativas  "
                 f"{self['violacoes']} violacoes  "
                 f"{self['segundos']:.2f}s  "
                 f"{self['por_segundo']:,.0f}/s").replace(",", ".")
        if self.ok:
            return linha
        return (linha
                + f"\n         motivo: {self['motivo']}"
                + f"\n         caso minimo: {json.dumps(self['caso_minimo'], ensure_ascii=False, default=str)}"
                + f"\n         reproduza: semente={self['semente']} indice={self['indice']}")


def _checar(invariante, caso):
    """Devolve None se o invariante foi respeitado, ou o motivo da violação."""
    try:
        r = invariante(caso)
    except Exception:
        return "excecao: " + traceback.format_exc(limit=2).strip().splitlines()[-1]
    if r is False or r is None:
        return "invariante devolveu falso"
    return None


def _tamanho(caso) -> int:
    """Medida grosseira de 'quão grande' é um contraexemplo, para encolher."""
    try:
        return len(json.dumps(caso, default=str))
    except Exception:
        return len(repr(caso))


def provar(nome, gerador, invariante, tentativas=10_000, semente=0,
           encolher=True, tentativas_encolhimento=400) -> Relatorio:
    """Busca adversarial por uma violação de `invariante`.

    gerador(rnd, escala) -> caso    (escala 0.01..1.0; varra o espaço com ela)
    invariante(caso) -> bool        (False ou exceção = violação)

    A escala sobe em rampa: casos pequenos primeiro (mais fáceis de ler quando
    quebram), extremos depois. Cada tentativa tem semente própria derivada de
    `semente`, então toda falha é reproduzível exatamente.
    """
    t0 = time.time()
    falha = None
    i = 0
    for i in range(tentativas):
        rnd = random.Random((semente << 24) ^ i)
        escala = (i % 100 + 1) / 100.0
        caso = gerador(rnd, escala)
        motivo = _checar(invariante, caso)
        if motivo:
            falha = (caso, motivo, i, escala)
            break

    if falha and encolher:
        falha = _encolher(gerador, invariante, falha, semente, tentativas_encolhimento)

    segundos = max(time.time() - t0, 1e-9)
    feitas = (i + 1) if tentativas else 0
    return Relatorio(
        nome=nome,
        tentativas=feitas,
        violacoes=0 if falha is None else 1,
        caso_minimo=None if falha is None else falha[0],
        motivo=None if falha is None else falha[1],
        indice=None if falha is None else falha[2],
        semente=semente,
        segundos=segundos,
        por_segundo=feitas / segundos,
    )


def _encolher(gerador, invariante, falha, semente, orcamento):
    """Procura o MENOR caso que ainda viola — contraexemplo pequeno é o que
    vira teste de regressão legível. Bisseção na escala + re-sorteios."""
    melhor_caso, melhor_motivo, melhor_i, escala = falha
    melhor_tam = _tamanho(melhor_caso)
    gasto = 0
    while escala > 0.005 and gasto < orcamento:
        escala /= 2.0
        achou = False
        for k in range(min(50, orcamento - gasto)):
            gasto += 1
            rnd = random.Random((semente << 24) ^ (0xE0C0 + gasto))
            caso = gerador(rnd, escala)
            motivo = _checar(invariante, caso)
            if motivo and _tamanho(caso) <= melhor_tam:
                melhor_caso, melhor_motivo, melhor_tam = caso, motivo, _tamanho(caso)
                achou = True
                break
        if not achou:
            break
    return (melhor_caso, melhor_motivo, melhor_i, escala)


def executar(relatorios, titulo="PROVA DE FOGO") -> int:
    """Imprime a tabela e devolve o código de saída (0 = tudo limpo).
    Use em CI: `raise SystemExit(executar([...]))`."""
    print(f"\n=== {titulo} ===")
    total = sum(r["tentativas"] for r in relatorios)
    ruins = [r for r in relatorios if not r.ok]
    for r in relatorios:
        print(r)
    print(f"--- {len(relatorios)} invariantes | {total:,} tentativas | "
          f"{len(ruins)} violados ---".replace(",", "."))
    return 1 if ruins else 0
