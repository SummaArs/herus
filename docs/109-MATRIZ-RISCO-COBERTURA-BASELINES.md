# Matriz risco–cobertura contra baselines

## Protocolo

O mesmo MInDS-14 `pt-PT` foi dividido em 417 exemplos de ajuste, 89 de calibração e 98 de holdout. O limiar de cada modelo foi escolhido somente na calibração para atingir pelo menos 95% de precisão seletiva.

## Holdout

| Modelo | Precisão seletiva | Cobertura |
|---|---:|---:|
| Linear SVM | 95,92% | 100,00% |
| Logistic Regression | **96,88%** | **97,96%** |
| Random Forest | **98,95%** | **96,94%** |
| HERUS consenso | 94,38% | 90,82% |

## Conclusão

Mesmo no regime seletivo, o HERUS ainda não vence os baselines. Random Forest obteve o melhor resultado medido neste corpus, com 98,95% de precisão seletiva e 96,94% de cobertura. Portanto, o próximo algoritmo próprio precisa superar esse ponto ou demonstrar uma vantagem independente e verificável em transferência, custo, memória, latência ou adaptação sem rótulos.

Não é correto contar esta rodada como ganho de desempenho. Ela aumenta a qualidade da avaliação e estabelece o teto que o HERUS precisa superar.

Limites: um corpus textual, limiar calibrado em somente 89 exemplos, sem independência por locutor verificada e sem alegação de generalidade universal.

A evidência está em `research/evidence/selective_reference_matrix_minds14_v1.json`.
