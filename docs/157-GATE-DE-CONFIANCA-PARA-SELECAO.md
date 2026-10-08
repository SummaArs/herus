# Gate de confiança para seleção de política

A falha do Banking77 mostrou que uma vantagem pontual na validação não é suficiente para trocar de política. Foi adicionado um gate baseado em bootstrap pareado por exemplo.

## Regra

A política alternativa só pode ser escolhida quando o limite inferior do intervalo de confiança de 95% para a diferença pareada de acurácia for estritamente maior que zero.

Caso contrário, o HERUS mantém a política padrão.

## Banking77

| Métrica | Resultado |
|---|---:|
| Diferença observada a favor da alternativa | +0,395 ponto percentual |
| Limite inferior de 95% | **−0,066 ponto percentual** |
| Limite superior de 95% | +0,922 ponto percentual |
| Política escolhida | **padrão** |
| Acurácia no holdout | **75,23%** |

A diferença não foi estatisticamente separável de zero. O seletor se absteve de trocar de política e evitou a regressão para 75,16% observada na seleção pontual anterior.

## Importância para a universalidade

A universalidade do HERUS agora inclui uma forma explícita de não agir quando a evidência é insuficiente. Isso é essencial para um algoritmo adaptativo: adaptar-se não significa sempre mudar; significa mudar somente quando a evidência suportar a mudança.

O gate ainda precisa ser replicado nos três hospedeiros com múltiplas seeds e métricas adicionais. Não há claim de SOTA geral.
