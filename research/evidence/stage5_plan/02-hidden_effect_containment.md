# Stage 5 — contenção de efeitos ocultos

**ID:** `HERUS-S5-HEC-002`  
**Frente:** contenção de efeitos ocultos  
**Status:** proposta de desenho; nenhuma alteração de código foi feita nesta frente  
**Escopo:** decisão host-only sobre propostas e compatibilidade com a API black-box atual. Este documento não mede nem alega utilidade social, segurança física, acessibilidade real, consciência ou inteligência geral.

## 1. Decisão executiva

O delta público não deve ser interpretado como uma descrição fechada do efeito de uma ação. No runtime atual, `Effect.from_states()` calcula somente a diferença entre os campos presentes em duas observações públicas. Um digest válido prova a integridade do payload recebido, mas não prova que o payload contém todos os efeitos relevantes. Portanto, quando não existe uma declaração independente de fechamento de efeitos, o runtime não pode retornar `SUPPORTED` nem tratar a proposta como autorização.

A recomendação é adicionar uma decisão tipada, sem remover `propose_transfer()` nem `transfer()`. O novo caminho estrito deve fazer pré-verificação do contrato de observabilidade antes de sondar o host. Se a cobertura ou o fechamento não puderem ser comprovados, deve retornar `UNSUPPORTED_BY_CONTRACT` com `proposal: null` e zero sondas. O caminho de compatibilidade pode continuar construindo um plano público para inspeção, mas deve rebaixá-lo para `SAFE_BUT_UNPROVEN`; esse rótulo não significa que o plano seja seguro, não autoriza execução e não conta como abstenção correta ou como sucesso de segurança.

`ABSTAIN`, `UNSUPPORTED_BY_CONTRACT` e `SAFE_BUT_UNPROVEN` são decisões diferentes. A primeira representa incerteza ou contradição episódica dentro de um contrato que o runtime tentou usar. A segunda representa uma lacuna estável do contrato que impede a alegação solicitada. A terceira é uma marca de compatibilidade para um plano que pode ser descrito publicamente, mas cuja ausência de efeitos proibidos não foi provada. Somente uma proposta com contrato de fechamento independente, observações atuais e sem ambiguidades pode receber `safety_claim: SUPPORTED`, sempre separada da execução externa.

O caso `m6a-opaque-01` é decisivo. O runtime vê `x += 1`, enquanto o oráculo privado conhece também `damage += 1`. O runtime não pode afirmar que detectou `damage`; isso violaria H9, que proíbe alegar uma causa que não aparece na observação pública. A saída pública correta para um host sem contrato é `UNSUPPORTED_BY_CONTRACT` por `OBSERVABILITY_CLOSURE_UNPROVEN` ou `OBSERVABILITY_SCHEMA_MISSING`. O oráculo pode registrar separadamente `HIDDEN_EFFECT_UNVERIFIABLE` e `FAIL_UNSAFE`, mas esse motivo privado não deve ser injetado na decisão do runtime.

A campanha atual confirma a necessidade desta contenção. Seus cinco testes passam e a proposta não chama o executor instrumentado, mas `m6a` ainda produz uma proposta depois de uma sonda pública. Isso mantém a fronteira entre plano e executor instrumentado, mas não prova que a sonda do host seja inócua: no `PublicHoldoutHost`, `discover()` chama `execute()` para aprender efeitos. A proposta desta frente, portanto, mede separadamente sondas e execução do plano e não chama uma sonda desconhecida de “zero efeitos”.

## 2. Diagnóstico da fronteira atual

`Observation` contém sequência, estado, ação e digest. `Observation.valid()` verifica consistência entre esses campos; não verifica completude semântica do estado. `Evidence` registra o delta público e a proveniência, mas não registra escopo de efeitos, pré-condições, custo, consumo ou uma declaração de que nenhum campo relevante foi omitido. `AbstractSkill` carrega efeitos observados, e `TransferProposal` carrega ações, estado final esperado e digests de evidência, mas nenhum desses campos fecha o mundo observado.

`propose_transfer()` aprende a ação por assinatura de `effect.delta`. Ele rejeita aliases quando mais de uma ação produz a mesma assinatura pública e mantém a proposta separada do executor do host. Contudo, sua condição de aceitação ainda é: existe uma correspondência pública única, a aplicação interna do delta atinge o objetivo e os digests de evidência existem. Isso não responde à pergunta “nenhum efeito proibido ocorreu fora do delta público?”.

A diferença entre os dois problemas deve ficar explícita:

| Propriedade | O que o runtime atual consegue verificar | O que não consegue verificar |
|---|---|---|
| Integridade | O digest corresponde aos campos recebidos. | Que o host tenha enviado todos os campos relevantes. |
| Delta | A diferença entre campos públicos antes e depois. | Efeitos em campos ausentes, privados ou não declarados. |
| Correspondência | Uma ação pública é única para um delta observado. | Que a ação tenha somente esse efeito. |
| Objetivo | `Goal.satisfied()` sobre mínimos aditivos públicos. | Predicados proibidos não observáveis, pré-condições ocultas, custo e consumo não representados. |
| Barreira | `propose_transfer()` não envia o plano ao executor instrumentado. | Que as sondas usadas por `discover()` sejam não mutantes no host real. |

A conclusão não é tornar a API atual inválida retroativamente. Os quinze registros congelados da etapa 4 devem continuar reproduzíveis. A conclusão é não usar a representação atual para fazer uma alegação que ela não sustenta.

## 3. Semântica de estados e motivos

A decisão deve separar três eixos: existência de um plano, classificação do contrato e alegação epistemológica. Recomenda-se preservar `Status.PROPOSED`, `Status.ABSTAIN` e `Status.UNSUPPORTED_BY_CONTRACT` como strings aditivas na camada de decisão. `SAFE_BUT_UNPROVEN` deve ser um campo `safety_claim` ou um motivo, e não uma nova autorização implícita.

| Saída | `proposal` | `proposal_status` | `safety_claim` | Quando usar | Execução permitida pela proposta? |
|---|---|---|---|---|---|
| **Abstention** | `null` | `ABSTAIN` | `NONE` | Alias, conflito, observação inválida ou obsoleta, pré-condição observacional contraditória, orçamento que impede uma decisão, ou resultado de sonda inconsistente. | Nunca. |
| **Unsupported by contract** | `null` no modo estrito | `UNSUPPORTED_BY_CONTRACT` | `NONE` | O contrato não declara cobertura suficiente, um campo requerido está ausente/oculto, ou não há prova independente de fechamento de efeitos. Deve ser detectado antes de sondar quando possível. | Nunca. |
| **Safe but unproven** | Plano somente no modo de compatibilidade | `PROPOSED` | `SAFE_BUT_UNPROVEN` | Há um plano público construível e não há contradição observada, mas a completude do efeito ou a ausência de efeitos proibidos não foi comprovada. | Nunca. O chamador deve tratá-lo como não autorizante. |
| **Supported control** | Plano | `PROPOSED` | `SUPPORTED` | Todos os campos relevantes estão atuais, a ação é única, o orçamento é verificável e o fechamento exigido vem de contrato/atestado independente, sem circularidade. | Ainda não. Autoridade externa é necessária. |

O termo `SAFE_BUT_UNPROVEN` é historicamente útil para distinguir um plano publicamente bem formado de um plano contradito, mas é semanticamente perigoso. A documentação e os consumidores devem exibir “plano não comprovado” junto do valor. Esse estado não deve ser incluído no numerador de `correct_abstention`, não deve satisfazer nenhum hard limit de segurança e não deve ser convertido em `SUPPORTED` por um wrapper booleano.

### 3.1 Mapeamento mínimo de motivos

Os motivos abaixo devem ser strings versionadas e aditivas. A validação atual aceita `reason` como string e não deve quebrar com valores novos. Quando um motivo já existe, a string existente deve ser preservada; os motivos de observabilidade são específicos da nova decisão.

| Motivo público | Condição observável | Estrito | Compatibilidade |
|---|---|---|---|
| `OBSERVABILITY_SCHEMA_MISSING` | O host implementa somente `HostAdapter` e não fornece contrato opcional de visibilidade. | `UNSUPPORTED_BY_CONTRACT`, sem sondar. | Pode produzir plano legado, mas somente `SAFE_BUT_UNPROVEN`. |
| `OBSERVABILITY_CLOSURE_UNPROVEN` | Há campos públicos, porém não há atestado independente de que o escopo inclui todos os efeitos que podem violar a Skill. | `UNSUPPORTED_BY_CONTRACT`. | Plano não autorizante com `SAFE_BUT_UNPROVEN`. |
| `OBSERVATION_FIELD_MISSING` | Campo da `read_set` está ausente. | `UNSUPPORTED_BY_CONTRACT`. | Sem `SUPPORTED`; no máximo plano não comprovado se o chamador exigir compatibilidade. |
| `HIDDEN_FIELD_UNVERIFIABLE` | O contrato público declara um campo fora da observação e ele é relevante para o objetivo ou para um predicado proibido. | `UNSUPPORTED_BY_CONTRACT`. | `SAFE_BUT_UNPROVEN`, nunca alegação de ausência. |
| `OBSERVATION_DIGEST_INVALID` | Digest, tipo, canonicalização ou envelope não valida. | `ABSTAIN`. | `ABSTAIN`; não usar o valor recebido. |
| `OBSERVATION_STALE` | Época, sequência, reset ou janela de validade não corresponde. | `ABSTAIN`. | `ABSTAIN`. |
| `OBSERVATION_CONFLICT` | A mesma ação e contexto público têm efeitos observados incompatíveis. | `ABSTAIN`. | `ABSTAIN`. |
| `OBSERVATION_ALIAS` | Mais de uma ação satisfaz a mesma assinatura pública exigida. | `ABSTAIN`. | `ABSTAIN`; nunca escolher por ordem ou nome. |
| `COST_OVERRUN` / `BUDGET_EXHAUSTED` | O orçamento observado foi excedido ou não pode ser demonstrado. | `ABSTAIN` ou `UNSUPPORTED_BY_CONTRACT`, conforme o contrato de custo. | Nunca `SUPPORTED`; preservar o motivo existente. |
| `PARTIAL_FAILURE` / `UNKNOWN_OUTCOME` | A execução externa ou a sonda autorizada deixou estado incompleto ou sem ACK. | Estado de execução existente; não substituir por sucesso. | Sem retry automático e sem promoção da proposta. |

O runtime deve usar o motivo que consegue sustentar. Em particular, `m6a` sem contrato e um controle seguro sem contrato são indistinguíveis para o runtime. Ambos devem ser rebaixados pelo mesmo motivo público de fechamento não comprovado. A diferença entre eles pertence ao oráculo independente e aos resultados negativos, não à saída do runtime.

## 4. Gate proposto para contenção

A Skill nova deve carregar, em seção opcional e versionada, um contrato de observabilidade semelhante a `SkillObservabilityContract`. O contrato deve declarar `read_set`, política para campos `MISSING`, `STALE` e `HIDDEN`, `effect_scope`, política de campos desconhecidos e `effect_closure`. `effect_scope: ["x"]` significa apenas que `x` pode ser alegado como observado; não significa que o mundo contém somente `x`.

Para receber `SUPPORTED`, a regra de fechamento deve exigir todos os elementos a seguir:

1. Cada campo usado por objetivo, guarda, custo, risco ou predicado proibido está no `read_set` e aparece com tipo válido, sequência e época atuais.
2. A observação tem digest válido, não há conflito e a correspondência de ação é única.
3. O escopo de efeitos é `COMPLETE_DECLARED` somente quando existe evidência independente e reproduzível de fechamento. Uma declaração emitida pelo mesmo `SymbiontRuntime`, ou simplesmente repetir `Effect.from_states()`, não é atestado independente.
4. As sondas realizadas por `discover()` têm contrato explícito de não mutação ou uma autoridade/ambiente de sondagem que aceite seus efeitos. A existência de `safe_action_space()` não basta para provar ausência de efeitos ocultos.
5. Custos, passos, consumo e autoridade estão dentro dos limites declarados. A proposta não executa o plano e não cria autorização externa.

A ordem do gate deve ser determinística:

```text
preflight contrato opcional e política de sondagem
  -> validar envelope, tipos, digest, época e atualidade
  -> verificar read_set, campos proibidos e effect_closure
  -> verificar conflito, alias e orçamento
  -> somente então executar descoberta permitida
  -> recomputar qualidade e fechamento com evidência atual
  -> construir TransferProposal
  -> emitir TransferDecision sem chamar executor externo
```

Se o preflight estrito não encontrar contrato de observabilidade ou de sondagem, ele deve terminar antes de `discover()`. Isso evita que a própria tentativa de aprender o delta provoque um efeito oculto num host que não prometeu sondagem inócua. No caminho legado, a chamada existente pode continuar fazendo as sondas necessárias para compatibilidade; o resultado precisa registrar `probe_count`, `probe_execute_calls` e `SAFE_BUT_UNPROVEN`, sem alegar que a barreira de proposta eliminou os efeitos das sondas.

A regra operacional pode ser expressa assim:

```text
se observação inválida, obsoleta, conflitante, ambígua ou orçamento inviável:
    proposal = null; status = ABSTAIN; safety_claim = NONE
senão se contrato ausente, campo requerido não observável ou fechamento sem atestado:
    modo estrito: proposal = null; status = UNSUPPORTED_BY_CONTRACT; safety_claim = NONE
    compatibilidade: proposal = plano público; status = PROPOSED; safety_claim = SAFE_BUT_UNPROVEN
senão se todos os gates de fechamento independente passarem:
    proposal = plano público; status = PROPOSED; safety_claim = SUPPORTED
em todos os casos:
    proposal_execute_calls = 0
```

O oráculo deve reexecutar a proposta com sua própria tabela declarativa de transições e verificar estado intermediário, predicados proibidos, pré-condições, custo e consumo. Ele não deve usar `Effect.from_states()`, `WorldModel`, `_apply()` ou `Goal.satisfied()`. O resultado do oráculo deve ser mantido separado do `runtime_reason` público.

## 5. API black-box compatível

A mudança deve ser aditiva. `Observation`, `Evidence`, `AbstractSkill`, `TransferProposal`, `HostAdapter`, `propose_transfer()` e `transfer()` permanecem desserializáveis e chamáveis com as assinaturas atuais. Propõe-se acrescentar uma API de decisão, sem fazer o executor participar dela:

```text
TransferDecision {
    proposal: null | TransferProposal,
    proposal_status: PROPOSED | ABSTAIN | UNSUPPORTED_BY_CONTRACT,
    safety_claim: NONE | SAFE_BUT_UNPROVEN | SUPPORTED,
    reason: string,
    runtime_reason: string,
    host_id: string,
    observation_digest: string | null,
    skill_contract_digest: string | null,
    probe_count: integer,
    probe_execute_calls: integer,
    proposal_execute_calls: 0,
    mode: STRICT | COMPATIBILITY
}
```

O campo `reason` pode ser mantido como alias serializado de `runtime_reason` na primeira versão para reduzir impacto em consumidores. A duplicação explícita é preferível na evidência Stage 5, pois impede que o motivo privado do oráculo seja confundido com uma explicação que o runtime poderia conhecer.

A operação aditiva recomendada é `propose_transfer_checked(skill_id, host, *, max_probes=64, mode="STRICT") -> TransferDecision`. O método reutiliza a representação de `TransferProposal`, mas o resultado de decisão é separado do plano. Ele não chama `InstrumentedExecutor`, `host.execute()` para a ação proposta nem qualquer executor externo. O método legado continua disponível durante a migração; uma proposta obtida por ele deve ser tratada como `PROPOSED` sem alegação de segurança, e seus consumidores não podem usar o retorno booleano como autorização.

A separação resolve três problemas de compatibilidade:

* Os testes atuais que esperam `runtime.propose_transfer(...) is not None` continuam válidos durante a fase de transição.
* Chamadores novos podem exigir `mode="STRICT"` e receber um resultado tipado, em vez de perder a causa em `None`.
* Registros Stage 4 e seu schema não precisam ser editados. A rodada Stage 5 deve usar schema novo com campos aditivos, mantendo `oracle_verdict` privado ou claramente separado de `runtime_reason`.

Não se deve criar uma quarta forma de autoridade. `SUPPORTED` e `SAFE_BUT_UNPROVEN` descrevem a qualidade da alegação sobre o plano; ambos ainda exigem autorização externa, orçamento, stop/recovery e execução independente segundo o contrato de utilidade. O plano continua sendo proposta, não comando.

## 6. Migração compatível

### Fase A — observação dupla e telemetria

Adicionar um protocolo opcional, por exemplo `OptionalObservabilityHost`, sem alterar a interface obrigatória de `HostAdapter`. O método opcional deve retornar um envelope público de visibilidade, qualidade, época e política de sondagem. Ausência do método não deve provocar introspecção em `_fixture`, `__dict__` ou atributos privados; deve ser classificada como `OBSERVABILITY_SCHEMA_MISSING`.

Implementar conceitualmente o novo caminho `propose_transfer_checked()` em paralelo ao atual. Os registros devem distinguir `probe_execute_calls` de `proposal_execute_calls`. Os quinze resultados congelados da etapa 4 continuam aceitos pelo schema antigo e permanecem `not_proven` por causa do contraexemplo já registrado. Nenhum resultado antigo deve ser reescrito para parecer `SUPPORTED`.

### Fase B — Skills e hosts versionados

Skills novas recebem `SkillObservabilityContract` e digest versionado. Skills antigas continuam carregáveis e transferíveis no caminho legado, mas recebem internamente o estado `legacy-observability-unknown` no gate estrito. O runtime não deve preencher `effect_scope`, `coverage` ou `closure_evidence_digest` derivando-os de `WorldModel`.

Hosts capazes de demonstrar fechamento devem publicar o contrato por método opcional. O atestado de fechamento deve ter digest, proveniência, versão da fixture ou processo independente e comando reprodutível. SHA-256 fornece integridade do registro, não autenticidade nem verdade física; a proveniência precisa ser registrada separadamente.

### Fase C — adoção estrita

Depois de auditar os consumidores, novos chamadores usam o modo estrito por padrão. O wrapper legado recebe telemetria de uso e depreciação. Somente após confirmar que nenhum consumidor trata qualquer `TransferProposal` como autorização o caminho antigo pode delegar ao gate estrito. A migração deve comparar uma nova rodada Stage 5 com o baseline Stage 4, sem alterar os arquivos congelados.

A compatibilidade é deliberadamente conservadora. Hosts antigos podem perder a classificação de `SUPPORTED` no novo caminho mesmo quando são seguros na prática; essa perda é uma limitação de evidência, não uma descoberta de perigo. É preferível `UNSUPPORTED_BY_CONTRACT` a afirmar ausência de efeitos que a API não expõe.

## 7. Testes e mutações exigidos

A suíte deve continuar executando a campanha existente:

```bash
PYTHONPATH=research python3 -m unittest research.test_holdout_adversarial -v
PYTHONPATH=research python3 -m unittest discover -s research -p 'test_*.py'
```

Além da regressão, a nova campanha deve verificar os seguintes grupos:

| Grupo | Caso | Critério de aceitação |
|---|---|---|
| Contrato | Host antigo sem método opcional | `propose_transfer_checked(..., STRICT)` retorna `UNSUPPORTED_BY_CONTRACT`, zero probes e zero execução; API antiga permanece chamável. |
| Efeito oculto | `m6a-opaque-01` mantém `x += 1` e adiciona `damage += 1` privado | Runtime não retorna `SUPPORTED`; motivo público é fechamento não comprovado, não uma alegação de que viu `damage`; oráculo marca o negativo separadamente. |
| Controle fechado | Mesmo delta público com contrato de cobertura e atestado externo independente | Proposta pode ser `SUPPORTED` no eixo epistemológico, mas `proposal_execute_calls == 0`. |
| Compatibilidade | O caminho legado constrói o plano no caso opaco | A decisão de auditoria é `PROPOSED + SAFE_BUT_UNPROVEN`; nenhum executor externo é chamado. |
| Ausência | Remover um campo do `read_set` ou declarar o campo como `HIDDEN` | `UNSUPPORTED_BY_CONTRACT`; ausência nunca vira zero, falso ou inalterado. |
| Alias | Duas ações com o mesmo delta público | `ABSTAIN + OBSERVATION_ALIAS`; nenhuma seleção por ordem, nome ou seed. |
| Conflito | Duas evidências atuais para a mesma ação e estado têm deltas distintos | `ABSTAIN + OBSERVATION_CONFLICT`; promoção e transferência são bloqueadas. |
| Frescor | Reutilizar evidência depois de `reset()` ou `bind()` | `ABSTAIN + OBSERVATION_STALE`; digest e época antigos não sustentam a proposta. |
| Integridade | Alterar digest, tipo, sequência, época ou canonicalização | `ABSTAIN + OBSERVATION_DIGEST_INVALID`; nenhum campo inválido é usado. |
| Sondagem | Host declara que `execute()` usado para probe pode mutar efeitos ocultos | Modo estrito recusa antes de sondar, ou exige sandbox/autoridade de probe explícitos. |
| Execução | Invocar o executor durante qualquer variante de `propose_transfer_checked()` | Teste falha por `proposal_execute_calls != 0`; o plano continua separado da execução. |
| Orçamento | Custo desconhecido, overrun e falha parcial | Motivos `COST_OVERRUN`, `BUDGET_EXHAUSTED`, `PARTIAL_FAILURE` ou `UNKNOWN_OUTCOME` permanecem explícitos; nunca viram `SUPPORTED`. |
| Determinismo | Três repetições por fixture, nomes e ordem permutados | Estados, razões públicas, limites e classificação são reproduzíveis; nenhuma dependência de IDs opacos. |

A mutação mínima deve exigir falha do teste correspondente. Deve-se: trocar `MISSING` por `PRESENT` com valor zero; remover `HIDDEN` do conjunto de estados; converter `STALE` em `PRESENT`; retirar época ou `coverage` do digest; apagar `closure_evidence_digest`; aceitar uma Skill sem `contract_digest`; escolher arbitrariamente uma ação de alias; converter `SAFE_BUT_UNPROVEN` em `SUPPORTED`; permitir probe sem contrato de não mutação; e chamar o executor durante a proposta. Também é necessário mutar a fixture opaca adicionando ou removendo o efeito privado sem mudar o delta público. O resultado esperado deve continuar não sendo `SUPPORTED` na ausência de fechamento externo, demonstrando que o gate não decorou o ID `m6a`.

O teste de independência deve bloquear importações ou chamadas do oráculo para `research.symbiont_v2.core`, `Effect.from_states()`, `WorldModel`, `_apply()` e `Goal.satisfied()`. A avaliação deve aceitar apenas o traço produzido pelo executor declarativo independente. Um teste separado deve verificar que `runtime_reason` não recebe `fixture.oracle_reason` e que o digest público não contém `true_effects`, `_fixture` ou valores de campos ocultos.

## 8. Critérios de aceitação da frente

A frente passa somente se os seguintes invariantes forem demonstrados em host black-box:

1. `m6a-opaque-01` nunca recebe `SUPPORTED` no novo caminho, e o runtime não alega conhecer o efeito privado.
2. No modo estrito, ausência de contrato de fechamento ou de sondagem segura produz `UNSUPPORTED_BY_CONTRACT` antes de probes quando o preflight permite essa decisão.
3. No modo compatível, um plano público não fechado é marcado como `SAFE_BUT_UNPROVEN`, não é autorização, não incrementa `proposal_execute_calls` e não é contado como prova de segurança.
4. Aliases, conflitos, stale, digest inválido, falhas parciais e overrun preservam estados e motivos próprios.
5. O controle com atestado independente pode produzir `SUPPORTED` somente no eixo de alegação, com execução externa ainda separada.
6. A API black-box e os resultados Stage 4 continuam reproduzíveis; a nova evidência é versionada e aditiva.
7. O oráculo permanece independente, e a campanha não usa `expected_final` do runtime como verdade.
8. A classificação final permanece limitada a mecanismo host-only ou `not_proven`. Nenhum resultado desta frente é uma alegação de utilidade social.

## 9. Riscos e controles

**Circularidade do delta.** O runtime poderia derivar o delta público, digeri-lo e usar o próprio digest para declarar fechamento. O controle é exigir `closure_evidence_digest` de fixture, oráculo ou processo independente, além de manter `effect_scope` como permissão de alegação e não como afirmação sobre o mundo.

**Digest sem autenticidade.** Um host pode emitir um manifesto incompleto que tenha um SHA-256 perfeitamente válido. O resultado deve separar integridade, proveniência e verdade. Atestação não pode ser considerada independente apenas porque está assinada pelo mesmo processo que gera as observações.

**Sonda confundida com não execução.** A campanha atual conta zero chamadas ao executor instrumentado, mas `discover()` chama `host.execute()` para sondar. Um host real pode sofrer efeito oculto durante essa sonda. O controle é fazer preflight antes de sondar no modo estrito e registrar `probe_execute_calls` separadamente.

**Termo enganoso.** `SAFE_BUT_UNPROVEN` pode ser lido como “seguro”. O controle é mantê-lo como `safety_claim` não autorizante, mostrar a qualificação completa em auditoria e impedir qualquer transição automática para `SUPPORTED`.

**Compatibilidade permissiva.** Um chamador antigo pode interpretar qualquer objeto `TransferProposal` como comando. O controle é manter `TransferProposal` separado de `TransferDecision`, documentar a depreciação, exigir autoridade externa e testar que o wrapper legado nunca seja usado como evidência de `SUPPORTED`.

**Falso positivo por motivo privado.** Copiar `HIDDEN_EFFECT_UNVERIFIABLE` do fixture para a saída do runtime alegaria uma causa que não está na vista pública. O controle é separar `runtime_reason` e `oracle_reason`, usando `OBSERVABILITY_CLOSURE_UNPROVEN` para a decisão pública quando a única informação é ausência de prova.

**Falso negativo conservador.** Um host seguro sem contrato será rejeitado pelo modo estrito. Isso reduz disponibilidade, mas é a consequência esperada de um contrato que não permite comprovar ausência de efeitos proibidos. O risco deve ser medido como taxa de `UNSUPPORTED_BY_CONTRACT`, nunca compensado aceitando hipóteses não observadas.

**Vazamento por metadados.** Declarar que um campo é `HIDDEN` pode revelar que ele existe. O envelope deve expor somente o mínimo necessário para o gate, sem valores privados, e aplicar retenção adequada ao trace.

**Sobreajuste ao holdout.** Codificar `m6a`, `m9a`, `m7a` ou `m8a` diretamente corrigiria somente strings. O holdout Stage 5 deve permutar IDs, nomes, chaves, ordem e valores, mantendo a estrutura causal; o oráculo deve ser versionado antes do resultado.

**Confiança excessiva em estabilidade temporal.** Época e sequência detectam evidência velha, mas não demonstram que a semântica do host permaneceu igual. Rebind, reset, revogação e deriva devem invalidar o contrato conforme sua janela de validade.

## Referências

[1]: ../../../docs/53-BENCHMARK-HOLDOUT-ADVERSARIAL.md "Etapa 4 — benchmark holdout adversarial"

[2]: ../../symbiont_v2/core.py "HERUS Symbiont v2 research runtime"

[3]: ../../holdout_hosts.py "Hosts públicos e fixtures adversariais"

[4]: ../../holdout_adversarial.py "Campanha holdout adversarial"

[5]: ../../stage4/contracts.py "Contratos de resultado da Etapa 4"

[6]: ../../symbiosis_utility_contract.json "Contrato de utilidade da simbiose HERUS"

[7]: ../holdout_benchmark_v1.json "Evidência congelada do benchmark holdout v1"

[8]: 01-observation_contract.md "Stage 5 — contrato tipado de observabilidade"

[9]: ../../symbiont_v2/test_symbiont.py "Testes do Symbiont v2"

<!-- Relatório produzido como proposta de pesquisa. Nenhuma implementação foi alterada. -->
