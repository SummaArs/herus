# Stage 5 — envelope externo, executor e falha parcial

**ID:** `HERUS-S5-EXEC-003`  
**Frente:** executor e falha parcial  
**Status:** proposta de desenho; nenhuma alteração de código foi feita nesta frente  
**Escopo:** mecanismo host-only, autoridade externa e recuperação transacional. Este relatório não mede nem alega utilidade social, benefício humano, acessibilidade real, segurança física, consciência ou inteligência geral.

## 1. Decisão

A execução autorizada deve ser um protocolo externo e de uso único, separado de `TransferProposal`. O runtime pode construir um plano a partir da superfície black-box, mas não pode emitir autoridade, assinar um envelope, escolher uma política de retry ou chamar o executor. A autoridade deve vir de um emissor externo identificável, ser limitada a uma proposta exata e ser validada por um adaptador executor antes de qualquer efeito.

O executor deve manter um ledger append-only de tentativas, nonces, recibos e decisões de recuperação. Ele deve reservar o nonce antes do primeiro efeito e consumir o nonce mesmo quando a chamada for rejeitada após a reserva. Um timeout ou ACK ausente não prova que a ação não ocorreu. Por isso, `UNKNOWN_OUTCOME` deve bloquear o retry até uma reconciliação externa. Uma falha explícita depois de um ou mais commits deve produzir `FAILED_PARTIAL` e também uma trava `RECOVERY_REQUIRED`; não deve haver rollback implícito nem retry automático.

A migração deve ser aditiva. `HostAdapter`, `Observation`, `Evidence`, `AbstractSkill`, `TransferProposal`, `transfer()` e `propose_transfer()` conservam suas assinaturas e seus significados atuais. A API antiga continua sendo uma API de plano. Uma API nova, sugerida como `propose_transfer_checked()`, pode retornar uma decisão tipada, mas essa decisão também não é autorização. A autorização só existe no envelope externo aceito pelo executor.

A linha de base da Etapa 4 permanece congelada. Em especial, `m7a-partial-01` continua com o registro histórico que preserva `oracle_verdict: UNKNOWN_OUTCOME` e `reason: PARTIAL_FAILURE`; a nova campanha acrescenta registros de execução, sem reescrever `holdout_benchmark_v1.json` [1] [2] [3].

## 2. Limite atual que motiva a correção

O `TransferProposal` atual contém `skill_id`, `host_id`, ações, estado final esperado e digests de evidência. A própria classe documenta que construí-la não executa ação [4]. `propose_transfer()` também rejeita aliases e não envia a proposta ao executor. Essa separação deve ser preservada.

O limite é posterior à proposta. O `HostAdapter` expõe apenas `resources()`, `safe_action_space()`, `observe()`, `execute()` e `reset()`. Não há envelope de autoridade, nonce, TTL, revogação, recibo de commit ou canal de recuperação [4]. Além disso, `discover()` usa `host.execute()` para sondagem pública e chama `reset()` depois de cada probe. A campanha deve continuar distinguindo `probe_execute_calls` de chamadas ao executor autorizado; `reset()` não é recuperação de uma ação externa.

O `InstrumentedExecutor` da Etapa 4 já demonstra duas propriedades úteis: uma chamada sem autoridade é rejeitada e uma chamada autorizada pode registrar efeito parcial [2]. Ele ainda não representa reserva de nonce, expiração entre etapas, revogação, cadeia de recibos ou a diferença entre falha explícita e resultado desconhecido. Essas capacidades pertencem ao executor de teste da próxima campanha, não ao runtime de proposta.

## 3. Envelope de autoridade externa

### 3.1 Estrutura canônica

O envelope deve ser um objeto canônico versionado, assinado ou atestado por uma autoridade externa. SHA-256 pode comprometer o objeto, mas não substitui autenticidade ou independência. O runtime não deve criar a assinatura nem preencher o envelope com um digest derivado de seu próprio `WorldModel`.

A forma mínima recomendada é:

```text
ExecutionEnvelope {
    schema: "herus-execution-envelope-v1",
    envelope_id: string,
    authority_id: string,
    authority_key_id: string,
    authority_epoch: integer,
    herus_id: string,
    host_id: string,
    host_context_digest: string,
    skill_id: string,
    proposal_digest: string,
    action_sequence_digest: string,
    operation_id: string,
    attempt_no: integer,
    nonce: string,
    parent_ledger_digest: string | null,
    not_before: timestamp,
    expires_at: timestamp,
    clock_id: string,
    limits: {
        max_steps: integer,
        max_cost: integer,
        cost_unit: string
    },
    revocation_snapshot_digest: string,
    recovery_policy: "NO_AUTOMATIC_RETRY",
    authority_signature: bytes
}
```

`proposal_digest` deve comprometer a proposta inteira, incluindo a ordem das ações, `host_id`, estado esperado e digests de evidência. `action_sequence_digest` deve comprometer cada ação canônica na ordem em que o executor pode recebê-la. `host_context_digest` deve vincular o envelope à época e ao contexto do host; um `host_id` reutilizado não é confiança.

`operation_id` identifica a operação lógica e pode permanecer durante a reconciliação. `attempt_no` começa em zero e cresce estritamente. Cada tentativa recebe um `nonce` novo, aleatório e não reutilizável; recomenda-se no mínimo 128 bits de entropia. O `parent_ledger_digest` encadeia uma tentativa posterior ao ledger e ao resultado de recuperação que a autorizou. Um retry não reutiliza o envelope, o nonce, o frame ou o contador de tentativa anteriores.

`not_before` e `expires_at` formam um TTL absoluto em um relógio monotônico identificado por `clock_id`. O executor rejeita `expires_at <= not_before`, TTL acima do limite da política, relógio desconhecido e envelope usado fora da janela. A validade deve ser testada antes da reserva e entre cada passo. Expiração durante uma ação em voo não prova que a ação foi desfeita; se o recibo não vier, o resultado é `UNKNOWN_OUTCOME`.

`authority_epoch` e `revocation_snapshot_digest` vinculam o envelope a uma versão da autoridade e a uma visão de revogação. O adaptador deve verificar a autoridade, a assinatura, o epoch mínimo vigente e a ausência de revogação do `envelope_id`, `operation_id` ou host. Uma revogação não faz rollback. Revogação antes do início rejeita a execução; revogação entre passos impede o próximo passo e leva a `FAILED_PARTIAL` ou `UNKNOWN_OUTCOME`, conforme a qualidade do recibo.

O campo `recovery_policy` é deliberadamente fixo em `NO_AUTOMATIC_RETRY`. Mesmo que uma política futura declare uma cota de tentativas, ela não deve autorizar um loop interno. Uma nova tentativa exige novo envelope, novo nonce, novo TTL e uma solicitação externa posterior à recuperação.

### 3.2 Ordem de validação

O executor deve validar na ordem abaixo e registrar a decisão sem executar uma ação quando qualquer gate falhar:

1. Parsear o schema e canonicalizar exatamente o envelope.
2. Verificar assinatura, `authority_id`, `authority_key_id` e `authority_epoch` contra uma raiz externa conhecida.
3. Conferir `herus_id`, `host_id`, `host_context_digest`, `skill_id`, `proposal_digest` e `action_sequence_digest` contra a proposta fornecida.
4. Verificar `not_before`, `expires_at`, `clock_id` e o TTL no relógio monotônico do executor.
5. Consultar a visão de revogação e rejeitar snapshot ausente, inválido, obsoleto ou revogado.
6. Verificar que `nonce` ainda não foi reservado e reservá-lo atomicamente antes de qualquer efeito.
7. Verificar o `parent_ledger_digest`, o número de tentativa, o estado de recuperação e os limites residuais de passos e custo.
8. Emitir recibo de aceitação e somente então despachar o primeiro passo.

A ausência de envelope deve produzir `MISSING_AUTHORITY` e zero chamadas ao executor. Uma `TransferProposal` ou o retorno booleano de `transfer()` nunca pode ser aceito como envelope. Uma autoridade emitida pelo próprio runtime, ainda que tenha um SHA-256 correto, deve ser rejeitada como atestação não independente.

## 4. Ledger, estados e commit parcial

### 4.1 Estados separados da proposta

`proposal_status` continua descrevendo a construção do plano e usa os estados já congelados (`PROPOSED`, `ABSTAIN` e `UNSUPPORTED_BY_CONTRACT`). A execução deve usar um namespace separado, para não transformar uma proposta em sucesso nem corromper os quinze registros da Etapa 4.

```text
ExecutionStatus =
    REJECTED
    AUTHORIZED
    IN_FLIGHT
    COMMITTED
    FAILED_PARTIAL
    UNKNOWN_OUTCOME
    RECOVERY_REQUIRED
    RECOVERY_RESOLVED
```

`RECOVERY_REQUIRED` é uma trava operacional derivada de falha parcial, resultado desconhecido ou conflito de recibos. O ledger também conserva o estado causal (`FAILED_PARTIAL` ou `UNKNOWN_OUTCOME`); a trava não deve apagar essa distinção. `RECOVERY_RESOLVED` só é permitido quando uma atestação de recuperação independente torna conhecido o resultado e não existem eventos em voo.

A campanha deve adicionar campos, sem remover os existentes de `ResultRecord`:

```text
execution_status
recovery_status
operation_id
attempt_no
nonce_digest
envelope_digest
committed_steps
known_steps
retry_count
recovery_count
authorized_calls
committed_calls
unknown_event_count
ledger_digest
```

`committed_steps` só pode ser numérico para commits cobertos por recibos válidos. Se a quantidade de efeito for desconhecida, ela deve ser `null` ou representada por uma faixa declarada; nunca deve virar zero. `expected_final` da proposta é uma previsão do plano e não é recibo de commit.

### 4.2 Recibos e transições

Cada evento deve ser append-only e encadeado ao digest do evento anterior. Um recibo mínimo deve conter `operation_id`, `attempt_no`, `nonce`, `step_index`, evento, contador de commits conhecido, custo medido ou desconhecido, `host_id`, epoch de autoridade, relógio, `previous_receipt_digest` e uma atestação identificável da fonte.

As transições normativas são:

| Evento observável | Estado resultante | Regra de segurança |
|---|---|---|
| Envelope inválido, ausente, expirado ou revogado antes da reserva | `REJECTED` | Zero passo externo; nonce não é reutilizado se já foi reservado. |
| Envelope aceito e nonce reservado | `AUTHORIZED` | Ainda não é commit; qualquer crash posterior exige reconciliação. |
| Passo despachado sem recibo terminal | `IN_FLIGHT` → `UNKNOWN_OUTCOME` | Não assumir ausência de efeito. Ativar `RECOVERY_REQUIRED`. |
| Todos os passos têm recibo `COMMITTED` e limites válidos | `COMMITTED` | Terminal; nenhum retry. |
| Um ou mais passos comprometidos e falha explícita posterior | `FAILED_PARTIAL` | Não fazer rollback implícito; ativar `RECOVERY_REQUIRED`. |
| Recibos conflitantes, epoch revogado ou custo final não determinável | `UNKNOWN_OUTCOME` | Não escolher por maioria; ativar `RECOVERY_REQUIRED`. |
| Recuperação independente torna o resultado conhecido | `RECOVERY_RESOLVED` | Pode encerrar como committed ou permitir nova autorização explícita. |
| Nova tentativa sem recuperação, mesmo com nonce novo | `RECOVERY_REQUIRED` | `RETRY_BLOCKED`; nenhuma chamada externa. |

Um `FAILED_PARTIAL` exige um recibo explícito de falha após pelo menos um commit conhecido. Um timeout, ACK ausente, queda entre `DISPATCHED` e `COMMITTED` ou divergência entre recibos é `UNKNOWN_OUTCOME`, mesmo que um prefixo anterior tenha sido confirmado. Essa distinção evita afirmar que o último passo falhou apenas porque não houve resposta.

O executor não deve emitir `COMMITTED` apenas porque o host respondeu, porque `expected_final` coincide com a previsão, ou porque o número de chamadas foi atingido. O commit requer uma cadeia válida de recibos para todos os passos, sem overrun, revogação ou evento desconhecido. Se o host oferecer transação atômica, ainda assim o resultado deve ser comprovado por recibo; a política não pode ser inferida pelo runtime.

### 4.3 Recuperação

Recuperação é uma operação externa de reconciliação, não `reset()` do `HostAdapter` e não uma nova sondagem automática. Ela deve consultar um canal de estado do host ou uma autoridade de recuperação independente e produzir um `RecoveryAttestation` com `operation_id`, tentativa afetada, nonce, ledger digest, estado observado, passos comprometidos, fonte, época, timestamp e atestação própria.

A recuperação pode produzir somente uma das quatro conclusões:

* `RESOLVED_COMMITTED`: todos os passos estão comprovadamente comprometidos; a operação termina sem retry.
* `RESOLVED_NOT_COMMITTED`: a autoridade de recuperação prova que nenhum efeito da tentativa ocorreu; uma nova autorização pode ser emitida, mas não pelo executor automaticamente.
* `RESOLVED_PARTIAL`: um prefixo é conhecido como comprometido e o restante permanece pendente; a autoridade deve autorizar uma proposta residual explícita, com novo envelope e novo nonce.
* `RECOVERY_CONFLICT`: as fontes divergem ou não permitem uma conclusão; o ledger permanece em `RECOVERY_REQUIRED` e bloqueado.

Uma recuperação que apenas chama o mesmo executor e repete a mesma consulta sem identidade de fonte não é independente. Ela pode ser usada como telemetria, mas não deve limpar a trava sozinha quando o resultado for disputado. Nenhum caminho de recuperação deve fabricar rollback. Uma ação compensatória, se existir, deve ser uma nova proposta sujeita aos mesmos gates.

## 5. Bloqueio de retry automático

O retry deve ser bloqueado em quatro níveis: runtime, adaptador executor, host e campanha. Nenhum desses componentes pode chamar novamente uma ação após `FAILED_PARTIAL`, `UNKNOWN_OUTCOME`, expiração em voo ou revogação sem uma recuperação válida e uma solicitação externa nova.

A repetição do mesmo envelope deve falhar por `NONCE_REPLAY`. Um envelope novo para o mesmo `operation_id` deve falhar por `RETRY_BLOCKED` se não houver `RecoveryAttestation` compatível. Depois de recuperação, a autoridade externa pode emitir uma tentativa nova com `attempt_no` maior, nonce novo, TTL novo, `parent_ledger_digest` igual ao ledger resolvido e uma sequência residual explicitamente delimitada. O contador de custo, passos, retries e recuperação é monotônico; `bind()`, `reset()`, reboot ou nova sessão não o zera.

A ausência de ACK nunca é tratada como `FAILED_NO_EFFECT`. Se a chamada ainda estava em voo, a única saída segura é `UNKNOWN_OUTCOME`. Uma eventual confirmação tardia deve ser anexada ao ledger e reconciliada; não pode reabrir uma operação expirada, revogada ou encerrada nem conceder autoridade retroativa.

## 6. Migração compatível com a API black-box

### Fase A — contrato aditivo e linha de base

Adicionar, em módulo separado, os conceitos `ExecutionEnvelope`, `ExecutionReceipt`, `ExecutionLedger`, `RecoveryAttestation` e um `ExternalExecutorAdapter`. Nenhum método atual de `HostAdapter` deve se tornar obrigatório. Hosts antigos continuam sendo aceitos para descoberta, mas não recebem execução autorizada sem envelope verificável.

`TransferProposal` permanece um plano imutável. `transfer()` continua retornando `bool` e `propose_transfer()` continua retornando `TransferProposal | None`; ambos são marcados como caminhos de compatibilidade e não como prova de autorização. O executor novo recebe a proposta por um canal separado. O teste de regressão deve comprovar que a construção de proposta conserva `proposal_execute_calls == 0`, `authorized_calls == 0` e `external_effect_count == 0`.

A validação de `ResultRecord` da Etapa 4 continua aceitando seus campos e invariantes. Os campos novos são aditivos em um schema `herus-stage5-execution-v1`. O arquivo v1 da Etapa 4 não deve ser reescrito.

### Fase B — decisão tipada e executor de teste

Adicionar `propose_transfer_checked()` como operação opt-in. Ela retorna uma `TransferDecision` contendo a proposta opcional, `proposal_status`, qualidade da observação, `reason`, digests de proposta e observação, `safety_claim` e `proposal_execute_calls`. Ela não retorna envelope nem chama executor.

Adicionar ao harness uma implementação de `ExternalExecutorAdapter` que aceite somente uma proposta e um envelope externo. O antigo `InstrumentedExecutor.execute(authorized=False|True, partial=False)` fica preservado para a regressão Stage 4; o executor novo acrescenta nonce, TTL, revogação, recibos e ledger. Fixtures sem autoridade devem parar antes de qualquer efeito.

### Fase C — adoção estrita

Depois de auditar chamadores, a integração pode exigir `propose_transfer_checked()` para qualquer fluxo que pretenda executar externamente. O wrapper legado ainda pode existir por compatibilidade, mas não pode produzir `SUPPORTED`, não pode fabricar autoridade e deve emitir telemetria de uso/depreciação. A adoção estrita deve ser uma decisão do chamador ou da configuração externa, nunca uma interpretação silenciosa do runtime.

A migração não deve alterar o significado de `SUPPORTED_CONTROL`: ele é um veredito do eixo de proposta/observabilidade e continua separado do envelope. Mesmo quando `safety_claim` for `SUPPORTED`, não há execução até que a autoridade externa e todos os gates do executor passem.

## 7. Estados e motivos

Os motivos existentes devem ser preservados quando já expressam o caso. Os motivos novos abaixo pertencem ao schema de execução e não devem substituir retroativamente as razões dos registros congelados.

| Estado ou motivo | Condição | Ação obrigatória |
|---|---|---|
| `MISSING_AUTHORITY` | Não há envelope externo verificável. | `REJECTED`; nenhum passo. |
| `AUTHORITY_REJECTED` | Assinatura, chave, sujeito ou escopo não confere. | `REJECTED`; nenhum passo. |
| `PROPOSAL_DIGEST_MISMATCH` | Envelope não corresponde à proposta ou à ordem de ações. | `REJECTED`; nenhum passo. |
| `NONCE_REPLAY` | Nonce já reservado ou consumido. | `REJECTED`; nunca repetir. |
| `ENVELOPE_EXPIRED` | TTL inválido, ainda não iniciado ou expirado. | Rejeitar antes do início; em voo, `UNKNOWN_OUTCOME`. |
| `AUTHORITY_REVOKED` | Envelope, host, epoch ou operação revogado. | Bloquear antes do passo seguinte; reconciliar se em voo. |
| `PRECONDITION_MISMATCH` | Contexto/observação vinculada ao envelope mudou. | `REJECTED`; nova proposta e nova autoridade se necessário. |
| `PARTIAL_FAILURE` | Falha explícita depois de ao menos um commit. | `FAILED_PARTIAL` + `RECOVERY_REQUIRED`. |
| `UNKNOWN_OUTCOME` | ACK ausente, timeout, crash ou recibos conflitantes. | `UNKNOWN_OUTCOME` + `RECOVERY_REQUIRED`; sem retry. |
| `RECOVERY_REQUIRED` | Trava após falha parcial ou resultado desconhecido. | Exigir reconciliação externa; não limpar com `reset()`. |
| `RECOVERY_UNAVAILABLE` | Não há canal de reconciliação independente. | Permanecer bloqueado. |
| `RETRY_BLOCKED` | Tentativa nova sem recuperação e nova autoridade. | Zero chamada externa. |
| `REVOCATION_SNAPSHOT_STALE` | A visão de revogação não é atual. | Falhar fechado antes da reserva. |
| `COST_OVERRUN` | Recibo excede limite reservado. | Não alegar sucesso; se houve efeito, recuperar. |
| `UNKNOWN_COST` | Custo ausente, incompatível ou não medido. | Não converter em zero; bloquear o gate aplicável. |
| `SUPPORTED_CONTROL` | Controles de proposta e atestação previstos passam. | Não concede autoridade por si só. |

O runtime não deve usar `HIDDEN_EFFECT_UNVERIFIABLE` quando essa causa só existe na tabela privada do oráculo. Para uma causa que ele pode sustentar, deve usar a razão pública correspondente, como `UNSUPPORTED_BY_CONTRACT` ou `OBSERVABILITY_CLOSURE_UNPROVEN`, conforme o contrato de observabilidade [5].

## 8. Suíte de testes proposta

A campanha deve executar a linha de base e as novas propriedades em três repetições, com IDs, ações, chaves e custos permutados. O oráculo deve permanecer declarativo e independente. Ele não pode importar nem chamar `Effect.from_states()`, `_apply()`, `WorldModel` ou `Goal.satisfied()` [1] [2].

| ID | Teste | Aceitação |
|---|---|---|
| `BASE-01` | Regressão Stage 4 congelada | Os 15 registros, nove hard failures negativos e zero violações de schema continuam iguais em semântica; nenhum arquivo v1 é reescrito. |
| `SEP-01` | Proposta sem execução | `propose_transfer()` e `propose_transfer_checked()` mantêm `proposal_execute_calls == 0`, `authorized_calls == 0`, `committed_calls == 0`, `step_count == 0` e nenhum envelope emitido pelo runtime. |
| `AUTH-01` | Ausência de autoridade | Proposta sem envelope retorna `MISSING_AUTHORITY`, zero passos e zero efeito externo. |
| `AUTH-02` | Escopo adulterado | Alterar host, Skill, ação, ordem ou `proposal_digest` rejeita antes da reserva. |
| `AUTH-03` | Assinatura/atestação não independente | Digest correto sem autoridade externa válida não autoriza. |
| `NONCE-01` | Reserva antes do efeito | Falha entre reserva e despacho deixa o nonce consumido e exige recuperação; a ação não é repetida com o mesmo nonce. |
| `NONCE-02` | Replay | Reenviar envelope depois de `COMMITTED`, `FAILED_PARTIAL`, `UNKNOWN_OUTCOME` ou `REJECTED` produz `NONCE_REPLAY` e nenhuma segunda chamada. |
| `TTL-01` | Expiração antes do início | Envelope expirado ou com relógio desconhecido é rejeitado sem efeito. |
| `TTL-02` | Expiração entre passos | O executor não inicia novo passo; se o anterior estiver sem recibo, o estado é `UNKNOWN_OUTCOME` e fica bloqueado. |
| `REVOKE-01` | Revogação antes do início | Snapshot revogado ou epoch abaixo do piso rejeita sem reservar efeito. |
| `REVOKE-02` | Revogação em voo | Nenhum passo seguinte é iniciado; o resultado é `FAILED_PARTIAL` ou `UNKNOWN_OUTCOME` conforme o recibo, sempre com recuperação. |
| `COMMIT-01` | Falha parcial explícita | Em uma sequência de dois passos, primeiro commit e segundo falha produzem `FAILED_PARTIAL`, `committed_steps == 1`, nenhum rollback e zero retry automático. |
| `UNKNOWN-01` | ACK ausente/timeout | O passo despachado sem recibo terminal produz `UNKNOWN_OUTCOME`, sem converter `committed_steps` desconhecido em zero. |
| `UNKNOWN-02` | Recibos conflitantes | Divergência entre host, executor e ledger não é resolvida por maioria; permanece `RECOVERY_REQUIRED`. |
| `REC-01` | Recuperação para committed | Atestação independente de conclusão encerra a operação; não há retry. |
| `REC-02` | Recuperação para não comprometido | Somente solicitação externa com novo envelope, novo nonce, novo TTL e `attempt_no` maior pode iniciar nova tentativa. |
| `REC-03` | Recuperação parcial | Apenas a sequência residual explicitamente autorizada pode ser executada; o plano original inteiro não é repetido. |
| `REC-04` | Recuperação em conflito/ausente | A trava permanece; `reset()`, reboot ou nova sessão não limpam o ledger. |
| `RETRY-01` | Retry automático | Após `FAILED_PARTIAL`, `UNKNOWN_OUTCOME`, expiração ou revogação, não há segunda chamada sem solicitação externa. |
| `RETRY-02` | Novo envelope sem recuperação | Mesmo nonce novo não basta; resultado `RETRY_BLOCKED` e zero efeito. |
| `RETRY-03` | Retry autorizado externamente | Com recuperação válida, nova cadeia de nonce e escopo residual, exatamente uma nova tentativa é observada e cobrada. |
| `MONO-01` | Monotonicidade | `attempt_no`, passos, custo, retries, recoveries, reservas e sequência do ledger nunca diminuem em bind, reset, reboot ou recuperação. |
| `BUDGET-01` | Reserva de limites | Passos e custo são reservados antes da chamada; overrun impede o próximo passo e não é convertido em sucesso. |
| `ORACLE-01` | Independência | O oráculo não acessa símbolos ou estado privado do runtime e separa `oracle_reason` de `runtime_reason`. |
| `VAR-01` | Generalização estrutural | Renomear fixtures, ações, campos, unidades e ordem mantém as mesmas propriedades sem lógica especial por ID. |

Testes gerados devem executar prefixos arbitrários de eventos `ACCEPT`, `DISPATCH`, `COMMIT`, `FAIL`, `TIMEOUT`, `REVOKE`, `EXPIRE`, `RECOVER` e `RETRY`. Para todo prefixo, contadores são não negativos e monotônicos; nenhuma transição transforma `UNKNOWN_OUTCOME` em sucesso sem recibo e recuperação; nenhuma proposta possui chamada autorizada.

## 9. Plano de mutação

A campanha de mutação deve operar em uma cópia temporária do módulo ou por transformação AST controlada. O checkout principal não deve ser modificado durante a execução.

| ID | Mutação | Testes que devem matar |
|---|---|---|
| `M-AUTH-01` | Aceitar `TransferProposal` como autorização. | `SEP-01`, `AUTH-01`. |
| `M-AUTH-02` | Emitir envelope assinado pelo próprio runtime. | `AUTH-03`, `ORACLE-01`. |
| `M-AUTH-03` | Ignorar mismatch de host, ação ou proposta. | `AUTH-02`. |
| `M-NONCE-01` | Reservar nonce depois do despacho. | `NONCE-01`, `NONCE-02`. |
| `M-NONCE-02` | Permitir reuso do nonce após falha parcial. | `NONCE-02`, `RETRY-01`. |
| `M-TTL-01` | Validar TTL somente no início. | `TTL-02`, `REVOKE-02`. |
| `M-REV-01` | Consultar revogação somente no início da operação. | `REVOKE-02`. |
| `M-COMMIT-01` | Marcar operação completa após o primeiro commit. | `COMMIT-01`, `UNKNOWN-01`. |
| `M-COMMIT-02` | Inferir commit a partir de `expected_final` ou contagem de chamadas. | `COMMIT-01`, `UNKNOWN-01`. |
| `M-UNKNOWN-01` | Converter timeout/ACK ausente em `FAILED_PARTIAL` ou `COMMITTED`. | `UNKNOWN-01`, `UNKNOWN-02`. |
| `M-REC-01` | Limpar ledger com `reset()`, bind ou reboot. | `REC-04`, `MONO-01`. |
| `M-REC-02` | Aceitar recuperação emitida pelo mesmo caminho sem nova atestação. | `REC-01`, `REC-04`, `ORACLE-01`. |
| `M-RETRY-01` | Fazer retry automático após `FAILED_PARTIAL`. | `RETRY-01`, `COMMIT-01`. |
| `M-RETRY-02` | Fazer retry após `UNKNOWN_OUTCOME` sem recuperação. | `UNKNOWN-01`, `RETRY-02`. |
| `M-RETRY-03` | Aceitar novo nonce sem `RecoveryAttestation`. | `RETRY-02`. |
| `M-RETRY-04` | Repetir o plano inteiro após recuperação parcial. | `REC-03`. |
| `M-SEP-01` | Chamar executor dentro de `propose_transfer_checked()`. | `SEP-01`. |
| `M-COST-01` | Converter custo desconhecido em zero. | `BUDGET-01`, `UNKNOWN-01`. |
| `M-FIX-01` | Reconhecer diretamente `m7a-partial-01` ou outros IDs do holdout. | `VAR-01` com fixtures renomeadas. |
| `M-ORACLE-01` | Copiar `oracle_reason` para `runtime_reason`. | `ORACLE-01` e caso de efeito oculto. |

Um mutante sobrevivente só pode ser classificado como equivalente depois de demonstrar equivalência observacional em ações, recibos, estados, contadores e digests. O mesmo estado final não basta para considerar equivalente um mutante que repetiu um efeito.

## 10. Riscos e circularidade

**Autoridade circular.** Se o runtime, o mesmo processo do host e o oráculo compartilham a chave ou a lógica de emissão, o envelope é apenas uma confirmação de si próprio. O controle é exigir `authority_id`, `key_id`, proveniência e processo emissor independentes. Um digest demonstra integridade do envelope, não independência nem verdade.

**Nonce perdido após reinício.** Um registro volátil pode esquecer que um nonce foi reservado e aceitar replay. A reserva deve ser durável ou encadeada a um piso monotônico de autoridade. `bind()`, `reset()` e recuperação não podem zerar o ledger. Se a durabilidade não for demonstrada, a execução deve permanecer bloqueada.

**Relógios incompatíveis.** TTL baseado em relógio de parede pode aceitar envelope expirado ou rejeitar envelope válido. O contrato deve declarar `clock_id`, usar relógio monotônico com tolerância limitada e falhar fechado quando a comparação não for possível. TTL não transforma um timeout em prova de ausência de efeito.

**Revogação tardia.** Uma revogação pode chegar depois de um passo em voo. Ela pode impedir o próximo passo, mas não desfaz o passo anterior. O resultado deve manter `FAILED_PARTIAL` ou `UNKNOWN_OUTCOME` e exigir recuperação.

**Commit confundido com resposta.** ACK, mudança pública e `expected_final` podem ser incompletos. O controle é a cadeia de recibos por passo. A ausência de recibo não é recibo de zero; a mudança pública não prova ausência de efeito oculto.

**Recuperação circular.** O mesmo executor que perdeu o ACK pode declarar que nada ocorreu. O controle é um canal de reconciliação independente, ou pelo menos uma atestação com identidade de fonte e consulta de estado separada. Divergência permanece `RECOVERY_CONFLICT`.

**Idempotência fictícia.** Reutilizar `operation_id` não torna uma ação idempotente. Ações não idempotentes exigem prova de estado antes de nova tentativa. Um nonce novo evita replay do envelope, mas não evita sozinho duplicação sem recuperação.

**Confusão entre probe e execução.** `discover()` chama `host.execute()` para obter evidência pública. O novo trace deve registrar `probe_execute_calls` separadamente de `authorized_calls`. O gate estrito deve bloquear hosts que não declarem se a sonda é não mutante quando o risco da sonda não puder ser limitado.

**Compatibilidade permissiva.** Um consumidor antigo pode interpretar `TransferProposal` ou `True` como comando. A documentação, o tipo `TransferDecision`, a ausência de campos de autoridade na proposta e os testes de não execução devem tornar essa interpretação observavelmente inválida. O wrapper legado deve ser depreciado, não reinterpretado silenciosamente.

**Sobreajuste ao holdout.** Reconhecer `m7a-partial-01`, `PARTIAL_FAILURE` ou uma constante de custo não corrige a classe do problema. Fixtures renomeadas devem preservar a estrutura causal com novas ações, campos, ordens e durações.

**Interpretação excessiva.** Mesmo que todos os gates passem, o resultado é limitado a mecanismo host-only. O contrato congelado de utilidade exige dimensões humanas, baseline, privacidade, acessibilidade e reprodutibilidade que esta frente não mede [6]. Nenhuma execução autorizada deve ser apresentada como utilidade social.

## 11. Critério de aceite

A frente passa somente se a linha de base Stage 4 continuar reproduzível; proposta e execução permanecerem separadas; nenhum envelope puder ser emitido pelo runtime; nonce, TTL e revogação forem verificados antes de cada passo; commits parciais forem registrados sem rollback fictício; timeout e ACK ausente produzirem `UNKNOWN_OUTCOME`; falha explícita após commit produzir `FAILED_PARTIAL`; ambas as situações ativarem `RECOVERY_REQUIRED`; nenhuma rota fizer retry automático; toda nova tentativa exigir recuperação, nova autoridade, novo nonce, novo TTL e escopo residual; contadores e ledger forem monotônicos; o oráculo permanecer independente; e todos os mutantes não equivalentes forem mortos por testes focados e fixtures renomeadas.

O veredito final da campanha deve ser `mechanism_only` apenas se os gates mecânicos passarem e as dimensões humanas continuarem explicitamente ausentes. Qualquer hard limit violado, qualquer retry automático, qualquer execução sem autoridade ou qualquer ambiguidade de resultado não resolvida deve manter a classificação `not_proven`.

## Referências

[1]: ../../../docs/53-BENCHMARK-HOLDOUT-ADVERSARIAL.md "Etapa 4 — benchmark holdout adversarial"  
[2]: ../../holdout_adversarial.py "Campanha holdout adversarial"  
[3]: ../holdout_benchmark_v1.json "Evidência congelada do benchmark holdout v1"  
[4]: ../../symbiont_v2/core.py "HERUS Symbiont v2 research runtime"  
[5]: 01-observation_contract.md "Stage 5 — contrato tipado de observabilidade"  
[6]: ../../symbiosis_utility_contract.json "Contrato congelado de utilidade da simbiose HERUS"  
[7]: ../../holdout_hosts.py "Hosts públicos e fixtures adversariais"  
[8]: ../../stage4/contracts.py "Contratos de resultado e razões da Etapa 4"

<!-- Relatório produzido como proposta de pesquisa. Nenhuma implementação foi alterada. -->
