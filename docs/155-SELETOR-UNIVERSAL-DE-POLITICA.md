# Seletor universal de política por hospedeiro

A seleção aninhada foi replicada nos dois datasets reais. A regra compara o roteador padrão com a política de scores calibrados usando apenas ajuste e validação internos; o holdout permanece intocado.

| Hospedeiro | Política escolhida | Validação | Holdout escolhido | Holdout padrão |
|---|---|---:|---:|---:|
| MInDS-14 | padrão | empate: 84,44% | **92,86%** | 92,86% |
| MIntRec | score calibrado | 44,34% contra 33,65% | **39,64%** | 38,34% |

## Interpretação

O seletor não aplica a calibração de score indiscriminadamente. Em MInDS-14, as políticas empataram na validação e a regra de desempate preservou o roteador padrão; no holdout, isso evitou a regressão para 91,84% observada quando a calibração era aplicada globalmente.

Em MIntRec, a validação favoreceu claramente a política calibrada, e a vantagem permaneceu no holdout.

Esse resultado apoia uma propriedade central do HERUS como algoritmo universal:

> **A universalidade não significa usar a mesma política em todos os ambientes; significa possuir um procedimento comum, verificável e limitado para descobrir qual política é segura em cada hospedeiro.**

## Limites

A seleção foi demonstrada em dois datasets de classificação de intenção. Ainda faltam um terceiro domínio, candidatos mais fortes, custo computacional e comparação com políticas aprendidas mais complexas. Portanto, o resultado não é claim de SOTA geral.
