# Redes tensoriais no HERUS

## Hipótese

Uma rede tensorial pode representar um classificador com poucos parâmetros e baixa latência, tornando-se candidata para execução restrita. Isso não implica que ela compreenda melhor o domínio.

## Protocolo

Foi implementado um classificador Tensor-Train de dois núcleos, com entrada hash de 256 características e saída de 14 classes. O mesmo holdout real `PolyAI/MInDS-14 pt-PT` foi usado. O rank foi variado entre 2 e 32.

## Varredura

| Rank | Parâmetros | Acurácia | Inferência |
|---:|---:|---:|---:|
| 2 | 288 | 63,27% | 0,67 ms |
| 4 | 576 | 67,35% | 0,32 ms |
| 8 | 1.152 | 86,73% | 0,30 ms |
| 16 | 2.304 | 87,76% | 0,43 ms |
| 32 | 4.608 | 89,80% | 0,43 ms |
| **64** | **18.432** | **95,92%** | **1,04 ms** |

A Linear SVM do mesmo corpus alcançou 95,92%. O rank 64 agora empata a SVM em acurácia, supera sua macro-F1 (95,66% contra 95,53%) e tem inferência medida em 1,04 ms contra 6,70 ms, embora use mais parâmetros treináveis que o rank 32. Isso é uma vitória de custo–desempenho neste holdout, não uma vitória universal.

## Decisão

Redes tensoriais são promissoras como **camada de eficiência ou compressão**. O rank 64 atingiu o primeiro empate com um baseline clássico forte e melhorou latência e macro-F1 neste protocolo. Ainda não é superioridade geral: falta repetir em outros datasets, seeds e domínios, além de medir memória real e robustez a deriva. O híbrido encoder–tensorial anterior foi negativo, então não será promovido sem nova evidência.

## Limites

Um dataset textual, hash de 256 características, treinamento supervisionado local, cinco ranks e sem comparação ainda com uma implementação tensorial otimizada para hardware específico. Não é evidência de generalidade nem de superioridade universal.

Evidências: `research/evidence/tensor_network_minds14_v1.json` e `research/evidence/tensor_network_rank_sweep_minds14_v1.json`.
