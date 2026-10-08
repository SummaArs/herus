# Curvas risco–cobertura reais

## Resultado

Foram calculadas curvas com 21 pontos para o selector HERUS e Naive Bayes em quatro domínios reais.

| Dataset/domínio | Cobertura alvo | HERUS | Naive Bayes | Leitura |
|---|---:|---:|---:|---|
| MIntRec S04 | ~25% | 0,6364 | 0,6364 | empate |
| MIntRec S04 | ~50% | **0,6207** | 0,4651 | HERUS superior |
| MIntRec S04 | ~75% | 0,3765 | **0,4219** | Naive Bayes superior |
| MIntRec S05 | ~25% | **0,7826** | 0,7292 | HERUS superior |
| MIntRec S05 | ~50% | **0,6860** | 0,6042 | HERUS superior |
| MIntRec S05 | ~75% | 0,4817 | **0,5139** | Naive Bayes superior |
| MIntRec S06 | ~25% | **0,6429** | 0,4667 | HERUS superior |
| MIntRec S06 | ~50% | **0,5000** | 0,4138 | HERUS superior |
| MIntRec S06 | ~75% | 0,3103 | **0,3256** | Naive Bayes superior |
| MInDS-14 | ~25% | 1,0000 | 1,0000 | empate |
| MInDS-14 | ~50% | 1,0000 | 1,0000 | empate |
| MInDS-14 | ~75% | 1,0000 | 1,0000 | empate |
| MInDS-14 | ~90% | 0,9765 | **0,9886** | Naive Bayes superior |

## Conclusão

A curva não é dominada integralmente pelo HERUS. O HERUS apresenta vantagem consistente em **cobertura baixa e média no MIntRec**, mas perde ou empata em cobertura alta. No MInDS-14, o comportamento é essencialmente empate em coberturas baixas e médias, com leve vantagem do Naive Bayes em cobertura alta.

Esse resultado é cientificamente valioso porque delimita o algoritmo:

> O HERUS não é um classificador geral SOTA. Ele é um selector de assurance que melhora a confiabilidade em regimes de cobertura controlada, especialmente sob drift temporal no MIntRec.

## Limite metodológico

O score atual do HERUS é uma composição do acordo Naive Bayes–centróide e da margem NB. Portanto, esta curva mede um **selector protótipo**, não o núcleo `SymbioticLearner` isolado.

A evidência completa está em `research/evidence/risk_coverage_curves_v1.json`.
