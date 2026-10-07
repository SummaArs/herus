# Seleção estável de representação — resultado

A seleção adaptativa foi endurecida com duas validações independentes antes do holdout:

1. validação temporal interna de S04: 186 exemplos dos episódios posteriores;
2. temporada S05: 1.272 exemplos;
3. refit em S04+S05: 1.838 exemplos;
4. holdout final S06: 386 exemplos.

A escolha foi feita por média e pior caso de acurácia nas validações. O SVM de palavras foi escolhido porque obteve o maior pior caso: 39,78%, contra 36,87% do SVM de caracteres e 33,10% da regressão logística.

No holdout S06, após refit:

| Método | Acurácia | Macro-F1 |
|---|---:|---:|
| SVM de palavras escolhido | 56,48% | 47,13% |
| SVM de caracteres | 57,51% | 47,12% |
| Regressão logística | 57,77% | 47,85% |

## Resultado científico

A seleção estável não elevou o teto em relação à rodada adaptativa anterior. Isso não é uma falha do protocolo: demonstra que a estabilidade interna não é suficiente para identificar a melhor representação no domínio futuro.

O próximo experimento deve combinar estabilidade com diversidade de erro — por exemplo, um ensemble calibrado entre palavra, caractere e representação tensorial — sempre mantendo S06 intocado.
