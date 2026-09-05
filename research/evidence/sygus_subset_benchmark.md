# SyGuS — comparação semântica limitada

## Resultado

O runner foi executado sobre uma cópia do repositório público `SyGuS-Org/benchmarks`, obtida em 2026-09-01. Foram encontrados **9.719 arquivos `.sl`**. O inventário lexical anterior marcou 526 como candidatos LIA com uma única `synth-fun`; a tradução semântica restrita do HERUS aceitou efetivamente **8 instâncias**, todas repetidas entre anos ou entre `comp` e `lib`.

| Estado | Quantidade | Interpretação |
|---|---:|---|
| `BOUNDED_VERIFIED` | 8 | Um candidato satisfaz as restrições nos pontos inteiros de `[-2, 2]` para cada variável, sob profundidade 2 e limite de 500 candidatos |
| `UNKNOWN` | 518 | Sintaxe, comandos, gramática ou semântica fora do subconjunto; não é falha da instância nem prova de inexistência |

Os oito casos positivos são `commutative.sl` e `constant.sl` de 2017, 2018, 2019/from_2018 e `lib/from_2018`. Os candidatos encontrados foram, respectivamente, `(+ x y)` e `0`. A repetição confirma o alerta do repositório oficial de que podem existir erros ou duplicatas; portanto, a contagem não deve ser apresentada como oito problemas independentes.

## Contrato do experimento

O executor aceita uma única função sintetizada de retorno `Int`, uma gramática de um não-terminal `Int`, terminais inteiros e variáveis declaradas, operadores `+` e `-`, e restrições de igualdade. A avaliação é finita: cada variável percorre `-2..2`, a profundidade máxima é 2 e o orçamento é 500 candidatos. O estado `BOUNDED_VERIFIED` significa apenas que o candidato passou nesse domínio finito. Não é uma prova sobre `Int` ilimitado e não é comparável diretamente ao resultado de um solver SyGuS completo.

Recursos fora do contrato retornam `UNKNOWN`, incluindo `define-fun`, Booleanos, `ite`, quantificadores, múltiplos `synth-fun`, oráculos, BitVec, Array, String, desigualdades e gramáticas não lineares. O runner não executa texto produzido por solvers externos e não promove compatibilidade lexical a solução.

## Reprodução

```text
rm -rf /tmp/sygus-official-benchmarks
gh repo clone SyGuS-Org/benchmarks /tmp/sygus-official-benchmarks -- --depth 1
cd /tmp/herus-integration-all
PYTHONPATH=research python3 research/run_sygus_subset_benchmark.py
```

O JSON completo está em `research/evidence/sygus_subset_benchmark.json`. O benchmark não sustenta alegação de superioridade sobre SyGuS-Comp: ele demonstra apenas que o pipeline do HERUS consegue traduzir e verificar um subconjunto pequeno e reproduzível de instâncias públicas, com limites declarados.

## Fontes

[1]: https://sygus-org.github.io/language/ SyGuS-Org — Specification Language.
[2]: https://github.com/SyGuS-Org/benchmarks SyGuS-Org — public benchmark repository.
[3]: https://sygus-org.github.io/artifacts/ SyGuS-Org — Benchmarks & Tools.
