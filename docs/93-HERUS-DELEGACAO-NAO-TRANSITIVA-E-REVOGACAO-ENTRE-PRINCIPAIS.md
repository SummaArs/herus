# HERUS — Delegação não transitiva e revogação entre principais

**Estado:** protótipo host-side em C11 estrito, pré-hardware.  
**Integração:** AGSC, AGSC-D, Poisoning Guard, Attribution Guard e composição contrafactual.

## Resumo

O compartilhamento explícito de uma memória não deve criar automaticamente o direito de compartilhá-la novamente. Se um principal recebe conhecimento do Core, deriva uma nova memória e reexporta essa derivação como se fosse sua, a cadeia de autorização perde o emissor original. Esse erro é uma forma de *authority laundering* transitivo: a informação permanece, mas o motivo pelo qual ela era admissível desaparece.

Esta etapa adiciona uma identidade de concessão ao grafo de atribuição. Um nó importado carrega `source_share_id`; qualquer derivação preserva esse identificador. Por padrão, um nó cuja origem é um share não pode ser exportado novamente e não pode entrar em composição com outro suporte. A revogação do emissor percorre a raiz local, é exportada para o destinatário exato e pode ser aplicada antes ou depois da importação. Um receptor novo pode armazenar a revogação em uma lista de bloqueio e rejeitar o share quando ele chegar posteriormente.

A motivação acadêmica vem de três problemas relacionados. A literatura de autorização multiagente trata delegação transitiva, inferência por agregação e validade temporal como subproblemas próprios de propagação de autorização [1]. WAVE mostra que delegação distribuída precisa transferir somente uma porção limitada de permissões e preservar a cadeia de confiança [2]. Causal Agent Replay fornece a inspiração para remover uma causa e verificar se o resultado muda, embora o HERUS implemente somente um teste determinístico bounded [3].

## Regra de não delegação transitiva

Para uma concessão `S` e uma derivação `D`:

```text
source_share(D) = source_share(S)
export(D)      = proibido por padrão
compose(D, X)  = proibido por padrão
```

Essa decisão é deliberadamente conservadora. O receptor pode usar o conteúdo como contexto dentro do próprio principal, sujeito a propósito, validade e epoch. Ele não pode transformar esse contexto em uma nova autoridade, em uma nova raiz de compartilhamento ou em um resultado composto que pareça independente.

| Situação | Resultado esperado |
|---|---|
| Importar share endereçado ao receptor | permitido após confirmação física |
| Derivar contexto local a partir do import | permitido, sem perder `source_share_id` |
| Exportar a derivação para outro principal | rejeitado |
| Compor um import com outro suporte | rejeitado |
| Transformar import em ação local soberana | rejeitado |
| Reutilizar share revogado | rejeitado |

A autoridade de ação não é transmitida pelo envelope. O principal local soberano continua sendo o único que pode alcançar a transição haptic local, e somente com uma preferência local pura e uma confirmação física nova.

## Revogação entre principais

O emissor mantém o vínculo entre `share_id`, nó de origem e destinatário. Depois de uma revogação física, `ag_revoke_share` revoga a raiz e todos os descendentes locais do emissor. `ag_export_revocation` cria uma prova endereçada ao destinatário registrado. O receptor aplica essa prova sobre o nó importado; a revogação percorre seus descendentes e adiciona o `share_id` à lista local de bloqueio.

A lista de bloqueio também pode ser preenchida antes do import. Isso é importante porque o transporte de uma revogação e o transporte do conteúdo podem ocorrer em ordens diferentes. A regra de admissão passa a ser:

```text
if share_id ∈ revoked_share_ids:
    reject import
else:
    accept only the exact addressed envelope
```

A revogação é idempotente. Repeti-la não reabre o nó e não produz uma nova autoridade. Redirecionar uma revogação para outro principal é rejeitado porque o destinatário está vinculado ao registro da exportação.

## Intervenção contrafactual

O cenário `attribution-transitive` executa uma intervenção concreta: o Core emite um share, o contato importa e deriva uma memória. Em seguida, o emissor revoga a raiz. A derivação deixa de ser admitida. A mesma revogação é entregue a um receptor novo antes do conteúdo; quando o envelope chega, o import é rejeitado.

A intervenção responde a uma pergunta estreita:

> Se a causa de autorização de uma memória compartilhada for removida, a cadeia ainda consegue produzir contexto admissível ou ação?

No modelo atual, o resultado muda de `AG_OK` para `AG_E_REVOKED` ou `AG_E_REPLAY`. Isso demonstra necessidade operacional da relação registrada, mas não é uma estimativa causal geral. Não há reexecução estocástica, intervalo de confiança ou alegação sobre uma LLM.

## Invariantes e redteam

O cenário `attribution-transitive` passou **15/15 invariantes**.

| Grupo | Evidência |
|---|---:|
| Criação e importação do share | 2 |
| Derivação contextual preservando emissor | 2 |
| Bloqueio de delegação e composição transitiva | 2 |
| Revogação física e exportação endereçada | 2 |
| Aplicação e propagação da revogação | 3 |
| Idempotência e bloqueio preventivo | 2 |
| Reuso de identificador e redirecionamento | 2 |
| **Total** | **15/15** |

A frente sabotadora removeu oito controles:

| Mutante | Controle exercitado |
|---|---|
| `derived-share-delegation-bypass` | bloqueio de reexportação de derivação |
| `imported-composition-bypass` | isolamento de imports na composição |
| `revoked-share-reentry-bypass` | lista de bloqueio durante import |
| `preventive-revocation-bypass` | revogação recebida antes do conteúdo |
| `transitive-revocation-bypass` | revogação da raiz importada e descendentes |
| `revocation-recipient-bypass` | destinatário exato da revogação |
| `revoked-share-id-reuse-bypass` | não reutilização de IDs revogados |
| `share-lineage-propagation-bypass` | propagação de `source_share_id` |

O resultado foi **8/8 mutantes mortos**. Falhas de compilação continuam sendo classificadas como lacunas de cobertura.

## Resultado consolidado

A integração elevou a bancada virtual de 261 para **276 invariantes**. A regressão global passou a auditoria de proveniência estrita, a bancada completa, o mutator global e sete campanhas específicas.

| Evidência | Resultado |
|---|---:|
| Auditoria de proveniência | **VÁLIDA** — 1 ativa, 3 pendentes |
| Bancada virtual | **276/276** |
| Mutator global | **7/7** |
| Semantic-life | **8/8** |
| AGSC | **8/8** |
| AGSC-D | **8/8** |
| Poisoning L1/L2/L3 | **8/8** |
| Attribution | **8/8** |
| Composition | **8/8** |
| Transitive | **8/8** |
| Veredito | **ALL INVARIANTS HOLD** |

## Limites científicos e próximos riscos

A implementação atual não é uma capacidade criptográfica completa. O envelope ainda é uma estrutura C em memória de processo; não há assinatura, MAC, anti-clonagem, armazenamento seguro ou verificação de chave no hardware. A revogação funciona quando uma prova válida chega ao receptor, mas o sistema não demonstra entrega garantida de revogações em rádio real.

Também não está provado que a principalidade do contato foi autenticada por pareamento físico, que concorrência entre múltiplos contatos é segura ou que o reboot atômico preserva listas de bloqueio em NVS. Esses riscos precisam de especificação e testes próprios na bancada física.

A alegação defensável é limitada: **no host bounded, um share importado não pode ser delegado novamente, uma revogação pode ser aplicada antes ou depois da importação e uma raiz revogada não pode reintroduzir seus descendentes no receptor testado**.

## Referências

[1]: https://arxiv.org/html/2605.05440v1 "Tallam, Authorization Propagation in Multi-Agent AI Systems: Identity Governance as Infrastructure"

[2]: https://www.usenix.org/conference/usenixsecurity19/presentation/andersen "Andersen et al., WAVE: A Decentralized Authorization Framework with Transitive Delegation"

[3]: https://arxiv.org/html/2606.08275v1 "Shah, Causal Agent Replay: Counterfactual Attribution for LLM-Agent Failures"
