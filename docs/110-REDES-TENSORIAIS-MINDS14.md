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

A Linear SVM do mesmo corpus alcançou 95,92%. Portanto, o melhor tensorial ainda perde 6,12 pontos percentuais, mas usa uma representação extremamente compacta e tem inferência sub-milisegundo.

## Decisão

Redes tensoriais são promissoras como **camada de eficiência ou compressão**, não como prova de superioridade do HERUS. A próxima hipótese útil é um modelo híbrido: encoder/protótipos simbióticos para selecionar a representação e núcleo tensorial pequeno para inferência. Esse híbrido só contará como avanço se mantiver o holdout intocado e superar a fronteira de custo-desempenho dos baselines.

## Limites

Um dataset textual, hash de 256 características, treinamento supervisionado local, cinco ranks e sem comparação ainda com uma implementação tensorial otimizada para hardware específico. Não é evidência de generalidade nem de superioridade universal.

Evidências: `research/evidence/tensor_network_minds14_v1.json` e `research/evidence/tensor_network_rank_sweep_minds14_v1.json`.
