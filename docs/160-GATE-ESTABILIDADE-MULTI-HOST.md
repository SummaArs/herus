# Gate de estabilidade multi-hospedeiro

O HERUS já conseguia selecionar políticas internamente em cada hospedeiro. Isso não bastava para chamar a política alternativa de universal: MIntRec favoreceu a calibração, MInDS-14 empatou e Banking77 teve ganho pequeno que não sustentou o holdout.

## Regra nova

A política alternativa só pode ser promovida como política universal se:

1. existirem pelo menos três hospedeiros independentes;
2. cada hospedeiro tiver métricas de validação disjuntas do holdout;
3. a alternativa superar a política padrão por pelo menos **1 ponto percentual** em todos os hospedeiros;
4. caso contrário, o HERUS mantém a política padrão e registra `ABSTAIN`.

## Resultado atual

| Hospedeiro | Delta de validação | Passa margem de 1 pp? |
|---|---:|---:|
| MIntRec | +10,6918 pp | sim |
| MInDS-14 | 0 pp | não |
| Banking77 | +0,4000 pp | não |

Decisão: **ABSTAIN**. Política selecionada: `universal_default`.

Isso não melhora artificialmente a acurácia atual. Melhora a validade da afirmação: o HERUS agora sabe distinguir adaptação local de superioridade universal.

Não há claim de SOTA. O próximo avanço válido é aumentar a evidência ou demonstrar um candidato que passe a margem em novos hospedeiros independentes.
