# Seleção adaptativa de representação no MIntRec

## Hipótese

A queda do Tensor-Train no MIntRec mostrou que o problema principal não era apenas o rank, mas a representação. Testamos três representações treináveis: TF-IDF de palavras com SVM linear, TF-IDF de caracteres com SVM linear e TF-IDF de palavras com regressão logística.

## Protocolo sem vazamento

- treino inicial: temporada S04, 566 exemplos;
- seleção da representação: temporada S05, 1.272 exemplos;
- refit somente depois da seleção: S04+S05, 1.838 exemplos;
- avaliação final: temporada futura S06, 386 exemplos;
- S06 não foi consultada durante seleção ou refit.

## Resultado no holdout S06

| Método | Acurácia | Macro-F1 |
|---|---:|---:|
| Naive Bayes publicado anteriormente | 49,22% | 37,44% |
| Seleção adaptativa: SVM de palavras | **56,48%** | 47,13% |
| Regressão logística — referência oracle pós-holdout | 57,77% | 47,85% |
| SVM de caracteres — referência pós-holdout | 57,51% | 47,12% |

A política adaptativa escolheu SVM de palavras usando somente S05 e depois atingiu 56,48% no S06. Isso supera Naive Bayes em 7,26 pontos percentuais de acurácia, ou aproximadamente 14,7% em termos relativos.

## Interpretação honesta

É um avanço real de generalização temporal, mas não é vitória universal. O oracle que olha o holdout mostra que a política ainda não escolhe sempre a melhor representação. O resultado valida a direção de **adaptação de representação**, não a conclusão de que o HERUS superou todos os algoritmos.

A próxima melhoria deve usar seleção por estabilidade entre múltiplas partições de treino, sem transformar S06 em conjunto de ajuste.
