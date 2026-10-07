# Tensor-Train no MIntRec temporal

## Protocolo

O mesmo desenho Tensor-Train usado no MInDS-14 foi aplicado a outro problema real: classificação textual de 20 intenções no `THU-IAR/MIntRec`. O treino usou as temporadas S04 e S05; o holdout foi a temporada futura S06, com 386 exemplos. A entrada teve 1.024 características hash e a cabeça teve saída de 20 classes.

## Resultado

| Modelo | Acurácia | Macro-F1 | Cobertura |
|---|---:|---:|---:|
| Tensor-Train rank 32 | 39,90% | 35,45% | 100% |
| **Tensor-Train rank 64** | **42,23%** | **37,03%** | **100%** |
| 1-NN cosseno | 39,90% | 32,20% | 100% |
| Naive Bayes multinomial | **49,22%** | **37,44%** | 100% |
| K-means calibrado | 31,35% | 15,99% | 100% |
| Transformer pequeno | 17,62% | 3,96% | 100% |
| Bandit contextual proxy | 11,92% | 10,94% | 99,48% |

## Conclusão

O Tensor-Train supera o K-means, o transformer pequeno e o proxy de reforço neste protocolo, mas perde para o Naive Bayes em acurácia e macro-F1. Mais importante: o resultado de 95,92% no MInDS-14 não transferiu para o MIntRec. O empate anterior era dependente do domínio.

A barreira principal agora é **adaptação de representação entre domínios e deriva temporal**, não apenas aumentar o rank. A próxima pesquisa deve testar uma representação híbrida que aprenda a geometria do domínio sem acessar o holdout S06.

Limites: texto apenas; MIntRec não é mapeado para eventos HERUS; uma temporada de holdout; baselines heterogêneos já publicados em protocolos próximos, não uma competição perfeitamente reexecutada no mesmo código.
