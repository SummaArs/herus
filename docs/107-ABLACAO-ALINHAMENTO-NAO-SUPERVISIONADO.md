# Ablação: alinhamento global não supervisionado

Foi testada uma adaptação sem rótulos que desloca os centroides ingleses pela diferença entre a média global dos embeddings ingleses e portugueses.

## Resultado

| Método | Acurácia PT | Cobertura |
|---|---:|---:|
| Encoder multilíngue congelado | 48,68% | 99,01% |
| + alinhamento de média | 48,68% | 99,01% |

A diferença foi zero no holdout. O procedimento observou textos portugueses sem seus rótulos, mas não alterou a decisão de maneira útil.

## Decisão

A hipótese de que uma correção global de distribuição resolveria a transferência foi rejeitada. O próximo desenho deve adaptar estrutura local — por exemplo, protótipos contrastivos ou um léxico de equivalências descoberto sem rótulo — e deverá ser comparado ao encoder congelado em holdout intocado.

A evidência está em `research/evidence/unsupervised_alignment_minds14_v1.json`.
