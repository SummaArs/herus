# Otimização de CPU: modelo cacheado

Foi identificado um gargalo estrutural no benchmark: `_nb_scores(train, row)` reconstruía o Naive Bayes inteiro para cada exemplo. Em Banking77, com 6.966 exemplos de ajuste e 3.080 de holdout, isso repetia o ajuste milhares de vezes.

A implementação agora separa:

1. `fit_nb(train)`: constrói o modelo uma única vez;
2. `MultinomialNBModel.score(row)`: pontua cada exemplo sem refazer o ajuste;
3. representação compacta com IDs inteiros, arrays e penalização explícita para tokens desconhecidos;
4. o mesmo modelo ajustado é compartilhado entre as políticas default e calibrada.

## Medição real no Banking77

| Medida | Antes observado | Cacheado |
|---|---:|---:|
| Tempo de parede | ~1.605 s | **35,627 s** |
| Redução observada | — | **97,78%** |
| Aceleração observada | 1× | **~45,1×** |
| Pico de RSS | não medido no processo anterior | 825.548 KB |

A comparação de velocidade usa a execução anterior real do mesmo benchmark como referência histórica; não é uma medição controlada no mesmo processo. Portanto, o número é evidência operacional forte, mas não uma publicação de performance definitiva.

## Equivalência

Em todos os 3.080 exemplos reais de Banking77, usando 6.966 exemplos reais de ajuste, as previsões antiga e compactada foram idênticas: **0 divergências**.

As métricas do benchmark permaneceram:

- política padrão: 75,23%;
- score calibrado: 75,16%.

## Limite encontrado

A CPU caiu drasticamente. Porém, o pico de RSS do processo completo permaneceu em aproximadamente 806 MiB; não há evidência suficiente de redução de memória porque o pipeline carrega dados e estruturas auxiliares. Isso impede declarar o HERUS economicamente resolvido. O próximo gargalo é medir o modelo isoladamente e transformar o carregamento em fluxo/streaming.

Não há claim de SOTA ou de eficiência energética geral.
