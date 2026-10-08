# Otimização de CPU e memória: modelo cacheado

Foi identificado um gargalo estrutural no benchmark: `_nb_scores(train, row)` reconstruía o Naive Bayes inteiro para cada exemplo. Em Banking77, com 6.966 exemplos de ajuste e 3.080 de holdout, isso repetia o ajuste milhares de vezes.

A implementação agora separa:

1. `fit_nb(train)`: constrói o modelo uma única vez;
2. `MultinomialNBModel.score(row)`: pontua cada exemplo sem refazer o ajuste;
3. representação compacta com IDs inteiros, arrays e penalização explícita para tokens desconhecidos;
4. o mesmo modelo ajustado é compartilhado entre as políticas default e calibrada;
5. um cache de features por `example_id` evita tokenização e vetorização repetidas;
6. PyTorch, Transformers, NumPy e o benchmark MInDS-14 não são importados no caminho Banking77.

## Medição real no Banking77

| Medida | Antes observado | Cacheado |
|---|---:|---:|
| Tempo de parede | ~1.605 s | **28,189 s** |
| Redução observada | — | **98,24%** |
| Aceleração observada | 1× | **~56,9×** |
| Pico de RSS | 824.084 KB | **46.312 KB** |

A comparação de velocidade usa a execução anterior real do mesmo benchmark como referência histórica; não é uma medição controlada no mesmo processo. Portanto, o número é evidência operacional forte, mas não uma publicação de performance definitiva.

A redução de RSS observada é um ganho de footprint do processo causado principalmente pelo isolamento dos imports pesados. O cache de features acrescenta memória limitada, subindo o RSS de cerca de 35 MiB para 46 MiB, mas reduz a latência do benchmark.

## Equivalência

Em todos os 3.080 exemplos reais de Banking77, usando 6.966 exemplos reais de ajuste, as previsões antiga e compactada foram idênticas: **0 divergências**.

As métricas do benchmark permaneceram:

- política padrão: 75,23%;
- score calibrado: 75,16%.

## Limite encontrado

A CPU e o footprint do processo melhoraram drasticamente. Ainda não há medição de energia nem comparação controlada de latência contra um transformer. O próximo passo é medir energia/latência em protocolo pareado e verificar o mesmo isolamento nos demais benchmarks.

Não há claim de SOTA geral.
