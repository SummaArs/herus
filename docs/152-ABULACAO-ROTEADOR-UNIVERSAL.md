# Ablação do roteador universal

A ablação compara o roteador HERUS com três referências: escolher sempre Naive Bayes, escolher sempre centróide e escolher aleatoriamente entre os dois. Também calcula o teto oracular, que conhece antecipadamente qual dos dois candidatos está correto.

| Dataset | Universal | Sempre NB | Sempre centróide | Aleatório | Teto oracular |
|---|---:|---:|---:|---:|---:|
| MInDS-14 | **92,86%** | 91,84% | 87,76% | 89,79% | 93,88% |
| MIntRec | **38,34%** | 36,53% | 36,01% | 36,26% | 51,81% |

## Interpretação

O universal não é equivalente a escolher aleatoriamente: em ambos os datasets supera a média aleatória e o melhor candidato fixo.

O teto oracular revela que existe espaço real para melhorar a coordenação: 93,88% no MInDS-14 e 51,81% no MIntRec. O HERUS atual alcança quase todo o teto no MInDS-14, mas ainda captura apenas parte dele em MIntRec.

Isso indica que o problema restante não é somente treinar candidatos melhores. É melhorar a estimativa de quando cada candidato deve ser confiado, especialmente sob drift temporal em MIntRec.

## Veredito

A ablação apoia a hipótese de que há ganho de roteamento real, mas não prova que o algoritmo é ótimo. Também mostra que a próxima melhoria deve atacar a calibração e a modelagem de incerteza, não simplesmente adicionar modelos.

Este documento é uma ablação científica e não um claim de SOTA geral.
