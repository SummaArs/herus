# Auditoria de `simbionte-1`

O repositório paralelo foi clonado e executado em 15 de setembro de 2026. Ele possui **170 testes passando** e apresenta ideias úteis, mas não deve ser fundido inteiro: sua implementação é um segundo runtime Python grande, com sobreposição de descoberta, memória, LLM, confiança, leases e causalidade já existentes no HERUS.

## Incorporação

Foi incorporado o contrato `research/action_lease.py`. Ele usa lease exclusivo por `action_id`, expiração monotônica e fencing token crescente. Uma instância antiga não consegue renovar ou liberar o lease depois que outro dono o adquiriu. O lease apenas serializa coordenação; não concede autoridade nem permite execução.

Essa propriedade é relevante para o ASA porque um wearable, celular, notebook ou servidor pode perder conexão, migrar e retornar. Sem fencing, uma instância antiga poderia tentar continuar uma proposta depois do rebind.

## Ideias aproveitadas conceitualmente

A auditoria confirmou como úteis e compatíveis com o HERUS: quarentena de fontes conflitantes, experimentos causais reversíveis, reconciliação após resposta perdida, migração sem segredos, memória de hipóteses separada de fatos e avaliação externa ao núcleo.

O HERUS já cobre parte dessas propriedades com proveniência, conflito de `WorldModel`, rebind limpo, sessões físicas, recuperação e separação entre proposta e execução. Elas não foram duplicadas.

## O que não foi incorporado

Não foi copiado o runtime paralelo inteiro, nem sua camada LLM, seus conectores Internet, seus “simbiontes-filhos” ou seus adaptadores de execução. Isso aumentaria a superfície antes do hardware e criaria falsas alegações de generalidade. Também não foi promovido HMAC de fixture a atestação física: a autoridade continua pendente de B1/B2 e do hardware real.

## Resultado

A contribuição efetiva é uma melhoria pequena, testável e diretamente relacionada a múltiplos hospedeiros: **coordenação stale-safe por fencing**. O restante permanece como material de pesquisa comparativa, não como dependência do firmware.
