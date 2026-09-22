# Pesquisa HERUS

## Papel

`research/` é o laboratório host-only. Ele contém protótipos, contratos JSON, benchmarks, testes, manifests e evidências. Não deve ser interpretado como o produto físico.

## Ordem de leitura

1. `semantic_ir.py` e `finite_reasoner.py` — representação e raciocínio finito.
2. `symbiont_v2/` — identidade, host, world model, Skills e propostas.
3. `stage4/` — contratos de execução sintética e ledger.
4. `symbiosis_utility*` — contrato e classificador de utilidade.
5. `holdout_*` e `extended_holdout.py` — campanhas e limites negativos.
6. `bridge_*.py` — campanha causal isolada e HERUS Bridge.
7. `social/` — protocolo futuro sem dados humanos fabricados.
8. `test_*.py` — regressões e mutações que dão significado aos claims.
9. `evidence/` — resultados brutos, planos e auditorias.

## Verificação

```bash
make test
PYTHONPATH=research python3 -m research.bridge_causal
PYTHONPATH=research python3 -m research.bridge_product
```

## Regra de interpretação

Uma proposta não é um commit. Um contrato textual não é uma atestação. Uma fixture pública não é um holdout independente. Um protocolo social sem participantes não é um resultado humano.
