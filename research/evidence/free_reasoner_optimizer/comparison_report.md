# Comparação de busca simbólica: enumeração, política estocástica, beam e MCTS

**Status:** evidência host-only, determinística por seed, sem autoridade operacional.

## Objetivo

Comparar quatro estratégias limitadas sobre o mesmo espaço algébrico: enumeração, política estocástica com atualização de valores de ação, beam search e MCTS. A métrica de sucesso não é apenas erro zero no treino. Cada candidato é avaliado em holdout e, no benchmark, confrontado com o alvo formal conhecido somente para avaliação externa.

## Configuração

A enumeração usa `enumerate_terms()` com profundidade máxima 5 e limite de 768 termos. A política estocástica usa 128 episódios, 6 passos por episódio, tamanho máximo 13, `epsilon=0.18` e seeds `0..4`. Beam usa largura 24 e limite de 768 avaliações. MCTS usa 128 simulações, profundidade máxima 6 e seeds `0..4`. Todos operam sobre o vocabulário explícito `x` e as operações `add`, `mul` e `neg`.

As tarefas principais usam treino em `x ∈ {-2,-1,0,1,2}` e holdout em `x ∈ {-7,3,4,9}` para `x²`, `x²+x` e `x²−x`. A tarefa `underdetermined_square` usa apenas `0 → 0` no treino e avalia contra `x²` no holdout. Ela existe para separar ajuste amostral de generalização.

## Resultados agregados

| Tarefa | Método | Treino sem erro | Holdout sem erro | Prova contra alvo | Avaliações |
|---|---|---:|---:|---:|---|
| `square` | enumeração | 1/1 | 1/1 | 1/1 | 3 |
| `square` | estocástico | 5/5 | 5/5 | 5/5 | 1, 13, 7, 7, 7 |
| `square` | beam | 1/1 | 1/1 | 1/1 | 1 |
| `square` | MCTS | 5/5 | 5/5 | 5/5 | 138, 145, 149, 138, 141 |
| `square_plus_x` | enumeração | 1/1 | 1/1 | 1/1 | 14 |
| `square_plus_x` | estocástico | 5/5 | 5/5 | 5/5 | 72, 104, 12, 128, 258 |
| `square_plus_x` | beam | 1/1 | 1/1 | 1/1 | 5 |
| `square_plus_x` | MCTS | 5/5 | 5/5 | 5/5 | 768, 272, 534, 278, 762 |
| `square_minus_x` | enumeração | 1/1 | 1/1 | 1/1 | 40 |
| `square_minus_x` | estocástico | 4/5 | 4/5 | 4/5 | 660, 768, 502, 670, 604 |
| `square_minus_x` | beam | 1/1 | 1/1 | 1/1 | 94 |
| `square_minus_x` | MCTS | 1/5 | 1/5 | 1/5 | 768, 768, 530, 768, 768 |
| `underdetermined_square` | enumeração | 1/1 | 0/1 | 0/1 | 1 |
| `underdetermined_square` | estocástico | 5/5 | 1/5 | 1/5 | 1, 1, 1, 1, 1 |
| `underdetermined_square` | beam | 1/1 | 1/1 | 1/1 | 1 |
| `underdetermined_square` | MCTS | 5/5 | 0/5 | 0/5 | 128, 128, 128, 128, 128 |

## Interpretação

Neste fragmento e nestes orçamentos, beam search foi o método mais eficiente para as três tarefas suficientemente observadas. Isso não demonstra superioridade geral: o espaço é pequeno, o vocabulário é explícito e a função de avaliação é exata. MCTS encontrou `x²−x` apenas em uma de cinco seeds e consumiu o limite em várias execuções. A política estocástica também não superou a enumeração de modo consistente.

O caso subdeterminado continua essencial. A enumeração e o MCTS falham no holdout; o estocástico acerta apenas uma seed; beam acerta por acaso neste benchmark porque o alvo externo é `x²` e a expressão aparece imediatamente na ordenação da fronteira. Isso não transforma uma única observação `0 → 0` em evidência suficiente. Na operação real, quando não houver alvo formal, o HERUS deve reportar ajuste e validação, nunca `proved`.

## Conclusão

O próximo ganho não virá de simplesmente adicionar mais aleatoriedade. A prioridade é uma política de seleção que use validação interna sem acessar o conjunto de prova, penalize complexidade e registre abstenção. Beam search é um baseline forte para o fragmento atual; MCTS precisa de uma função de valor melhor ou de um espaço de ações mais informativo. Nenhum método deve substituir o kernel exato.

O experimento é host-only. Nenhum resultado mede ESP32, energia, latência de hardware ou generalização para linguagem natural.

## Reprodução

```bash
PYTHONPATH=research python3 -m free_reasoner.compare_search
PYTHONPATH=research python3 -m unittest -v free_reasoner.test_search_methods
```
