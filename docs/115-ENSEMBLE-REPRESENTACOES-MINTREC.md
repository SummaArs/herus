# Ensemble adaptativo de representações no MIntRec

## Protocolo

Foram treinados três candidatos em S04, selecionados em S05 e refitados em S04+S05 antes da avaliação no holdout temporal S06:

- SVM de palavras;
- SVM de caracteres;
- regressão logística de palavras;
- votação palavra+caractere;
- votação dos três modelos.

S06 permaneceu oculto durante a seleção.

## Resultado

| Modelo | Acurácia S06 | Macro-F1 |
|---|---:|---:|
| SVM de palavras escolhido | 56,48% | 47,13% |
| SVM de caracteres | 57,51% | 47,12% |
| Regressão logística | **57,77%** | **47,85%** |
| Votação palavra+caractere | 56,48% | 47,13% |
| Votação dos três | 57,25% | 47,77% |

## Conclusão

A diversidade de representações não produziu ganho automático. A votação dos três modelos ficou abaixo da regressão logística. O ensemble é preservado como baseline e evidência negativa, mas não é incorporado como melhoria principal.

A próxima hipótese deve usar pesos calibrados e diversidade de confiança, não apenas voto majoritário. Qualquer ganho deverá ser confirmado em holdout temporal independente.
