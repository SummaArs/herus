# Comparação entre paradigmas de ML em dados reais

## Dados e protocolo

Foi usado o MIntRec público, com 2.224 registros reais de texto e 20 intenções. O ajuste usa S04, a calibração de clusters usa S05 e o holdout temporal é S06.

Nenhum rótulo MIntRec foi transformado em evento HERUS.

## Resultados

| Paradigma/adaptador | Método | Acurácia | Macro-F1 | Cobertura |
|---|---|---:|---:|---:|
| Supervisionado | Naive Bayes multinomial | 49,22% | 37,44% | 100% |
| Supervisionado | 1-NN cosseno | 39,90% | 32,20% | 100% |
| Não supervisionado | K-means + mapeamento S05 | 31,35% | 15,99% | 100% |
| Reforço | Bandit contextual proxy | 13,21% | 12,52% | 99,48% |
| Memória simbiótica exata | Contexto finito | 6,22% | 11,88% | 6,22% |

## Auto-supervisionado

Não foi apresentado como resultado nesta rodada. O corpus não fornece um protocolo auto-supervisionado independente que seja comparável sem introduzir uma decisão metodológica arbitrária. Criar um “auto-supervisionado” apenas renomeando augmentation ou clustering seria maquiagem experimental.

O próximo protocolo auto-supervisionado deverá especificar:

1. tarefa pretexto sem rótulo;
2. divisão temporal;
3. representação aprendida;
4. congelamento antes da avaliação;
5. transferência para S06;
6. comparação contra os mesmos baselines.

## Limites do reforço

O bandit foi incluído como adaptador transparente. Seus “estados” são textos, suas “ações” são rótulos e sua recompensa é acerto de classificação. Isso não constitui um ambiente de reforço completo porque o MIntRec não possui transições causais, consequências de ação ou interação.

Logo, não é correto afirmar que o HERUS venceu ou perdeu o RL em geral. O resultado apenas mostra que esse proxy não é competitivo na tarefa textual escolhida.

## Estado científico

O HERUS ainda não vence os baselines supervisionados reais. O valor diferencial continua sendo a combinação de proposta, abstenção, memória verificável e controle de autoridade. Para competir em desempenho, ainda falta uma representação transferível que preserve essas garantias.
