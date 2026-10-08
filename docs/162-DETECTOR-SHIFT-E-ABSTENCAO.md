# Detector de shift e abstention — Banking77

## Veredito

**Ablação promissora, mas não promovida ao caminho padrão.**

O HERUS foi executado no holdout real do Banking77 usando um detector lexical calibrado somente nos 1.519 exemplos de validação. Nenhum rótulo do holdout foi usado para escolher o limiar.

| Ataque | Acurácia atacada sem gate | Precisão dos aceitos | Cobertura | Wrong-label flip |
|---|---:|---:|---:|---:|
| Caixa/pontuação | 68,93% | 73,82% | 93,38% | 2,55% |
| Deleção | 62,73% | 75,03% | 83,60% | 4,67% |
| Prefixo irrelevante | 54,16% | 61,75% | 87,69% | 17,32% |
| Typo | 56,88% | 67,54% | 84,22% | 10,67% |

Os valores de precisão são condicionais aos casos aceitos; abstenções não são acertos automáticos.

## Interpretação

O detector reduz mudanças erradas de rótulo em todos os quatro stress tests, mas paga por isso com abstention de 6,62% a 16,40%. No prefixo irrelevante, o risco condicional permanece alto. Portanto, o resultado é uma melhora de assurance, não uma prova de robustez.

## Limites

- As transformações continuam sem adjudicação semântica humana;
- não há claim de ataques label-preserving;
- o detector é lexical e pode falhar em drift sem mudança de vocabulário;
- não foi promovido ao caminho padrão antes de validação em outros domínios;
- não há claim de SOTA.

Próximo teste: repetir o mesmo contrato em MIntRec, MInDS-14 e um conjunto OOS, mantendo o limiar congelado antes de cada holdout.
