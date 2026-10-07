# Matriz de baselines ML no MInDS-14

## Protocolo

Todos os modelos foram avaliados na configuração real `pt-PT` do PolyAI/MInDS-14, usando os mesmos 417 exemplos de ajuste e 98 exemplos de holdout determinístico. A métrica principal é acurácia no holdout; macro-F1, cobertura e tempo também foram registrados.

## Resultados

| Modelo | Acurácia | Macro-F1 | Treino (ms) | Inferência (ms) |
|---|---:|---:|---:|---:|
| TF-IDF + Logistic Regression | 94,90% | 94,64% | 1.350,22 | 4,81 |
| **TF-IDF + Linear SVM** | **95,92%** | **95,53%** | **88,58** | 6,70 |
| TF-IDF + k-NN | 93,88% | 93,48% | 23,95 | 5,90 |
| TF-IDF + Random Forest | **95,92%** | **95,36%** | 418,01 | 15,72 |
| Naive Bayes HERUS baseline | 91,84% | 89,29% | — | — |

O consenso HERUS seletivo teve 94,38% de precisão seletiva e 90,82% de cobertura neste mesmo domínio, mas não deve ser comparado diretamente à acurácia total sem explicitar a abstenção.

## Conclusão crítica

O HERUS ainda não vence os baselines clássicos fortes neste problema. A Linear SVM é aproximadamente 4,08 pontos percentuais superior ao Naive Bayes usado como baseline local. Random Forest e SVM têm a melhor acurácia medida.

Isso define um alvo real: qualquer nova versão do Symbiotic Learning precisa superar pelo menos a Linear SVM em holdout independente, ou demonstrar uma vantagem que acurácia não captura — por exemplo, risco seletivo, adaptação sem rótulos, custo de memória ou transferência entre hosts — sem esconder a comparação.

## Limites

O benchmark usa texto, um corpus e uma partição sem independência por locutor verificada. Não prova superioridade universal nem compara ainda com todos os algoritmos de ML existentes. A alegação correta é: **matriz clássica real executada; HERUS ainda não venceu**.

A evidência bruta está em `research/evidence/ml_reference_matrix_minds14_v1.json`.
