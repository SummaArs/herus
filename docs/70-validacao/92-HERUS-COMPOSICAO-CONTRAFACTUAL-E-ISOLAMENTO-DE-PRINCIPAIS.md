# HERUS — Composição contrafactual e isolamento entre principais

**Estado:** protótipo host-side em C11 estrito, pré-hardware.  
**Integração:** AGSC, AGSC-D, Poisoning Guard L1/L2/L3 e Attribution Guard.

## Resumo

A proveniência de um registro isolado não é suficiente quando uma oferta depende de mais de uma fonte. Uma composição pode transformar duas evidências fracas em uma autorização forte se usar união de autoridade ou de escopo. Também pode apagar a segunda causa, reclassificar uma composição local+Core como memória local ou atravessar um principal diferente sem concessão explícita.

Esta etapa implementa uma extensão bounded do Attribution Guard com duas operações distintas. `ag_compose` registra uma relação de suporte com dois pais, conserva a máscara de fontes e calcula interseção de autoridade e escopo. `ag_export_share` e `ag_import_share` transportam um envelope explicitamente endereçado, com emissor, destinatário, epoch, propósito, proveniência e confirmação física. O destinatário recebe uma memória atribuída a si, mas a origem e o emissor permanecem visíveis; o conteúdo não ganha autoridade local.

A ideia é compatível com duas linhas recentes da literatura. O trabalho sobre memória colaborativa separa memória privada e compartilhada, preserva atributos imutáveis de proveniência e distingue políticas de leitura e escrita [1]. O trabalho sobre Causal Agent Replay trata uma execução como modelo causal e usa intervenções para observar se a remoção de uma etapa altera o resultado [2]. O HERUS implementa uma versão determinística e limitada dessas ideias, sem reivindicar causalidade estatística geral.

## Contrato de composição

Se `L` e `R` são nós ativos do mesmo principal, a composição `C` satisfaz:

```text
authority(C) = authority(L) ∩ authority(R)
scope(C)     = scope(L) ∩ scope(R)
parents(C)  = {L, R}
source(C)   = source_mask(L) ∪ source_mask(R)
owner(C)    = owner(L) = owner(R)
```

A união aparece somente na máscara de proveniência, porque a composição precisa registrar que local e Core participaram. Ela não aparece na autoridade nem no escopo. Se a interseção for vazia, a composição é rejeitada; não existe fallback para união.

A composição somente produz conhecimento ou política contextual. Ela não produz preferência local, oferta ou ação. Mesmo com confirmação física, uma oferta `AT_SOURCE_COMPOSITE` não satisfaz o contrato da ação haptic local, que exige fonte local pura, emissor local, proprietário local e papel `AG_ROLE_PREFERENCE`.

| Atributo | Composição permitida | Composição proibida |
|---|---|---|
| Autoridade | interseção bit a bit | união ou promoção de bits |
| Escopo | interseção bit a bit | união ou alcance novo |
| Linhagem | dois pais preservados | apagar o segundo pai |
| Fonte | máscara local+Core | rotular como local |
| Papel | knowledge ou policy | preference ou action |
| Validade | mínimo dos limites | filho além do pai |
| Principal | mesmo proprietário | mistura silenciosa de proprietários |

## Intervenção contrafactual bounded

A bancada executa duas intervenções simples. Primeiro, revoga a raiz local e tenta recompor a mesma política com o suporte Core ainda ativo. Como a raiz revogada é uma causa necessária, a recomposição deve falhar. Segundo, o cenário verifica que a oferta composta, quando sua segunda causa é removida, também não pode continuar sendo admitida.

Isso é uma intervenção de contrato, não uma inferência causal sobre o mundo. Ela responde à pergunta mínima: **se uma aresta registrada como suporte for removida, o HERUS ainda produzirá a mesma oferta autorizável?** Se a resposta for sim, a proveniência é decorativa ou o oráculo está fraco.

O Causal Agent Replay observa que intervenções em políticas estocásticas produzem distribuições de resultados e que a reamostragem pode contaminar etapas posteriores; por isso, usa um ponto de compromisso e estimadores contrastivos [2]. No HERUS host-side, não há aleatoriedade nem Monte Carlo. O ponto de compromisso é a transição local de confirmação, e o teste exige mudança discreta: `AG_OK` contra `AG_E_REVOKED`, `AG_E_PRINCIPAL` ou `AG_E_SHARE`.

## Isolamento entre principais

O índice agora possui um `principal_id`. O principal local soberano é distinto do Core e de um contato pareado. Cada nó registra proprietário e emissor. A exportação não permite enviar para o próprio emissor; a importação exige que o destinatário do envelope seja exatamente o principal que está importando.

O envelope compartilhado contém `share_id`, nó e proveniência original, raiz, máscara de fontes, emissor, destinatário, fonte, papel, propósito, autoridade, escopo, geração, expiração, epoch do emissor e confirmação física. O `share_id` ocupa um namespace próprio e não é confundido com `node_id` ou `provenance_id`.

A importação cria um novo pertencimento local para o receptor, mas não muda o emissor ou a fonte. O mesmo envelope só pode ser consumido uma vez. Alterar o destinatário, a fonte ou a confirmação não produz uma importação válida. Um contato pode recuperar uma política do Core como contexto, mas `ag_grant_local_action` rejeita a ação porque o proprietário não é o principal local soberano.

> Compartilhar conhecimento não é transferir autoridade. O HERUS importa a proveniência; não importa uma permissão de ação.

A separação segue a arquitetura de memória colaborativa, que distingue fragmentos privados e compartilhados, conserva proveniência imutável e calcula visões conforme permissões atuais [1]. No HERUS, a política de escrita é a exportação física explícita; a política de leitura é a admissão do receptor com propósito, epoch, destinatário e validade compatíveis.

## Invariantes e redteam

O cenário `attribution-composition` verifica **18 invariantes**.

| Grupo | Evidência |
|---|---:|
| Composição e propósito | 3 |
| Interseção de autoridade e escopo | 2 |
| Bloqueio de ação e papel | 2 |
| Intervenção por revogação | 2 |
| Exportação e preservação de emissor | 2 |
| Importação e propriedade do receptor | 3 |
| Replay, destinatário, fonte e confirmação | 4 |
| **Total** | **18/18** |

A frente sabotadora removeu oito controles críticos:

| Mutante | Controle exercitado |
|---|---|
| `composition-union-authority` | interseção de autoridade |
| `composition-union-scope` | interseção de escopo |
| `composition-single-parent` | conservação da segunda causa |
| `composition-source-laundering` | marca `AT_SOURCE_COMPOSITE` |
| `recomposition-revocation-bypass` | intervenção após revogação |
| `share-recipient-bypass` | destinatário exato |
| `share-source-bypass` | vínculo fonte–máscara |
| `share-confirmation-bypass` | confirmação física na importação |

O resultado foi **8/8 mutantes mortos**. Mutantes que não compilam continuam sendo falhas de cobertura, não vitórias do construtor.

## Resultado consolidado

Após a integração, a bancada total passou de 243 para **261 invariantes**. O resultado inclui 26 invariantes de atribuição básica, 7 do benchmark contra baselines permissivos e 18 da composição multi-principal. Os cinco redteams anteriores e o novo redteam de composição passaram integralmente.

| Evidência | Resultado |
|---|---:|
| Auditoria de proveniência estrita | **VÁLIDA** |
| Invariantes totais da bancada | **261/261** |
| Mutator global | **7/7** |
| Semantic-life | **8/8** |
| AGSC | **8/8** |
| AGSC-D | **8/8** |
| Poisoning L1/L2/L3 | **8/8** |
| Attribution | **8/8** |
| Composition | **8/8** |
| Veredito | **ALL INVARIANTS HOLD** |

## Limites científicos

A composição foi testada sobre índices bounded em memória do processo. O resultado não prova que o sistema reconhece corretamente múltiplos interlocutores, que o contato físico é autêntico, que uma revogação chega ao hardware, que um envelope é criptograficamente protegido em rádio real ou que a memória humana foi modelada corretamente.

Também não prova isolamento contra todo canal lateral. O contrato cobre IDs, fonte, principal, propósito, epoch, autoridade, escopo e replay no modelo implementado. Persistência NVS, falha de energia durante importação, concorrência entre contatos, armazenamento seguro, anti-clonagem, ruído de rádio e ataques ao firmware ainda exigem bancada física e testes adicionais.

A alegação defensável é mais estreita: **dentro do modelo host-side testado, composição não amplia autoridade, uma causa removida impede a recomposição e um principal diferente não consegue transformar uma memória compartilhada em ação local soberana**.

## Referências

[1]: https://arxiv.org/html/2505.18279v1 "Rezazadeh et al., Collaborative Memory: Multi-User Memory Sharing in LLM Agents with Dynamic Access Control"

[2]: https://arxiv.org/html/2606.08275v1 "Shah, Causal Agent Replay: Counterfactual Attribution for LLM-Agent Failures"
