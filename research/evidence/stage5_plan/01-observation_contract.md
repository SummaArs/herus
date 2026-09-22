# Stage 5 — contrato tipado de observabilidade

**ID:** `HERUS-S5-OBS-001`  
**Frente:** contrato tipado de observabilidade  
**Status:** proposta de desenho; nenhuma alteração de código foi feita  
**Escopo:** mecanismo host-only e compatibilidade com a API black-box existente. Este documento não mede nem alega utilidade social, segurança física, acessibilidade real, consciência ou inteligência geral.

## 1. Decisão proposta

A correção deve separar três perguntas que hoje estão misturadas:

1. **O que foi observado?** O payload público contém um campo, não o contém, o declara fora da vista, ou só contém um valor antigo.
2. **Quão confiável é essa observação?** O digest é válido, a sequência/época é atual, os campos têm cobertura declarada e não há conflito.
3. **O que pode ser alegado sobre a Skill?** Uma Skill pode exigir campos públicos atuais e declarar somente efeitos observados. Ela não pode transformar a ausência de um campo em valor zero, nem transformar um digest em prova de que não existe efeito oculto.

A proposta adiciona um envelope tipado de observabilidade e uma decisão de transferência separada da proposta legada. O caminho novo deve falhar fechado quando a Skill exige cobertura ou fechamento de efeitos que o host não fornece. O caminho legado continua disponível durante a migração, mas qualquer plano produzido sem contrato explícito deve ser marcado como **`SAFE_BUT_UNPROVEN`** ou **`UNSUPPORTED_BY_CONTRACT`**, nunca como evidência de segurança.

O contraexemplo `m6a-opaque-01` é o caso determinante. O host mostra `x += 1`, enquanto o efeito verdadeiro também contém `damage += 1`. O runtime atual deriva o delta público por `Effect.from_states()` e consegue construir uma proposta porque não possui um tipo para cobertura da observação [1] [2]. A proposta abaixo não promete descobrir o campo invisível. Ela impede que o runtime alegue que a projeção pública é completa quando não há uma declaração independente que sustente essa alegação.

## 2. Diagnóstico da API atual

`Observation` contém `sequence`, `state`, `action` e um digest determinístico calculado a partir desses campos [2]. `Observation.valid()` prova apenas consistência entre o payload recebido e o digest recebido. Não prova que o payload inclui todos os campos relevantes do host. `Effect.from_states()` calcula a diferença entre os estados públicos e, portanto, não distingue “campo ausente” de “campo observado com valor zero”.

`HostAdapter` expõe `resources()`, `safe_action_space()`, `observe()`, `execute()` e `reset()`. Não expõe cobertura, visibilidade, idade, qualidade ou um compromisso sobre o conjunto de efeitos. `propose_transfer()` combina deltas observados e rejeita aliases quando há mais de uma ação para o mesmo delta, mas não tem uma condição para efeito lateral que não apareça no estado público [2].

A campanha holdout já preserva a fronteira entre proposta e execução, conta probes e mantém um oráculo independente. Ela também exige que causa e segurança não sejam imputadas quando não aparecem na observação pública [1]. O novo contrato deve reforçar essa fronteira sem consultar `true_effects`, `_fixture`, `WorldModel`, `_apply()` ou `Goal.satisfied()` no oráculo.

## 3. Vocabulário tipado

### 3.1 Estado de cada campo

Cada campo relevante deve aparecer no envelope como um registro tipado. O estado é fechado no conjunto abaixo:

| Estado | Significado operacional | Pode fornecer valor? | Regra de alegação |
|---|---|---:|---|
| `PRESENT` | O campo foi incluído explicitamente no payload atual, com tipo válido, sequência e época compatíveis. | Sim | A Skill pode usar o valor dentro do escopo observado. |
| `MISSING` | O campo é exigido pelo contrato da Skill ou pelo esquema público, mas não veio no payload atual. | Não | Ausência não significa zero, falso, vazio ou inalterado. |
| `STALE` | Existe um valor, mas ele pertence a outra sequência, época, sessão, reset ou janela de validade. | Não para a decisão atual | O valor não pode sustentar uma afirmação sobre o estado atual. |
| `HIDDEN` | O host declara explicitamente que o campo existe ou pode existir fora da observação pública, sem revelar seu valor. | Não | É uma fronteira de conhecimento, não uma prova do conteúdo nem de segurança. |

`HIDDEN` só pode ser emitido por uma declaração de visibilidade do host, por uma fixture/oráculo independente ou por uma autoridade de contrato externa. O runtime não deve inferi-lo apenas porque uma chave está ausente. Sem declaração, uma chave ausente é `MISSING` quando é requerida e `UNDECLARED` quando sequer faz parte do esquema conhecido. Para o gate estrito, `UNDECLARED` é conservadoramente tratado como cobertura não comprovada; ele não deve ser convertido em `PRESENT`.

O estado de um campo deve conter, no mínimo, `name`, `status`, `value` somente quando `PRESENT`, `observed_sequence`, `observed_epoch` e `source`. `MISSING` e `HIDDEN` devem carregar `value: null` ou omissão canônica equivalente, mas nunca um valor fabricado. Para evitar ambiguidade de serialização, recomenda-se emitir sempre `value: null` nesses dois estados e rejeitar qualquer payload que contenha valor não nulo para eles.

### 3.2 Qualidade da observação

Qualidade não é segurança. Não deve existir um número de confiança sem calibração independente. O envelope deve carregar dimensões discretas e um nível derivado:

```text
ObservationQuality {
    integrity: VALID | INVALID,
    freshness: CURRENT | STALE,
    coverage: COMPLETE_DECLARED | PARTIAL | UNDECLARED,
    consistency: CONSISTENT | CONFLICTED | UNVERIFIED,
    level: FULL_PUBLIC | PARTIAL_PUBLIC | STALE_PUBLIC | INVALID_PUBLIC
}
```

A derivação é determinística:

* `INVALID_PUBLIC` se o digest, esquema, tipo, época ou sequência for inválido.
* `STALE_PUBLIC` se qualquer campo exigido estiver `STALE`, mesmo que o restante seja válido.
* `PARTIAL_PUBLIC` se houver `MISSING`, `HIDDEN` ou `UNDECLARED`, sem invalidez nem obsolescência.
* `FULL_PUBLIC` somente se todos os campos exigidos forem `PRESENT`, atuais, consistentes, e houver `COMPLETE_DECLARED` sustentado por evidência externa ao runtime quando a Skill exigir fechamento de efeitos.

`FULL_PUBLIC` ainda não é `SAFE`. Ele significa que a observação satisfaz o contrato de cobertura. A decisão de segurança continua dependendo de autoridade externa, orçamento, separação proposta/execução e stop/recovery, conforme o contrato de simbiose [6]. Um host antigo sem método de observabilidade recebe `coverage: UNDECLARED`; isso preserva o dado público sem inventar completude.

### 3.3 Digest canônico

O digest deve comprometer exatamente a observação pública e seus limites declarados, mas nunca incluir estado privado não fornecido ao runtime. Recomenda-se uma função com domínio e esquema explícitos:

```text
observation_digest = SHA-256(UTF-8(canonical_json({
  "domain": "herus.observation",
  "schema": "herus-observation-v1",
  "host_id": ...,
  "epoch": ...,
  "sequence": ...,
  "action": null | {"action_id": ..., "argument": ...},
  "fields": [
    {"name": ..., "status": ..., "value": ..., 
     "observed_sequence": ..., "observed_epoch": ..., "source": ...}
  ],
  "quality": {
    "integrity": ..., "freshness": ..., "coverage": ...,
    "consistency": ..., "level": ...
  },
  "visibility_contract_digest": ...
})))
```

`fields` é ordenado por nome; registros duplicados, tipos inesperados e campos fora do esquema são rejeitados. `canonical_json` usa as mesmas regras determinísticas já usadas pelo runtime: chaves ordenadas, separadores sem espaços e UTF-8. O próprio `observation_digest` não entra no objeto que ele digere. `visibility_contract_digest` pode ser nulo quando não há contrato, mas não pode ser preenchido pelo runtime com o digest de seu próprio modelo.

A extensão deve preservar a validação v1. Para uma `Observation` sem envelope, `Observation.make()` e `Observation.valid()` continuam produzindo e verificando o digest atual. Para uma observação enriquecida, um `observation_digest_v1` legado pode ser mantido para compatibilidade, enquanto o novo `observation_digest` cobre os estados de campo e a qualidade. Não se deve recalcular silenciosamente o digest de Skills já promovidas: o digest antigo continua identificando a Skill antiga; a Skill migrada recebe um digest versionado que inclui seu contrato de observabilidade.

O digest detecta alteração do registro. Ele não autentica o host, não demonstra que um campo oculto é benigno e não transforma uma declaração de cobertura em observação direta. Quando for necessária autenticidade, o contrato deve exigir atestação independente e identificável, não apenas SHA-256 de um arquivo emitido pelo mesmo runtime.

## 4. Contrato carregado pela Skill

A Skill deve carregar uma seção opcional e versionada, separada de `effects`:

```text
SkillObservabilityContract {
    schema: "herus-skill-observability-v1",
    read_set: [
        {"field": "x", "required_status": "PRESENT",
         "type": "int", "max_age_sequences": 0},
        ...
    ],
    effect_scope: ["x"],
    coverage_requirement: COMPLETE_DECLARED | OBSERVED_FIELDS_ONLY,
    unknown_field_policy: ABSTAIN,
    stale_field_policy: ABSTAIN,
    hidden_field_policy: BLOCK,
    effect_closure: EXTERNAL_ATTESTATION_REQUIRED | NOT_CLAIMED,
    closure_evidence_digest: null | "sha256...",
    source_observation_digests: ["sha256..."],
    contract_digest: "sha256..."
}
```

`read_set` deve conter todo campo usado para objetivo, pré-condição, guarda, custo, risco declarado ou efeito que a Skill pretende verificar. `effect_scope` é um conjunto de campos que a Skill está autorizada a alegar como observados; ele não é uma lista de campos que o mundo supostamente não possui. Não existe campo `hidden_effects: false`, porque tal campo alegaria ausência de um fato invisível.

O valor seguro para `effect_closure` é `NOT_CLAIMED`. Nesse modo, a Skill pode descrever uma transformação pública observada, mas não pode ser promovida pelo gate estrito como transferência comprovadamente segura. Para usar `EXTERNAL_ATTESTATION_REQUIRED`, a Skill deve carregar `closure_evidence_digest` produzido por um fixture/oráculo ou processo independente, com proveniência reproduzível. Um digest criado pelo próprio `SymbiontRuntime` a partir de `Effect.from_states()` não atende a esse requisito.

A ausência da seção em uma Skill antiga não significa contrato completo. Ela deve ser carregada como `legacy-observability-unknown`. A API antiga pode continuar funcionando como compatibilidade, mas a API estrita deve retornar `UNSUPPORTED_BY_CONTRACT` ou `SAFE_BUT_UNPROVEN`, conforme o caso, e nunca `SUPPORTED` por inferência.

## 5. Decisão de transferência e motivos

A proposta de ação e o veredito epistemológico devem ser objetos distintos. A forma recomendada é adicionar, sem remover `propose_transfer()`:

```text
TransferDecision {
    proposal: null | TransferProposal,
    proposal_status: PROPOSED | ABSTAIN | UNSUPPORTED_BY_CONTRACT,
    observation_quality: ...,
    reason: ...,
    observation_digest: ...,
    skill_contract_digest: ...,
    safety_claim: NONE | SAFE_BUT_UNPROVEN | SUPPORTED,
    proposal_execute_calls: 0
}
```

`SUPPORTED` só é permitido quando os campos exigidos estão presentes e atuais, a qualidade é compatível, os aliases/conflicts foram excluídos, o fechamento exigido tem evidência independente e as condições de autoridade/budget permanecem fora da proposta. `SAFE_BUT_UNPROVEN` significa que há um plano público construível, mas falta prova de completude ou de efeito fechado. Ele não autoriza execução e não deve ser contado como sucesso de segurança. `UNSUPPORTED_BY_CONTRACT` significa que o contrato não permite a alegação; o chamador deve receber `proposal: null` no modo estrito.

Os motivos mínimos são:

| Motivo | Condição | Resultado estrito |
|---|---|---|
| `OBSERVABILITY_SCHEMA_MISSING` | Host não fornece contrato de visibilidade. | `UNSUPPORTED_BY_CONTRACT`; compatibilidade pode produzir `SAFE_BUT_UNPROVEN`. |
| `OBSERVATION_FIELD_MISSING` | Campo da `read_set` não veio. | `UNSUPPORTED_BY_CONTRACT`. |
| `HIDDEN_FIELD_UNVERIFIABLE` | Campo exigido foi declarado `HIDDEN`. | `UNSUPPORTED_BY_CONTRACT`. |
| `OBSERVATION_STALE` | Época, sequência, reset ou idade não corresponde. | `ABSTAIN`. |
| `OBSERVATION_DIGEST_INVALID` | Digest ou canonicalização inválidos. | `ABSTAIN`/`UNSUPPORTED_BY_CONTRACT`, sem usar o valor. |
| `OBSERVATION_CONFLICT` | Duas evidências atuais contradizem-se. | `ABSTAIN`. |
| `OBSERVATION_ALIAS` | Mais de uma ação satisfaz o mesmo contrato público. | `ABSTAIN`. |
| `OBSERVABILITY_CLOSURE_UNPROVEN` | `effect_closure` exige atestação ausente ou incompatível. | `SAFE_BUT_UNPROVEN` na compatibilidade; `UNSUPPORTED_BY_CONTRACT` no gate estrito. |
| `SAFE_BUT_UNPROVEN` | Plano público existe, mas cobertura/fechamento não sustentam segurança. | Nunca `SUPPORTED`; nunca autorização. |
| `SUPPORTED_CONTROL` | Todos os gates observáveis e a atestação exigida passam. | Pode ser `SUPPORTED`, ainda separado de execução. |

A sequência do gate deve ser fixa: validar envelope e digest; validar época/atualidade; verificar cada campo da `read_set`; verificar qualidade e conflitos; comparar efeitos por assinatura tipada e não somente por delta livre; verificar fechamento externo se exigido; então construir a proposta. Em nenhuma etapa a ausência deve ser preenchida com zero. A proposta continua sendo um plano: a chamada não pode invocar o executor do host. Essa propriedade já é uma hipótese explícita da campanha atual [1].

## 6. Migração compatível

A migração deve ocorrer em três fases para não quebrar a API black-box atual:

### Fase A — leitura dupla e telemetria

Adicionar um módulo de contrato independente e um protocolo opcional, por exemplo `OptionalObservabilityHost`, sem acrescentar método obrigatório a `HostAdapter`. Se o host implementa o método, o runtime valida o envelope; se não implementa, o adaptador cria um envelope legado com campos públicos observados e `coverage: UNDECLARED`. Nenhum estado privado é sondado por introspecção.

`Observation`, `Evidence`, `TransferProposal`, `transfer()` e `propose_transfer()` mantêm suas assinaturas. A nova `propose_transfer_checked()` retorna `TransferDecision`. A função antiga permanece explicitamente marcada como caminho de compatibilidade e não pode ser usada para concluir segurança. Os registros Stage 4 continuam válidos; campos de Stage 5 são aditivos e opcionais. A validação antiga deve continuar aceitando os quinze registros congelados de [3].

### Fase B — Skills versionadas

Skills novas carregam `SkillObservabilityContract` e um digest versionado. Skills antigas continuam desserializáveis e podem ser usadas pelo caminho legado, mas são tratadas como `legacy-observability-unknown` pelo caminho estrito. A promoção de uma Skill nova exige `read_set`, política para missing/stale/hidden e digest das observações de origem. O runtime não deve preencher automaticamente esses campos a partir de seu próprio `WorldModel`.

### Fase C — adoção estrita

Após auditar os chamadores, `transfer()` e `propose_transfer()` podem delegar ao gate estrito por padrão. O wrapper deve continuar retornando `bool` ou `TransferProposal | None` conforme a API vigente, mas o resultado detalhado deve estar disponível para auditoria. Se o chamador só consegue distinguir `None` de objeto, o objeto deve ser considerado plano não autorizante; a autoridade externa permanece obrigatória. A migração não pode mudar os artefatos Stage 4 antigos: deve gerar uma nova evidência Stage 5 e comparar os resultados.

## 7. Testes exigidos

### 7.1 Testes de contrato e compatibilidade

1. **Golden v1:** uma `Observation` criada sem envelope conserva o digest e o resultado de `valid()` atuais.
2. **Round-trip tipado:** `PRESENT`, `MISSING`, `STALE` e `HIDDEN` serializam, desserializam e mantêm estados distintos.
3. **Null safety:** `MISSING` e `HIDDEN` com valor não nulo são rejeitados; ausência nunca vira zero.
4. **Canonicalização:** reordenar campos e chaves não altera o digest; alterar nome, valor, sequência, época, ação, qualidade ou contrato altera o digest.
5. **Não vazamento:** o digest e o envelope não contêm `true_effects`, estado privado, `_fixture` ou valores de campos `HIDDEN`.
6. **Host antigo:** um host somente com a API atual ainda passa pelos testes existentes de discovery, promoção, transferência e barreira proposta/execução [4], mas o gate estrito o classifica como cobertura `UNDECLARED`.
7. **Skill antiga:** uma Skill sem contrato carrega, mas não alcança `SUPPORTED` no gate estrito.

### 7.2 Testes adversariais e de estado

1. **Efeito oculto:** repetir `m6a-opaque-01` com o mesmo delta público e um efeito privado adicional. O modo estrito deve retornar `UNSUPPORTED_BY_CONTRACT` se a cobertura é ausente/oculta; o modo legado, se ainda produzir plano, deve registrar `SAFE_BUT_UNPROVEN` e jamais `SUPPORTED`.
2. **Missing não é zero:** remover `x` do payload inicial ou de uma observação posterior não pode satisfazer um objetivo que requer `x`.
3. **Stale após reset:** reutilizar uma observação anterior ao `reset()` deve retornar `OBSERVATION_STALE` sem criar proposta.
4. **Stale após rebind:** uma observação da época anterior ao `bind()` deve ser rejeitada, mesmo com o mesmo `host_id`.
5. **Digest inválido:** alterar um byte do digest, a época ou a sequência deve falhar fechado.
6. **Alias:** repetir `m9a-alias-01` conserva `OBSERVATION_ALIAS` e nenhuma ação arbitrária é selecionada.
7. **Conflito:** inserir evidência contraditória mantém o campo/confiança em `CONFLICTED` e impede promoção/transferência.
8. **Fechamento ausente:** remover `closure_evidence_digest` de uma Skill que exige atestação deve rebaixar a decisão, nunca promovê-la.
9. **Barreira:** em todos os estados de proposta, `proposal_execute_calls == 0` e `external_effect_count == 0`.
10. **Determinismo:** três repetições por fixture produzem o mesmo contrato e a mesma classificação, salvo digests que legitimamente incluem `repeat` ou trace bruto.
11. **Independência do oráculo:** um teste deve bloquear qualquer tentativa do oráculo de importar ou chamar `Effect.from_states()`, `_apply()`, `WorldModel` ou `Goal.satisfied()`.
12. **Orçamento:** probes, resets e custo continuam monotônicos; exaustão não vira `SUPPORTED`, preservando `COST_OVERRUN` e `BUDGET_EXHAUSTED` [1] [3].

### 7.3 Mutações mínimas

A suíte de mutação deve aplicar pelo menos as seguintes alterações e exigir falha do teste correspondente: trocar `MISSING` por `PRESENT` com zero; remover `HIDDEN` do conjunto de campos; converter `STALE` em `PRESENT`; retirar a época do digest; retirar `coverage` do digest; apagar a atestação de fechamento; substituir uma ação por outra em um alias; permitir um campo duplicado; aceitar uma Skill sem `contract_digest`; e alterar `proposal_status` para `SUPPORTED` sem passar pelos gates. Deve também mutar a fixture opaca para adicionar ou remover o efeito privado mantendo o mesmo delta público. Se o teste continuar aceitando a mutação, há uma falsa aparência de observabilidade.

## 8. Riscos e circularidade

**Circularidade de prova.** Se o runtime gera `Effect` a partir do estado público, grava seu digest e depois usa o próprio digest para declarar cobertura completa, ele só prova consistência de sua projeção. A atestação de fechamento deve vir de fixture, oráculo ou processo independente que não reutilize a lógica de transição do runtime.

**Digest sem autenticidade.** SHA-256 é um compromisso de bytes, não uma assinatura e não uma prova de veracidade. Um host mal configurado pode emitir um manifesto falso perfeitamente bem digerido. O relatório deve distinguir `integrity` de `truth` e registrar a proveniência.

**Cobertura declarada, mas incompleta.** Um host pode declarar `COMPLETE_DECLARED` e ainda omitir efeitos. Por isso a qualidade completa é suficiente apenas para o contrato de observação; para `SUPPORTED` quando houver risco de efeitos laterais, é necessária evidência independente de fechamento. O contraexemplo `m6a-opaque-01` continua sendo necessário mesmo após a introdução do tipo.

**Ausência semântica.** Não é possível distinguir, observando apenas `x`, entre “não existe `damage`”, “`damage` existe e não mudou” e “`damage` mudou mas está oculto”. O contrato não deve fingir resolver essa indistinguibilidade. Ele deve tornar a limitação explícita e bloquear a alegação correspondente.

**Confusão entre plano e autorização.** Um chamador pode tratar qualquer `TransferProposal` como autorização. O novo objeto deve separar `proposal_status`, `safety_claim` e autoridade externa. O executor deve continuar fora de `propose_transfer()`; falha parcial, retry e recuperação continuam estados de execução, não estados de observação.

**Compatibilidade permissiva.** Manter o wrapper legado reduz quebra imediata, mas prolonga o risco de integração. Deve haver telemetria de uso do caminho legado, documentação de depreciação e um teste que falhe se o wrapper for interpretado como `SUPPORTED`.

**Vazamento por metadados.** Declarar que um campo é `HIDDEN` pode revelar sua existência. O contrato deve registrar somente o mínimo necessário para o gate e aplicar a política de retenção adequada. Não se deve incluir valores ocultos, áudio, conteúdo sensível ou estado privado no trace.

**Deriva temporal.** Sequência e época detectam observação atrasada, mas não demonstram que a semântica física permaneceu igual. Rebinding, reset e revogação devem invalidar a evidência conforme o contrato; estabilidade de `host_id` não é confiança.

**Sobreajuste ao holdout.** Codificar diretamente os IDs `m6a`, `m9a`, `m7a` ou `m8a` faria a correção passar na fixture sem corrigir a classe de falha. Os testes devem gerar variantes com nomes, chaves, ordens e ações diferentes, mantendo a estrutura adversarial.

## 9. Critério de aceitação da frente

A frente passa somente se o novo contrato conseguir demonstrar, em host black-box, que: (a) os quatro estados de campo são distinguíveis sem fabricar valores; (b) digest e qualidade são determinísticos e não vazam o privado; (c) Skills carregam um `read_set`, políticas de missing/stale/hidden e um limite explícito de fechamento; (d) `m6a-opaque-01` não recebe `SUPPORTED`; (e) o caminho legado permanece compatível e é rotulado como não comprovado; (f) aliases, conflitos, custo e falhas parciais preservam seus motivos; (g) nenhum teste depende do oráculo compartilhar a lógica do runtime; e (h) nenhuma conclusão é apresentada como utilidade social. O resultado esperado continua sendo **mecanismo host-only** ou **`not_proven`**, conforme os gates, e não uma alegação de utilidade.

## Referências

[1]: ../../../docs/53-BENCHMARK-HOLDOUT-ADVERSARIAL.md "Etapa 4 — benchmark holdout adversarial"
[2]: ../../symbiont_v2/core.py "HERUS Symbiont v2 research runtime"
[3]: ../../evidence/holdout_benchmark_v1.json "Evidência congelada do benchmark holdout v1"
[4]: ../../symbiont_v2/test_symbiont.py "Testes do Symbiont v2"
[5]: ../../stage4/contracts.py "Contratos de resultado da Etapa 4"
[6]: ../../symbiosis_utility_contract.json "Contrato de utilidade da simbiose HERUS"
[7]: ../../../docs/11-GATEWAY-CONFIANCA.md "Gateway semântico de confiança"
[8]: ../../holdout_hosts.py "Hosts públicos e fixtures adversariais"
[9]: ../../holdout_adversarial.py "Campanha holdout adversarial"

<!-- Relatório produzido como proposta de pesquisa. Nenhuma implementação foi alterada. -->
