# Seleção de política por hospedeiro

A calibração de score melhorou MIntRec, mas piorou MInDS-14. Em vez de escolher uma política global, o HERUS recebeu uma camada de seleção por hospedeiro.

## Protocolo aninhado

No MIntRec, a temporada S05 foi dividida deterministicamente em duas partes:

- tune: 636 exemplos, usados para ajustar cada política;
- validation: 636 exemplos, usados para escolher entre o roteador padrão e a calibração de score;
- holdout S06: 386 exemplos, preservados até a decisão final.

A escolha não acessou os rótulos do holdout.

## Resultado da seleção

| Política | Validação interna | Holdout |
|---|---:|---:|
| Roteador padrão | 33,65% | 38,34% |
| Score calibrado | **44,34%** | **39,64%** |
| Política escolhida | score calibrado | score calibrado |

A política escolhida manteve a vantagem no holdout, elevando o resultado em 1,30 ponto percentual sobre o roteador padrão.

## Veredito

Este é um avanço algorítmico real: o HERUS não usa mais obrigatoriamente uma única política de seleção. Ele pode escolher uma política de acordo com o hospedeiro, usando somente evidência de ajuste e validação.

O mecanismo ainda não foi promovido como padrão global. A seleção foi demonstrada em MIntRec e precisa ser replicada em MInDS-14 e em um terceiro dataset antes de ser considerada generalizável.

Não há claim de SOTA geral.
