# Achados acadêmicos — delegação transitiva, revogação e principalidade

## Authorization Propagation in Multi-Agent AI Systems

Fonte: Tallam, *Authorization Propagation in Multi-Agent AI Systems: Identity Governance as Infrastructure*, arXiv:2605.05440v1, https://arxiv.org/html/2605.05440v1.

O artigo define authorization propagation como a preservação de invariantes de autorização enquanto principais não humanos recuperam dados, delegam tarefas e sintetizam resultados através de fronteiras mutáveis. O problema é separado em delegação transitiva, inferência por agregação e validade temporal. Um ponto importante para o HERUS é que a combinação de duas fontes pode revelar ou autorizar algo que nenhuma fonte isolada autorizaria; portanto, a autorização do resultado composto precisa ser verificada, não inferida pela existência de duas entradas válidas.

Aplicação ao HERUS: `ag_compose` deve ser uma operação de interseção e não de união. Uma cadeia de compartilhamento deve carregar o principal emissor, o receptor e a validade de cada concessão. Uma concessão vencida ou revogada invalida as derivações que dependem dela. O teste deve atacar tanto a delegação explícita quanto o caso aparentemente benigno de agregação entre duas fontes.

## WAVE

Fonte: Andersen et al., *WAVE: A Decentralized Authorization Framework with Transitive Delegation*, USENIX Security 2019, https://www.usenix.org/conference/usenixsecurity19/presentation/andersen.

WAVE apresenta autorização descentralizada com delegação de uma porção de permissões, proteção criptográfica e armazenamento não confiável, sem uma autoridade central capaz de modificar ou observar todas as permissões. A contribuição relevante para o HERUS é a distinção entre delegar uma porção limitada da autoridade e transferir autoridade total. A descentralização não elimina a necessidade de preservar a cadeia e verificar cada fronteira.

Aplicação ao HERUS: o compartilhamento continua sendo explicitamente limitado e não delegável por padrão. O envelope importado não pode ser exportado novamente por um contato, a menos que exista uma capacidade de delegação separada e mais restrita. A primeira versão será mais conservadora: conhecimento compartilhado pode ser lido no contexto correto, mas não pode gerar uma nova concessão.

## Requisitos derivados

| Requisito | Fonte | Teste proposto |
|---|---|---|
| Interseção após agregação | Authorization Propagation | composição em cadeia nunca excede o menor conjunto de autoridade |
| Cadeia de principal | Authorization Propagation | cada importação preserva issuer, recipient e parent share |
| Validade temporal propagada | Authorization Propagation | child expiry não excede o menor expiry da cadeia |
| Delegação limitada | WAVE | envelope importado não pode ser exportado por padrão |
| Revogação descendente | ambas | revogar share raiz invalida imports e derivações dependentes |
| Armazenamento não confiável | WAVE | envelope adulterado falha por vínculo de campos, não por confiança no transporte |

## Limites

WAVE é um sistema distribuído com mecanismos criptográficos; o HERUS atual ainda possui um contrato host-side bounded e não reivindica equivalência criptográfica. Os achados servem para criar invariantes de autoridade e uma futura especificação de envelope físico, não para afirmar que o simulador já resiste a clonagem, replay de rádio ou comprometimento de chave.
