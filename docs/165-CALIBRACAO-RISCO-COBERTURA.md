# Calibração risco–cobertura — Banking77

## Resultado

Foi implementada uma política que escolhe o maior conjunto de exemplos aceitos cujo limite superior de Wilson permanece abaixo de um alvo de risco. A calibração usa 1.519 exemplos separados; o holdout de 3.080 exemplos não escolhe a política.

O alvo de risco `0,20` não possui região segura na calibração. O menor limite superior observado foi aproximadamente `0,319` na unanimidade. Com alvo `0,32`, a política escolheu limiar de concordância `1` e cobertura de 100%.

| Ataque | Cobertura | Risco seletivo | Flip errado |
|---|---:|---:|---:|
| Caixa/pontuação | 100,00% | 25,78% | 5,39% |
| Typo | 100,00% | 31,66% | 15,91% |
| Deleção | 100,00% | 25,49% | 11,95% |
| Prefixo irrelevante | 100,00% | 37,27% | 26,27% |

## Interpretação científica

Este é um resultado negativo importante: a calibração limpa não transfere segurança para ataques. O seletor corretamente evita fabricar cobertura segura; ao usar um alvo compatível com a calibração, ele seleciona cobertura total, mas não robustez adversarial.

A unanimidade continua sendo a única política testada que reduz substancialmente os flips no stress test. O próximo avanço exige calibração em perturbações representativas e uma separação explícita entre risco limpo e risco adversarial.

**Claim permitido:** calibrador fail-closed reproduzível e diagnóstico de shift entre calibração limpa e ataques.

**Claims proibidos:** robustez certificada, SOTA, universalidade ou segurança de produção.
