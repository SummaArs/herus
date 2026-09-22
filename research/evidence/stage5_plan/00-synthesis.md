# HERUS Etapa 5 — síntese arquitetural mínima

**ID:** `HERUS-S5-SYN-006`  
**Escopo:** corrigir o contraexemplo `m6a-opaque-01` no caminho estrito, sem converter o runtime em autoridade, e tornar orçamento, falha parcial e diagnóstico observáveis.  
**Decisão:** implementar uma API aditiva de decisão tipada com pré-voo de contrato, um ledger público append-only e um resultado de execução externo separado. Manter `transfer()` e `propose_transfer()` inalterados durante a migração.

## Conclusão

A menor alteração que elimina o contraexemplo não é tentar descobrir `damage` no runtime. Isso seria impossível pela superfície black-box e faria o runtime assumir autoridade sobre um fato privado. A alteração mínima é impedir a alegação que o delta público não sustenta.

Adicionar `propose_transfer_checked(..., mode="STRICT") -> TransferDecision`. Antes de qualquer `discover()` ou `host.execute()`, esse caminho exige um contrato opcional de observabilidade da Skill e do host, uma declaração de fechamento de efeitos com atestação independente e um limite de custo verificável. Se qualquer requisito faltar, retorna `proposal: None`, `proposal_status: UNSUPPORTED_BY_CONTRACT`, `safety_claim: NONE` e motivo público `OBSERVABILITY_SCHEMA_MISSING` ou `OBSERVABILITY_CLOSURE_UNPROVEN`. Em `m6a-opaque-01`, portanto, não há proposta apoiada nem sonda feita pelo caminho estrito; o runtime não afirma ter visto `damage`.

O caminho antigo continua sendo um caminho de compatibilidade. Ele pode continuar retornando o `TransferProposal` que os consumidores atuais esperam, mas esse objeto permanece apenas um plano. A camada de auditoria deve rotulá-lo como `PROPOSED + SAFE_BUT_UNPROVEN`, nunca como `SUPPORTED` ou autorização. O arquivo congelado `holdout_benchmark_v1.json` continua histórico e não é reescrito.

A linha de base observada antes da mudança é verde: os testes do runtime v2 executam 9 casos e os testes do holdout executam 5 casos. Ela também confirma a lacuna: `PublicHoldoutHost` mantém `true_effects` privado, `discover()` aprende por `host.execute()`, e `propose_transfer()` deriva o efeito apenas com `Effect.from_states()`. No holdout v1, `m6a-opaque-01` ainda aparece como `FAIL_UNSAFE` com `HIDDEN_EFFECT_UNVERIFIABLE`; essa razão vem do oráculo privado e não pode ser copiada para `runtime_reason`.

## Diagnóstico do código atual

`Observation.valid()` comprova somente que o digest corresponde a `sequence`, estado e ação recebidos. Não comprova cobertura semântica. `Effect.from_states()` calcula a diferença dos campos públicos e não distingue um campo ausente de um campo existente que não mudou. `propose_transfer()` agrupa evidências por `effect.delta`, verifica a meta e constrói `TransferProposal`, mas não possui um tipo para cobertura ou fechamento dos efeitos.

Há ainda duas lacunas de orçamento. `DiscoveryBudget` limita `max_probes`, mas o parâmetro `max_probes` de `propose_transfer()` somente fatia candidatos depois da descoberta. Além disso, o harness calcula `cost_actual` depois da proposta usando a verdade da fixture; isso mede o holdout, não prova que o runtime reservou ou aplicou um orçamento.

A barreira proposta/execução existente deve ser preservada. Construir `TransferProposal` não deve chamar o executor do plano. Porém, uma probe de discovery ainda chama `host.execute()` e pode ser mutante em um host que não declarou o contrário. A telemetria Stage 5 precisa separar `probe_execute_calls` de `proposal_execute_calls`.

## Alteração mínima implementável

A implementação deve adicionar `research/symbiont_v2/stage5.py` e um método pequeno em `SymbiontRuntime`. O módulo novo contém tipos serializáveis, validação e ledger; `core.py` somente fornece o caminho checked e um helper privado de discovery limitado. Não se deve editar `stage4/contracts.py`, o schema v1 ou a semântica de `transfer()` e `propose_transfer()` nesta fase.

### Tipos compatíveis

Os nomes abaixo reutilizam os estados já presentes em `research/stage4/contracts.py` e adicionam apenas estados aditivos.

```text
ProposalStatus = Status.PROPOSED | Status.ABSTAIN | Status.UNSUPPORTED_BY_CONTRACT
SafetyClaim    = NONE | SAFE_BUT_UNPROVEN | SUPPORTED
DecisionMode   = STRICT | COMPATIBILITY

FieldStatus    = PRESENT | MISSING | STALE | HIDDEN | UNDECLARED
Coverage       = COMPLETE_DECLARED | PARTIAL | UNDECLARED
ProbeMode      = NON_MUTATING | MUTATING_AUTHORIZED | UNKNOWN

CostStatus     = EXACT | BOUNDED_UNKNOWN | UNKNOWN | OVERRUN
BudgetState    = OPEN | EXHAUSTED | OVERRUN | UNKNOWN

ExecutionStatus = REJECTED | AUTHORIZED | IN_FLIGHT | COMMITTED
                  | FAILED_PARTIAL | UNKNOWN_OUTCOME
                  | RECOVERY_REQUIRED | RECOVERY_RESOLVED
```

`UNDECLARED` é um estado de cobertura, não deve ser confundido com `MISSING` ou `HIDDEN`. O runtime só pode emitir `HIDDEN` quando o contrato público do host o declarar. Ausência sem declaração é `UNDECLARED` no contrato e `MISSING` somente quando o campo fizer parte do `read_set` exigido pela Skill. Nenhum desses estados fornece valor implícito; `MISSING` e `HIDDEN` serializam `value: null`.

A Skill recebe um campo opcional ao fim de `AbstractSkill`, preservando chamadas posicionais existentes:

```text
SkillObservabilityContract {
    schema: "herus-skill-observability-v1",
    read_set: [{field, required_status, type, max_age_sequences}],
    effect_scope: [field],
    coverage_requirement: COMPLETE_DECLARED | OBSERVED_FIELDS_ONLY,
    unknown_field_policy: ABSTAIN,
    stale_field_policy: ABSTAIN,
    hidden_field_policy: BLOCK,
    effect_closure: EXTERNAL_ATTESTATION_REQUIRED | NOT_CLAIMED,
    closure_evidence_digest: string | null,
    contract_digest: string
}
```

A ausência desse campo em uma Skill antiga não significa cobertura completa. A Skill antiga continua válida para o caminho legado, mas é `legacy-observability-unknown` no caminho estrito. Para não alterar digests históricos, o campo só entra na canonicalização de uma Skill quando não é nulo; Skills antigas conservam o digest legado.

O host não recebe métodos obrigatórios novos. Hosts novos podem implementar protocolos opcionais separados, para não acoplar observabilidade a orçamento:

```text
OptionalObservabilityHost.observability_contract() -> HostObservabilityContract
OptionalBudgetHost.budget_contract() -> BudgetContract
```

`HostObservabilityContract` publica `schema`, `coverage`, `probe_mode`, época, estados públicos de campo, `effect_closure`, `closure_evidence_digest`, proveniência e `contract_digest`. `BudgetContract` publica cotações `EXACT`, `UPPER_BOUND` ou `UNKNOWN` para probe, reset e execução. O runtime não inspeciona `__dict__`, `_fixture`, `true_effects`, `WorldModel` ou qualquer atributo privado para preencher esses objetos.

### Decisão, diagnóstico e custo

```text
TransferDecision {
    proposal: TransferProposal | null,
    proposal_status: ProposalStatus,
    safety_claim: SafetyClaim,
    runtime_reason: string,
    diagnostic: Diagnostic,
    observation_quality: ObservationQuality,
    budget_ledger: BudgetLedger,
    planned_steps: uint,
    host_id: string,
    observation_digest: string | null,
    skill_contract_digest: string | null,
    probe_execute_calls: uint,
    proposal_execute_calls: 0,
    mode: DecisionMode
}
```

`reason` pode ser mantido como propriedade/alias serializado de `runtime_reason` para consumidores que já leem esse nome. `Diagnostic` deve conter `phase` (`PREFLIGHT`, `DISCOVERY`, `SYNTHESIS` ou `EXECUTION`), `code`, uma descrição curta e somente digests/evidências públicas. `oracle_reason` fica fora de `TransferDecision`; o harness pode registrá-lo em campo separado. Assim, `m6a` produz `OBSERVABILITY_CLOSURE_UNPROVEN` quando o runtime só sabe que não tem prova de fechamento, e não `HIDDEN_EFFECT_UNVERIFIABLE`, que é conhecimento privado do oráculo.

`BudgetLimits` deve aceitar os aliases antigos sem criar uma segunda semântica:

```text
BudgetLimits {
    probe_max: uint,       # alias de max_probes
    reset_max: uint,
    step_max: uint,        # alias de max_steps
    cost_max: uint,        # alias de max_cost
    retry_max: uint = 0,
    recovery_max: uint = 0
}
```

Valores ausentes, negativos, fracionários, booleanos e aliases conflitantes são rejeitados no modo estrito. Ausência de `cost_max` não significa infinito.

```text
BudgetLedger {
    schema: "herus-budget-ledger-v1",
    budget_scope_id: string,
    parent_ledger_digest: string | null,
    limits: BudgetLimits,
    probe_count: uint,
    reset_count: uint,
    step_count: uint,
    cost_count: uint,
    retry_count: uint,
    recovery_count: uint,
    cost_status: CostStatus,
    cost_actual: uint | null,
    cost_upper_bound: uint | null,
    unknown_event_count: uint,
    budget_state: BudgetState,
    event_seq: uint,
    ledger_digest: string
}
```

O ledger é append-only e monotônico. A tentativa é cobrada antes de `host.execute()` ou `reset()`, inclusive quando lança exceção ou não retorna. `cost_actual` só é numérico quando todos os eventos possuem medição exata ou recibo explícito `MEASURED_ZERO`; caso contrário é `null`. `step_count` permanece zero durante qualquer construção de proposta, enquanto `planned_steps` pode ser positivo. `reset()` não zera contadores nem é recuperação.

### Falha parcial sem autoridade no runtime

A execução não entra em `propose_transfer_checked()`. Um adaptador externo recebe uma proposta e uma autorização emitida fora do runtime. Seu resultado deve usar `ExecutionStatus` e um relatório separado:

```text
ExecutionReport {
    status: ExecutionStatus,
    runtime_reason: string,
    committed_steps: uint | null,
    known_steps: uint,
    retry_count: uint,
    recovery_count: uint,
    authorized_calls: uint,
    external_effect_count: uint,
    ledger_digest: string,
    recovery_required: bool
}
```

Falha explícita depois de um commit é `FAILED_PARTIAL + RECOVERY_REQUIRED`. Timeout, ACK ausente, crash ou recibos conflitantes são `UNKNOWN_OUTCOME + RECOVERY_REQUIRED`; ausência de ACK nunca vira “zero efeito”. Não há retry automático. Um retry exige reconciliação independente, novo nonce/envelope e nova solicitação externa; esses mecanismos pertencem ao adaptador executor, não ao runtime de proposta. `TransferProposal` não ganha campos de autoridade e nunca é aceito como autorização.

## Gate determinístico

`propose_transfer_checked()` deve executar esta sequência:

1. Validar Skill, modo, limites e aliases de orçamento.
2. Consultar contratos opcionais sem introspecção privada.
3. Se a Skill ou o host não tiver contrato de observabilidade, retornar `UNSUPPORTED_BY_CONTRACT + OBSERVABILITY_SCHEMA_MISSING` antes de sondar em `STRICT`.
4. Se a cobertura, `read_set`, atualidade, fechamento ou `closure_evidence_digest` forem insuficientes, retornar `UNSUPPORTED_BY_CONTRACT + OBSERVABILITY_CLOSURE_UNPROVEN`, `OBSERVATION_FIELD_MISSING` ou `HIDDEN_FIELD_UNVERIFIABLE`, conforme a evidência pública.
5. Validar cotação e `cost_max`. Sem limite ou sem medição/limite superior finito, retornar `COST_LIMIT_MISSING` ou `UNKNOWN_COST`; não assumir zero.
6. Somente se o preflight passar, executar discovery limitado por `min(runtime_probe_max, call_probe_max, budget.probe_max)`, cobrando cada evento antes da chamada.
7. Revalidar digest, época, stale, conflito, alias, orçamento e fechamento com a evidência atual.
8. Construir o `TransferProposal` sem chamar executor. Se os gates independentes passarem, emitir `PROPOSED + SUPPORTED`; caso exista apenas um plano público não fechado em `COMPATIBILITY`, emitir `PROPOSED + SAFE_BUT_UNPROVEN`.

A regra para `m6a-opaque-01` é deliberadamente indistinguível de um host seguro sem contrato: o runtime não vê `damage` e não deve tentar inferi-lo. A diferença entre os dois casos fica no oráculo independente e no trace privado da campanha, não na decisão pública.

## Migração sem quebrar as APIs

### Fase 0 — congelar a linha de base

Executar e guardar os comandos históricos:

```bash
PYTHONPATH=research python3 -m unittest research.symbiont_v2.test_symbiont -v
PYTHONPATH=research python3 -m unittest research.test_holdout_adversarial -v
PYTHONPATH=research python3 -m unittest discover -s research -p 'test_*.py'
```

Não alterar `holdout_benchmark_v1.json`, `holdout_benchmark_schema.json` nem `stage4/contracts.py`. O baseline continuará podendo registrar o `FAIL_UNSAFE` histórico do m6a.

### Fase 1 — tipos e telemetria aditiva

Adicionar `stage5.py`, os enums, contratos e `BudgetLedger`. Acrescentar apenas campos opcionais ao `AbstractSkill` e aos registros Stage 5. `Observation.make()` e `Observation.valid()` sem envelope continuam usando o digest atual. Hosts antigos são adaptados como `coverage: UNDECLARED`, `effect_closure: NOT_CLAIMED` e custo `UNKNOWN`/`BOUNDED_UNKNOWN`; nenhuma propriedade privada é lida.

### Fase 2 — decisão checked opt-in

Adicionar `propose_transfer_checked(skill_id, host, *, budget=None, max_probes=64, mode="STRICT", budget_scope_id=None)`. O parâmetro legado `max_probes` continua aceito e passa a ser efetivo somente nessa API nova, antes da primeira probe. `transfer()` continua retornando `bool`; `propose_transfer()` continua retornando `TransferProposal | None`. Nenhum dos dois muda de assinatura ou passa a emitir autoridade.

No modo `COMPATIBILITY`, a implementação pode chamar o método legado e anexar ledger/diagnóstico. Uma proposta não nula do caso opaco recebe `SAFE_BUT_UNPROVEN`. No modo `STRICT`, o caso opaco sem contrato retorna proposta nula e zero probes; esse é o gate que elimina o contraexemplo sem quebrar consumidores antigos.

### Fase 3 — executor externo e adoção

Adicionar o adaptador de teste externo e `ExecutionReport`, mantendo `InstrumentedExecutor` e registros Stage 4. Testar `FAILED_PARTIAL`, `UNKNOWN_OUTCOME`, `RECOVERY_REQUIRED` e `RETRY_BLOCKED` fora do runtime. Depois de auditar consumidores, novos fluxos usam `propose_transfer_checked(..., STRICT)` e um executor externo com autoridade. O wrapper legado recebe telemetria e depreciação; somente em uma fase posterior poderá delegar ao gate estrito por configuração explícita, preservando seu tipo de retorno histórico.

## Testes de aceitação

A nova suíte deve ser `research/test_stage5_synthesis.py` ou equivalente, sem substituir a suíte histórica. Ela deve usar fixtures declarativas e IDs permutáveis, nunca regras especiais para `m6a`, `m7a`, `m8a` ou `m9a`.

| ID | Caso | Aceitação mínima |
|---|---|---|
| `S5-BASE-01` | Baseline congelado | Os 9 testes do runtime, os 5 do holdout e os 15 registros v1 continuam válidos; nenhum arquivo v1 é reescrito. |
| `S5-M6A-01` | Mesmo delta público, efeito privado diferente | Duas fixtures expõem o mesmo `x += 1`; sem atestado externo, ambas são `UNSUPPORTED_BY_CONTRACT` no estrito e nunca `SUPPORTED`. O runtime não recebe a razão privada do oráculo. |
| `S5-CONTRACT-01` | Host antigo | `propose_transfer_checked(..., STRICT)` retorna `UNSUPPORTED_BY_CONTRACT + OBSERVABILITY_SCHEMA_MISSING`, zero probes e zero execução; a API antiga continua chamável. |
| `S5-CLOSURE-01` | Digest de fechamento criado pelo próprio runtime | Retorna `OBSERVABILITY_CLOSURE_UNPROVEN`; um digest de bytes não prova independência nem completude. |
| `S5-FIELD-01` | Campo ausente/oculto/stale | `MISSING`, `HIDDEN` e `STALE` permanecem distintos; nenhum valor vira zero e nenhum campo inválido satisfaz a meta. |
| `S5-ALIAS-01` | Alias público | `ABSTAIN + OBSERVATION_ALIAS`, proposta nula e nenhuma escolha por ordem. |
| `S5-COST-01` | Custo ausente/desconhecido | `cost_status == UNKNOWN`, `cost_actual is None`, `unknown_event_count > 0` e `UNKNOWN_COST` ou `COST_LIMIT_MISSING`; nunca zero ou infinito. |
| `S5-COST-02` | Limite e overrun | A chamada que não cabe não inicia; recibo real acima do limite produz `OVERRUN + COST_OVERRUN` e fecha novas operações. |
| `S5-LEDGER-01` | Falha depois da cobrança | Probe, reset e exceção são contados antes da chamada; contadores não diminuem após reset, bind ou snapshot velho. |
| `S5-SEP-01` | Barreira de execução | Em qualquer modo de proposta, `proposal_execute_calls == 0`, `step_count == 0`, `authorized_calls == 0` e `external_effect_count == 0`. |
| `S5-PARTIAL-01` | Falha parcial externa | Commit seguido de falha produz `FAILED_PARTIAL + RECOVERY_REQUIRED`, preserva `committed_steps`, não faz rollback fictício e não tenta novamente. |
| `S5-UNKNOWN-01` | ACK ausente | Produz `UNKNOWN_OUTCOME + RECOVERY_REQUIRED`; `committed_steps` permanece nulo/desconhecido e retry direto é `RETRY_BLOCKED`. |
| `S5-ORACLE-01` | Independência | O oráculo não importa `core.py`, não usa `Effect.from_states()`, `_apply()`, `WorldModel` ou `Goal.satisfied()`, e mantém `oracle_reason` separado. |
| `S5-VAR-01` | Generalização | Renomear fixture, ações, campos e ordem preserva as classificações; a implementação não reconhece IDs do holdout. |

Mutações obrigatórias: transformar `MISSING` em `PRESENT` com zero; remover fechamento do gate; aceitar custo desconhecido como zero; zerar o ledger em `reset()`; contar probe como execução autorizada; chamar executor dentro da proposta; converter `SAFE_BUT_UNPROVEN` ou `UNKNOWN_OUTCOME` em `SUPPORTED`; permitir retry sem recuperação; e copiar `oracle_reason` para `runtime_reason`. Cada mutação deve ser morta por um teste focado e por uma fixture renomeada.

## Ordem de implementação recomendada

1. Congelar baseline e adicionar o schema de evidência Stage 5, sem editar artefatos v1.
2. Implementar `stage5.py` com enums, validação de contratos, aliases de orçamento, `Diagnostic` e `BudgetLedger` imutável por evento.
3. Acrescentar contrato opcional à Skill e ao host, preservando digests e assinaturas legadas.
4. Implementar `propose_transfer_checked()` com preflight antes de `discover()` e helper de discovery que aplica o limite efetivo antes da primeira chamada.
5. Integrar cobrança de probe/reset e serialização do diagnóstico. Validar `proposal_execute_calls == 0`.
6. Adicionar `ExecutionReport` e adaptador externo de teste para falha parcial, resultado desconhecido, recuperação e bloqueio de retry; não passar executor ao runtime.
7. Executar regressões, propriedades geradas e mutações. Só depois habilitar `STRICT` para novos chamadores.

## Riscos e limites

**Falso negativo conservador.** Um host seguro sem contrato será bloqueado no modo estrito. Isso é perda de evidência, não prova de perigo, e é preferível a alegar fechamento inexistente.

**Digest circular.** SHA-256 prova integridade do registro recebido, não autenticidade, independência ou completude. `closure_evidence_digest` só vale quando a proveniência e o emissor são externos ao runtime.

**Sonda mutante.** `probe_count` mede consumo, mas não torna `host.execute()` inócuo. `probe_mode: UNKNOWN` deve bloquear o modo estrito quando a Skill depender de não mutação.

**Compatibilidade permissiva.** Consumidores antigos podem tratar qualquer proposta como comando. A documentação, os testes de barreira, a ausência de autoridade em `TransferProposal` e a depreciação do wrapper devem impedir essa interpretação. Não se deve mudar silenciosamente o tipo de retorno.

**Estado parcial.** `reset()` não é reconciliação. Falha parcial e ACK ausente permanecem estados de execução e exigem recuperação externa; não são convertidos em sucesso por coincidência de `expected_final`.

**Escopo.** Esta síntese propõe um mecanismo host-only e uma classificação `not_proven` quando os gates não passam. Não mede utilidade social, segurança física, consciência ou inteligência geral.

## Referências

[1]: ./01-observation_contract.md "Stage 5 — contrato tipado de observabilidade"
[2]: ./02-hidden_effect_containment.md "Stage 5 — contenção de efeitos ocultos"
[3]: ./03-partial_failure_executor.md "Stage 5 — envelope externo, executor e falha parcial"
[4]: ./04-budget_cost_model.md "Stage 5 — modelo de custo e orçamento"
[5]: ./05-regression_and_mutations.md "Stage 5 — testes de regressão e mutações"
[6]: ../../symbiont_v2/core.py "HERUS Symbiont v2 research runtime"
[7]: ../../holdout_hosts.py "Hosts públicos e fixtures adversariais"
[8]: ../../holdout_adversarial.py "Campanha holdout adversarial"
[9]: ../../stage4/contracts.py "Contratos de resultado da Etapa 4"
[10]: ../holdout_benchmark_v1.json "Evidência congelada do benchmark holdout v1"
[11]: ../../holdout_benchmark_schema.json "Schema do benchmark holdout Stage 4"

<!-- Síntese arquitetural produzida a partir dos cinco relatórios Stage 5 e do código atual. Nenhuma alteração de runtime foi feita por esta revisão. -->
