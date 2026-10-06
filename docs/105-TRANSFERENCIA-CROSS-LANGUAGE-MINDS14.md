# Transferência cross-language no MInDS-14

## Protocolo

- ajuste: configuração `en-US`, 416 exemplos;
- calibração: `en-US`, 147 exemplos;
- holdout externo: configuração `pt-PT`, 604 exemplos;
- 14 classes compartilhadas pelo próprio dataset;
- limiar escolhido somente na calibração inglesa;
- áudio não utilizado.

## Resultado

| Método | Cobertura no português | Precisão seletiva |
|---|---:|---:|
| Naive Bayes | 100% | 8,11% |
| Consenso NB + protótipo | 11,92% | 29,17% |

A calibração inglesa alcançou 98,53% de precisão seletiva com 92,52% de cobertura, mas isso não transferiu para o português. O consenso reduziu o risco em relação ao Naive Bayes, mas ainda ficou muito abaixo de uma política útil.

## Conclusão

O HERUS não possui transferência cross-language demonstrada. A política atual depende fortemente da representação lexical do hospedeiro. Este resultado invalida qualquer alegação de generalidade linguística e define o próximo problema de pesquisa: adaptação de representação usando dados não rotulados do hospedeiro-alvo, com nova calibração independente e sem usar seus rótulos de holdout.

## Limites

Não há áudio decodificado, independência por locutor verificada ou segundo par linguístico. As classes MInDS-14 não são eventos HERUS.

A evidência está em `research/evidence/minds14_cross_language_v1.json`.
