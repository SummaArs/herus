# Otimização de CPU e memória: modelo cacheado

Foi identificado um gargalo estrutural no benchmark: `_nb_scores(train, row)` reconstruía o Naive Bayes inteiro para cada exemplo. Em Banking77, com 6.966 exemplos de ajuste e 3.080 de holdout, isso repetia o ajuste milhares de vezes.

A implementação final mantém:

1. `fit_nb(train)`: constrói o modelo uma única vez;
2. `MultinomialNBModel.score(row)`: pontua cada exemplo sem refazer o ajuste;
3. representação compacta com IDs inteiros, arrays e penalização explícita para tokens desconhecidos;
4. o mesmo modelo ajustado é compartilhado entre as políticas default e calibrada;
5. PyTorch, Transformers, NumPy e o benchmark MInDS-14 não são importados no caminho Banking77.

## Medição operacional

| Medida | Estado inicial | Caminho final |
|---|---:|---:|
| Tempo de parede observado | ~1.605 s | **35,726 s** |
| Redução observada | — | **97,77%** |
| Aceleração observada | 1× | **~44,9×** |
| Pico de RSS | 824.084 KB | **35.192 KB** |

A comparação de velocidade usa a execução anterior real do mesmo benchmark como referência histórica; não é uma medição controlada no mesmo processo. O ganho de RSS é atribuído principalmente ao isolamento dos imports pesados.

## Ablação controlada do cache de features

Foi testada uma alternativa que mantinha um vetor por `example_id` para evitar nova vetorização. A avaliação foi feita em processos separados, com o mesmo dataset real, os mesmos splits e a mesma região de medição:

| Variante | CPU | Parede | RSS | Acurácia default |
|---|---:|---:|---:|---:|
| Sem cache de features | 24,161 s | 24,169 s | 34.940 KB | 75,23% |
| Com cache de features | 25,341 s | 25,350 s | 46.240 KB | 75,23% |

Resultado: o cache de features foi **4,89% mais lento em CPU**, **4,82% mais lento em parede** e consumiu mais memória. As previsões foram iguais, mas a otimização foi rejeitada como padrão. O recurso permanece apenas como experimento reversível, não como claim de melhoria.

## Equivalência

Em todos os 3.080 exemplos reais de Banking77, usando 6.966 exemplos reais de ajuste, as previsões antiga e compactada foram idênticas: **0 divergências**.

As métricas permaneceram:

- política padrão: 75,23%;
- score calibrado: 75,16%.

## Limites

Ainda não há medição de energia nem comparação controlada de latência contra um transformer. O próximo passo é medir energia/latência em protocolo pareado e verificar o mesmo isolamento nos demais benchmarks.

Não há claim de SOTA geral.
