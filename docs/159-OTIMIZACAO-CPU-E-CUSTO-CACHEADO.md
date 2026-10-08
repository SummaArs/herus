# Otimização de CPU: modelo cacheado

Foi identificado um gargalo estrutural no benchmark: `_nb_scores(train, row)` reconstruía o Naive Bayes inteiro para cada exemplo. Em Banking77, com 6.966 exemplos de ajuste e 3.080 de holdout, isso repetia o ajuste milhares de vezes.

A implementação agora separa:

1. `fit_nb(train)`: constrói o modelo uma única vez;
2. `MultinomialNBModel.score(row)`: pontua cada exemplo sem refazer o ajuste.

## Medição real no Banking77

| Medida | Antes observado | Cacheado |
|---|---:|---:|
| Tempo de parede | ~1.605 s | **41,364 s** |
| Redução observada | — | **97,42%** |
| Aceleração observada | 1× | **~38,8×** |
| Pico de RSS | não medido no processo anterior | 822.752 KB |

A comparação de velocidade usa a execução anterior real do mesmo benchmark como referência histórica; não é uma medição controlada no mesmo processo. Portanto, o número é evidência operacional forte, mas não uma publicação de performance definitiva.

## Equivalência

Em 50 exemplos reais de Banking77, usando 500 exemplos reais de ajuste, as previsões antiga e cacheada foram idênticas: **0 divergências**.

As métricas do benchmark permaneceram:

- política padrão: 75,23%;
- score calibrado: 75,16%.

## Limite encontrado

A CPU caiu drasticamente, mas o pico de memória cacheado foi aproximadamente 803 MiB. Isso impede declarar o HERUS economicamente resolvido. O próximo gargalo é reduzir a representação das contagens e evitar manter estruturas redundantes em memória, preservando equivalência bit a bit nas previsões.

Não há claim de SOTA ou de eficiência energética geral.
