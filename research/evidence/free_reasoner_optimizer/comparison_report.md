# Comparação de busca simbólica: enumeração versus política estocástica

**Status:** evidência host-only, determinística por seed, sem autoridade operacional.

## Objetivo

Comparar uma enumeração limitada com a política estocástica discreta do HERUS sob tarefas algébricas declaradas. A métrica de sucesso não é apenas erro zero no treino: o candidato também é avaliado em holdout e confrontado com o alvo formal conhecido somente pelo benchmark.

## Configuração

A enumeração usa `enumerate_terms()` com profundidade máxima 5 e limite de 768 termos. A política estocástica usa 128 episódios, 6 passos por episódio, tamanho máximo 13, exploração `epsilon=0.18`, atualização de valores de ação e seeds `0..4`. Ambos operam sobre o vocabulário explícito `x` e as operações `add`, `mul` e `neg`.

As tarefas principais usam treino em `x ∈ {-2,-1,0,1,2}` e holdout em `x ∈ {-7,3,4,9}` para `x²`, `x²+x` e `x²−x`. A tarefa `underdetermined_square` usa apenas `0 → 0` no treino e `x²` em holdout. Ela existe para testar a diferença entre ajuste amostral e generalização.

## Resultados

| Tarefa | Método | Treino sem erro | Holdout sem erro | Prova contra alvo | Avaliações |
|---|---|---:|---:|---:|---|
| `square` | enumeração | 1/1 | 1/1 | 1/1 | 3 |
| `square` | estocástico | 5/5 | 5/5 | 5/5 | 1, 13, 7, 7, 7 |
| `square_plus_x` | enumeração | 1/1 | 1/1 | 1/1 | 14 |
| `square_plus_x` | estocástico | 5/5 | 5/5 | 5/5 | 72, 104, 12, 128, 258 |
| `square_minus_x` | enumeração | 1/1 | 1/1 | 1/1 | 40 |
| `square_minus_x` | estocástico | 4/5 | 4/5 | 4/5 | 660, 768, 502, 670, 604 |
| `underdetermined_square` | enumeração | 1/1 | 0/1 | 0/1 | 1 |
| `underdetermined_square` | estocástico | 5/5 | 1/5 | 1/5 | 1, 1, 1, 1, 1 |

## Brecha encontrada

Na tarefa subdeterminada, todos os métodos conseguem obter erro zero no único exemplo de treino, mas somente uma das cinco seeds produz o alvo correto. As demais produzem candidatos que ajustam `0 → 0` e falham no holdout. Isto confirma uma regra central do HERUS: **erro zero em exemplos não é prova e não autoriza promoção**.

Também fica claro que a política estocástica não supera a enumeração neste primeiro benchmark. Para as três tarefas suficientemente observadas, a enumeração encontra os alvos em 3, 14 e 40 avaliações; a política usa de 1 a 768 avaliações e falha em uma de cinco seeds para `x²−x`. O ganho potencial da política ainda não foi demonstrado.

## Conclusão

A política estocástica é uma camada válida de exploração e reforço, mas nesta versão não deve substituir a enumeração como baseline nem ser usada como verificador. O próximo ciclo deve melhorar a função de recompensa, penalizar complexidade, reservar exemplos de validação durante a busca, comparar MCTS/beam search e medir custo sob o mesmo número de avaliações. O verificador formal continua obrigatório e independente.

O experimento é host-only. Nenhum resultado aqui mede ESP32, energia, latência de hardware ou generalização para linguagem natural.

## Reprodução

```bash
PYTHONPATH=research python3 -m free_reasoner.compare_search
```
