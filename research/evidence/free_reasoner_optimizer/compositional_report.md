# Campanha composicional de busca simbólica

**Status:** evidência host-only, com kernel exato e autoridade operacional inexistente.

## Configuração

Foram adicionadas três tarefas ao comparador existente: `x³+x`, `x³+x²` e `(x²−x)(x+x)`. O treino usa `x ∈ {-2,-1,0,1,2}` e o holdout usa `x ∈ {-7,3,4,9}`. O limite permanece em 768 avaliações para enumeração, beam e políticas limitadas; as políticas estocásticas e MCTS usam cinco seeds.

## Resultados

| Tarefa | Enumeração | Estocástico | Beam | MCTS |
|---|---:|---:|---:|---:|
| `cube_plus_x` | 1/1 provado, 80 avaliações | 5/5 provados, 15–183 avaliações | 1/1 provado, 14 avaliações | 3/5 provados, 408–768 avaliações |
| `cube_plus_square` | 1/1 provado, 81 avaliações | 1/5 provado, 165–768 avaliações | 1/1 provado, 15 avaliações | 3/5 provados, 658–768 avaliações |
| `nested_difference` | 1/1 provado, 604 avaliações | 0/5, limite esgotado | 0/1, 255 avaliações | 0/5, limite esgotado |

A notação `n/k provados` significa que `n` execuções produziram candidato com erro zero no treino e holdout e que passou pelo `Prover`, de `k` execuções. Para `nested_difference`, o alvo exige reutilização de uma subárvore; a gramática linear atual não possui uma pilha de subtermos e a falha é esperada.

## Interpretação

A composição univariada mais profunda (`x³+x` e `x³+x²`) é alcançável por todos os métodos em parte das condições. Beam continua sendo o melhor método neste espaço porque sua ordenação por erro exato encontra rapidamente expressões curtas. A política estocástica melhora em `x³+x`, mas perde consistência em `x³+x²`. MCTS ainda é caro e dependente de seed.

O resultado mais importante é negativo: aumentar a profundidade do alvo não basta para obter raciocínio composicional geral. A tarefa `nested_difference` exige memória estrutural de subexpressões, e todos os métodos exceto enumeração falham no orçamento atual. Isso aponta para uma mudança concreta necessária: ações que criem, armazenem e reutilizem subtermos, com tipos e limites de memória, em vez de somente aplicar operações sobre o termo inteiro atual.

Nenhuma execução autoriza promoção automática. O alvo formal é conhecido pelo benchmark, não pelo agente; em uso real sem alvo, apenas treino e validação podem ser reportados. O resultado não mede linguagem natural, grounding ou ESP32.

## Reprodução

```bash
PYTHONPATH=research python3 -m free_reasoner.compare_search
PYTHONPATH=research python3 -m unittest -v free_reasoner.test_search_methods
```
