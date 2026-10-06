# Avaliador real de programação v1

## Objetivo

Até esta rodada, a habilidade de programação só produzia propostas e o ledger fechava evidências. Agora existe um avaliador de pesquisa separado que executa um candidato explícito em um subprocesso temporário, com limite de tempo e casos de teste fornecidos.

A separação é intencional:

```text
biblioteca pública: proposal-only
harness de pesquisa: executa tarefa explícita
ledger: registra autoridade e resultado
```

## Resultado reproduzido

O ensaio v1 contém três casos:

| Caso | Resultado |
|---|---|
| candidato correto | `pass` |
| candidato incorreto | `fail` |
| candidato não terminante | `timeout` |

A autoridade registrada é `isolated-subprocess`. O resultado não é tratado como prova universal: ele só fecha o caso específico executado.

A evidência bruta está em `research/evidence/programming_evaluator_v1.json`.

## Limites

Este avaliador ainda não é um sandbox de segurança. Ele usa subprocesso, `-I`, diretório temporário, stdin fechado, ambiente vazio e timeout, mas não promete isolamento contra um adversário nativo. Não deve receber código não confiável fora de um ambiente apropriado.

Também não mede ainda:

- cobertura de um repositório real;
- testes ocultos de terceiros;
- qualidade de patch;
- segurança semântica;
- custo de inferência;
- comparação contra transformer ou agente de programação.

## Critério de avanço

A próxima campanha deve fornecer tarefas reais versionadas, um conjunto de testes públicos para desenvolvimento e testes ocultos para avaliação. A vitória será medida por correção funcional sob orçamento, não por aparência do código ou quantidade de testes locais.
