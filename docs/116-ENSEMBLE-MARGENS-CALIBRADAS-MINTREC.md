# Ensemble de margens calibradas no MIntRec

## Hipótese

Voto majoritário descarta a intensidade da evidência. A hipótese foi combinar as margens de decisão de um SVM lexical e de um SVM de caracteres com pesos selecionados em S05.

## Protocolo

- ajuste inicial em S04;
- busca de 120 pares de pesos em S05;
- pesos escolhidos: palavra `0,7`, caractere `0,4`;
- refit em S04+S05;
- avaliação única em S06.

## Resultado

| Método | Acurácia S06 | Macro-F1 |
|---|---:|---:|
| SVM de palavras | 56,48% | 47,13% |
| Votação dos três modelos | 57,25% | 47,77% |
| **Margens calibradas** | **56,48%** | **48,02%** |
| Regressão logística | 57,77% | 47,85% |

## Conclusão

A calibração não elevou a acurácia, mas elevou o macro-F1 de 47,13% para 48,02%, uma melhora de 0,89 ponto percentual. Isso indica distribuição mais equilibrada entre classes, sem superioridade global.

O resultado é contado como melhoria parcial de qualidade, não como vitória contra os baselines. A falha inicial do experimento também foi corrigida no harness: uma matriz de margens estava sendo indexada como mapa de classes.
