# HERUS — plano restante: executor externo e recuperação

**ID:** `HERUS-REM-EXT-EXEC-001`  
**Frente:** envelope de autoridade, nonce, TTL, revogação, ledger append-only, falha parcial, `UNKNOWN_OUTCOME`, recuperação e retry bloqueado.  
**Status:** desenho implementável; **nenhuma alteração de código foi feita nesta frente**.  
**Escopo:** mecanismo host-only e executor de teste separado. Este plano não concede autoridade ao runtime HERUS e não constitui plano de atuação física.

## 1. Decisão executiva

A proposta do HERUS deve continuar sendo um plano imutável e não autorizante. A execução que pode produzir efeito externo deve ocorrer em um processo ou adaptador separado, depois de uma autoridade externa emitir um envelope verificável. O runtime pode produzir `TransferProposal` ou `TransferDecision`; não pode assinar, emitir, reservar nonce, consultar a chave da autoridade, escolher retry, limpar recuperação ou chamar o executor. Essa separação é a continuação necessária da fronteira já estabelecida entre proposta e execução. [1] [2]

O protocolo recomendado tem quatro objetos independentes:

1. **Proposta:** digest canônico do plano, objetivo, ordem de ações e evidências públicas. Não contém autorização.
2. **Envelope de execução:** autorização externa, limitada ao digest exato da proposta, ao host, à época, ao escopo, ao custo e a uma tentativa única.
3. **Ledger de execução:** registro durável, append-only e encadeado de validações, reservas, commits, falhas, revogações, recibos e recuperação.
4. **Atestação de recuperação:** conclusão emitida por uma fonte independente sobre o que ocorreu quando a execução terminou parcialmente, expirou ou perdeu o ACK.

Os estados de proposta e execução não devem ser colapsados. `PROPOSED` nunca significa `AUTHORIZED`; `SUPPORTED` é uma alegação de contrato/observabilidade e não uma permissão para atuar. Na execução, `FAILED_PARTIAL` e `UNKNOWN_OUTCOME` são estados causais distintos, mas ambos ativam `RECOVERY_REQUIRED`. O resultado final da frente só pode ser `mechanism_only` quando todos os gates mecânicos passarem; qualquer hard limit violado mantém `not_proven`. [3] [4]

## 2. Arquitetura e fronteira de autoridade

O fluxo implementável é:

```text
runtime HERUS
  └─→ TransferProposal / TransferDecision
        └─→ canal serializado de revisão
              └─→ autoridade externa independente
                    └─→ ExecutionEnvelope assinado/atestado
                          └─→ ExternalExecutorAdapter
                                ├─→ valida envelope, TTL, revogação e nonce
                                ├─→ registra ledger antes do primeiro efeito
                                ├─→ executa passos e fecha recibos
                                └─→ em falha: UNKNOWN_OUTCOME ou FAILED_PARTIAL
                                      └─→ recuperação externa
```

O executor não deve receber objetos Python, closures, `__dict__`, tabelas privadas de transição ou callbacks do runtime. O protocolo deve transportar apenas dados canônicos serializados. O oráculo permanece fora do caminho de decisão do runtime e avalia o traço depois de fechado; `oracle_reason` não pode ser copiado para `runtime_reason`. O precedente do firmware é aplicável: o auditor verifica condições já observadas, enquanto a autoridade de handoff permanece em outra camada; revogação domina um caminho anteriormente válido. [5]

A sonda usada pela descoberta é um canal diferente da execução autorizada. O código atual usa `host.execute()` para aprender efeitos públicos, portanto `probe_execute_calls` deve ser contabilizado separadamente de `authorized_calls`, `committed_calls` e `external_effect_count`. `reset()` usado após uma sonda não é recuperação e não prova que um efeito externo foi desfeito. Se o host não declarar que a sonda é não mutante, o modo estrito deve bloquear antes da sonda quando a decisão depender dessa propriedade. [2] [6]

## 3. Envelope de autoridade externa

### 3.1 Estrutura mínima

O novo envelope deve ser versionado e canonicalizado antes da assinatura ou atestação:

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
    not_before: logical_time,
    expires_at: logical_time,
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

`proposal_digest` deve cobrir a proposta inteira, inclusive objetivo, argumentos tipados, ordem, `host_id`, estado esperado e digests de evidência. `action_sequence_digest` deve cobrir cada ação na ordem despachável. `host_context_digest` deve vincular a época e o contexto do host; `host_id` isolado não é identidade suficiente. `herus_id` vincula a operação ao runtime, mas nunca se transforma em autoridade.

`operation_id` identifica a operação lógica. `attempt_no` começa em zero e cresce estritamente. Cada tentativa recebe nonce novo, não reutilizável, com pelo menos 128 bits de entropia. Um retry nunca reutiliza envelope, nonce, frame ou contador de tentativa anterior. `parent_ledger_digest` deve encadear uma tentativa posterior ao ledger resolvido que permitiu a solicitação externa.

A assinatura deve ser verificável por uma raiz ou keystore externo conhecido. Um SHA-256 correto de um envelope emitido pelo próprio runtime demonstra integridade dos bytes, não independência da autoridade. O executor deve rejeitar uma autoridade cuja identidade, chave, época ou proveniência não possam ser verificadas.

### 3.2 Ordem de validação

Antes de qualquer efeito, o adaptador deve executar e registrar esta ordem determinística:

1. Parsear o schema e canonicalizar exatamente o envelope.
2. Verificar assinatura, `authority_id`, `authority_key_id` e `authority_epoch` contra a raiz externa.
3. Comparar `herus_id`, `host_id`, `host_context_digest`, `skill_id`, `proposal_digest` e `action_sequence_digest` com a proposta fornecida.
4. Verificar `clock_id`, `not_before`, `expires_at` e TTL no relógio monotônico do executor.
5. Consultar o snapshot de revogação e rejeitar snapshot ausente, inválido, obsoleto ou revogado.
6. Verificar `parent_ledger_digest`, `attempt_no`, estado de recuperação e limites residuais.
7. Reservar o nonce atomicamente e de forma durável antes do primeiro efeito.
8. Emitir recibo de aceitação; só então despachar o primeiro passo.

A ausência de envelope é `REJECTED + MISSING_AUTHORITY`, com zero passos e zero efeitos. `TransferProposal`, `transfer() == True`, Skill verificada, confiança, nome de ação, posição na enumeração ou digest emitido pelo runtime nunca são substitutos do envelope.

## 4. Nonce, TTL e revogação

### 4.1 Nonce de uso único

A reserva do nonce precisa ser uma operação atômica no ledger durável. A chave lógica mínima é `(authority_id, operation_id, attempt_no, nonce)`, com vínculo ao digest do envelope. Depois da reserva, o nonce fica consumido mesmo que o executor falhe entre a reserva e o despacho. Se o mesmo nonce reaparecer, o resultado é `REJECTED + NONCE_REPLAY`, sem segunda chamada externa.

Uma rejeição que ocorre antes da reserva, como schema inválido ou assinatura ausente, não deve produzir efeito. Entretanto, o envelope rejeitado não deve ser reinterpretado como uma nova autorização. Após a reserva, crash, timeout, falha parcial, revogação ou expiração, nenhum reset, reboot, `bind()` ou nova sessão pode apagar o fato de consumo.

### 4.2 TTL lógico

O TTL deve usar um relógio monotônico identificado por `clock_id`, preferencialmente injetável no harness para tornar os testes determinísticos. `expires_at <= not_before`, clock desconhecido e TTL acima do máximo da política são inválidos. A validade deve ser verificada antes da reserva, antes de cada passo e antes de cada commit que possa produzir efeito.

Expiração antes do início produz `REJECTED + ENVELOPE_EXPIRED`, sem efeito. Expiração enquanto há passo em voo não prova rollback ou ausência de efeito: se o recibo terminal não existir, o resultado é `UNKNOWN_OUTCOME + RECOVERY_REQUIRED`. TTL é uma janela de autorização, não uma prova de que o mundo voltou ao estado anterior.

### 4.3 Revogação

A autoridade deve publicar uma visão versionada de revogação. O envelope compromete `revocation_snapshot_digest`; o executor verifica a visão antes da reserva e novamente antes de cada passo. Deve bloquear se estiver revogado o `envelope_id`, `operation_id`, host, epoch ou autoridade correspondente, ou se o snapshot estiver stale.

Revogação antes do início produz `REJECTED + AUTHORITY_REVOKED`, sem efeito. Revogação entre passos impede o próximo passo. Se já houver commit conhecido, o estado é `FAILED_PARTIAL + RECOVERY_REQUIRED`; se o passo em voo não tiver recibo conclusivo, é `UNKNOWN_OUTCOME + RECOVERY_REQUIRED`. Revogação não faz rollback e não concede autoridade retroativa a uma ação tardia.

## 5. Ledger append-only e estados

### 5.1 Registro mínimo

O ledger deve ser durável, append-only, encadeado por digest e monotônico. Cada evento deve incluir, no mínimo:

```text
LedgerEvent {
    schema: "herus-execution-ledger-event-v1",
    event_seq: integer,
    previous_event_digest: string | null,
    operation_id: string,
    attempt_no: integer,
    envelope_digest: string,
    proposal_digest: string,
    authority_digest: string,
    nonce_digest: string,
    host_id: string,
    host_context_digest: string,
    event_type: string,
    logical_step: integer | null,
    execution_status: string,
    recovery_status: string,
    cost_reserved: integer | null,
    cost_actual: integer | null,
    external_effect_count: integer | null,
    source_id: string,
    event_digest: string
}
```

Tipos de evento necessários incluem `proposal_received`, `schema_checked`, `authority_checked`, `ttl_checked`, `revocation_checked`, `nonce_reserved`, `reservation_created`, `execution_requested`, `action_started`, `irreversible_commit`, `external_effect_emitted`, `cost_debited`, `ack_returned`, `cancel_requested`, `stop_applied`, `partial_failure`, `unknown_outcome`, `recovery_requested`, `recovery_attested`, `duplicate_rejected`, `stale_authority_rejected` e `retry_blocked`.

A persistência deve rejeitar snapshot velho ou `event_seq` regressivo. Contadores de passos, custo, reservas, retries e recuperações nunca diminuem. `reset()`, `bind()`, reboot e recuperação não zeram o ledger. Se a durabilidade do nonce ou da cadeia não puder ser demonstrada, o executor deve permanecer bloqueado, não degradar silenciosamente para memória volátil.

### 5.2 Estados de execução

```text
REJECTED
  └─→ AUTHORIZED
        └─→ IN_FLIGHT
              ├─→ COMMITTED
              ├─→ FAILED_PARTIAL ─→ RECOVERY_REQUIRED
              └─→ UNKNOWN_OUTCOME ─→ RECOVERY_REQUIRED

RECOVERY_REQUIRED ──(atestação independente)──→ RECOVERY_RESOLVED
```

`RECOVERY_REQUIRED` é uma trava operacional; preserva o estado causal original. Não deve substituir `FAILED_PARTIAL` ou `UNKNOWN_OUTCOME` por um sucesso genérico. `COMMITTED` exige recibos terminais válidos para todos os passos autorizados, sem overrun, revogação ou evento desconhecido. `expected_final`, número de chamadas ou ACK isolado não são recibos de commit.

A matriz normativa é:

| Evidência do traço | Estado | Regra obrigatória |
|---|---|---|
| Envelope ausente, inválido, expirado ou revogado antes da reserva | `REJECTED` | Zero passo e zero efeito. |
| Envelope aceito e nonce reservado | `AUTHORIZED` | Ainda não é commit; crash posterior exige reconciliação. |
| Passo despachado sem recibo terminal | `UNKNOWN_OUTCOME` + `RECOVERY_REQUIRED` | Não assumir ausência de efeito; retry bloqueado. |
| Prefixo comprometido e falha explícita posterior | `FAILED_PARTIAL` + `RECOVERY_REQUIRED` | Preservar `committed_steps`; não fazer rollback fictício. |
| Todos os passos cobertos por recibos válidos | `COMMITTED` | Encerrar; não repetir. |
| Recibos conflitantes, custo final desconhecido ou epoch revogado em voo | `UNKNOWN_OUTCOME` + `RECOVERY_REQUIRED` | Não decidir por maioria. |
| Nova tentativa sem recuperação válida | `RETRY_BLOCKED` | Zero chamada externa, mesmo com nonce novo. |

`committed_steps` somente pode ser numérico para commits cobertos por recibos válidos. Quando o passo em voo é desconhecido, deve ser `null` ou faixa explicitamente declarada; nunca zero. Um prefixo conhecido pode ser preservado em `known_steps`, sem converter a operação inteira em `FAILED_PARTIAL` quando o resultado do último passo é incerto.

## 6. Recuperação e retry bloqueado

Recuperação não é `reset()` do `HostAdapter`, não é nova sondagem automática e não é uma repetição da mesma consulta pelo executor que perdeu o ACK. Deve consultar uma fonte independente de estado ou autoridade de reconciliação e emitir uma `RecoveryAttestation` com `operation_id`, tentativa, nonce, digest do ledger, estado observado, passos comprometidos, fonte, epoch, relógio e atestação própria.

Conclusões permitidas:

- **`RESOLVED_COMMITTED`:** todos os passos estão comprovadamente comprometidos; encerra sem retry.
- **`RESOLVED_NOT_COMMITTED`:** a fonte prova que nenhum efeito da tentativa ocorreu; uma nova autorização externa pode ser solicitada.
- **`RESOLVED_PARTIAL`:** um prefixo é conhecido como comprometido; somente uma sequência residual explicitamente autorizada pode ser proposta.
- **`RECOVERY_CONFLICT`:** fontes divergem ou não permitem conclusão; permanece `RECOVERY_REQUIRED`.

Não deve haver retry automático após `FAILED_PARTIAL`, `UNKNOWN_OUTCOME`, expiração em voo ou revogação. Mesmo depois de `RESOLVED_NOT_COMMITTED`, o executor não deve se autoautorizar. Uma nova tentativa exige, externamente, `attempt_no` maior, envelope novo, nonce novo, TTL novo, digest-pai apontando para o ledger resolvido e escopo residual explícito. A nova autorização deve ser posterior à atestação de recuperação.

Um envelope novo sem recuperação válida produz `RETRY_BLOCKED`, ainda que o nonce seja novo. Ações não idempotentes não se tornam idempotentes por reutilizar `operation_id`; nonce novo impede replay do envelope, mas não prova que duplicação é segura.

## 7. Arquivos prováveis e responsabilidades

A lista abaixo é um mapa de implementação provável. Ela é deliberadamente aditiva e não representa alteração realizada nesta frente.

| Arquivo provável | Responsabilidade | Restrição de autoridade |
|---|---|---|
| `research/stage4/execution_contracts.py` | `ExecutionEnvelope`, `ExecutionReceipt`, `RecoveryAttestation`, enums de execução, canonicalização e validação de schema | Não importa o runtime para decidir validade; não assina envelope. |
| `research/stage4/ledger.py` | Eventos append-only, digest encadeado, reserva atômica de nonce, monotonicidade e snapshots | Não oferece método de “reset completo”; falha se a cadeia persistida regredir. |
| `research/stage4/executor.py` | `ExternalExecutorAdapter`, validação por passo, TTL, revogação, despacho, stop e fechamento de recibos | Processo/proxy separado; aceita somente envelope externo válido. |
| `research/stage4/recovery.py` | Protocolo de reconciliação e `RecoveryAttestation` | Fonte distinta do executor que perdeu ACK; não fabrica rollback. |
| `research/stage4/runner.py` | Orquestra runtime, autoridade, executor, oráculo e repetição da campanha | O runtime termina antes de o oráculo fechar o resultado; não recebe a chave da autoridade. |
| `research/stage4/results.schema.json` ou schema novo versionado | Campos de execução, ledger, recuperação e separação de razões | Não reescrever `holdout_benchmark_v1.json` nem quebrar o schema histórico. |
| `research/symbiont_v2/stage5.py` | Somente tipos aditivos compartilhados de decisão/orçamento, se necessários | Não deve emitir envelope, assinar, reservar nonce ou chamar executor. A implementação atual tem um snapshot de `BudgetLedger`, não um ledger externo completo. |
| `research/symbiont_v2/core.py` | Apenas preservar/ajustar a decisão tipada e a barreira proposta–execução | `propose_transfer_checked()` não recebe executor; `TransferProposal` continua sem autoridade. |
| `research/stage4/test_executor.py` | Testes de envelope, nonce, TTL, revogação, commits e estados desconhecidos | Deve observar efeitos pelo trace, não por booleano de retorno. |
| `research/stage4/test_recovery.py` | Recuperação independente, residual, conflito e bloqueio de retry | `reset()`, reboot e nova sessão não podem limpar o ledger. |
| `research/stage4/test_protocol.py` | Isolamento de processo, não vazamento e separação de probes | Verifica `proposal_execute_calls == 0`, `authorized_calls == 0` e `external_effect_count == 0`. |
| `research/stage4/test_stage4.py` | Fixtures C0/M6–M9, três repetições, mutações e IDs permutados | Não usa rótulos privados ou lógica especial para nomes das fixtures. |

A implementação deve preservar `transfer()` e `propose_transfer()` como wrappers históricos. O novo executor não deve ser passado como parâmetro ao runtime. O caminho estrito de proposta pode retornar `UNSUPPORTED_BY_CONTRACT` ou `ABSTAIN`, mas nunca um envelope. A autoridade é uma dependência do adaptador externo, não uma propriedade da Skill, do digest ou da identidade.

## 8. Critérios de aceitação

A frente só deve ser aceita quando todos os critérios abaixo passarem; uma violação de autoridade, retry ou resultado desconhecido é hard failure e não pode ser compensada por médias positivas.

1. A linha de base Stage 4 permanece reproduzível, com seus 15 registros sem reescrita semântica.
2. A construção de qualquer proposta mantém `proposal_execute_calls == 0`, `authorized_calls == 0`, `committed_calls == 0`, `step_count == 0` e `external_effect_count == 0`.
3. O runtime não possui chave de autoridade, não assina envelope, não reserva nonce e não escolhe política de retry ou recuperação.
4. Envelope ausente, assinatura inválida, escopo adulterado, host/epoch divergente, TTL inválido e snapshot de revogação stale são rejeitados antes do efeito.
5. Nonce é reservado atomicamente antes do primeiro efeito, fica consumido após reserva e rejeita replay em qualquer estado terminal ou de recuperação.
6. TTL e revogação são verificados antes de cada passo; revogação não faz rollback e bloqueia passos seguintes.
7. Ledger, `event_seq`, custo, passos, reservas, retries e recoveries são append-only e monotônicos em `bind`, `reset`, reboot e recuperação.
8. Falha explícita depois de commit produz `FAILED_PARTIAL + RECOVERY_REQUIRED`, preserva o prefixo comprometido e não inventa rollback.
9. Timeout, crash, ACK ausente ou recibos conflitantes produzem `UNKNOWN_OUTCOME + RECOVERY_REQUIRED`; `committed_steps` não é convertido em zero.
10. `reset()` isolado, nova sessão, novo nonce ou novo envelope sem atestação não limpa `RECOVERY_REQUIRED`; o resultado é `RETRY_BLOCKED` e zero chamada externa.
11. Retry autorizado somente após recuperação válida usa novo envelope, nonce, TTL, tentativa, digest-pai e escopo residual.
12. A recuperação emitida pelo mesmo caminho que perdeu o ACK não é aceita como independente sem atestação adicional.
13. O oráculo não importa nem usa `core.py`, `Effect.from_states()`, `_apply()`, `WorldModel` ou `Goal.satisfied()` para decidir a verdade privada.
14. IDs, ordem, ações, campos e unidades podem ser permutados sem mudar a propriedade testada e sem reconhecimento especial de `m7a`, `m8a` ou outros rótulos.
15. O resultado final é `mechanism_only` somente quando os gates mecânicos passam e as dimensões humanas continuam explicitamente ausentes; qualquer hard failure mantém `not_proven`.

## 9. Testes e mutações

A campanha deve rodar a linha de base e as novas fixtures em três repetições byte a byte, usando relógio monotônico injetável e eventos declarativos. O executor deve ser testado por traço e estado, nunca apenas por retorno booleano. A matriz mínima é:

| ID | Caso | Verificação |
|---|---|---|
| `BASE-01` | Regressão congelada | 15 registros Stage 4 preservados; nenhum artefato v1 reescrito. |
| `SEP-01` | Proposta sem execução | Nenhum envelope emitido pelo runtime; contadores autorizados e efeitos externos permanecem zero. |
| `AUTH-01` | Sem autoridade | `MISSING_AUTHORITY`, `REJECTED`, zero passos e zero efeitos. |
| `AUTH-02` | Escopo adulterado | Alterar host, ação, ordem, objetivo ou digest rejeita antes da reserva. |
| `AUTH-03` | Autoridade circular | Digest correto ou assinatura do runtime não autoriza. |
| `NONCE-01` | Reserva antes do efeito | Falha entre reserva e despacho consome nonce e exige recuperação. |
| `NONCE-02` | Replay | Reenvio depois de commit, rejeição ou falha retorna `NONCE_REPLAY`, sem segunda chamada. |
| `TTL-01` | Expiração pré-início | Envelope expirado ou clock desconhecido é rejeitado sem efeito. |
| `TTL-02` | Expiração entre passos | Nenhum passo seguinte inicia; passo em voo sem recibo vira `UNKNOWN_OUTCOME`. |
| `REVOKE-01` | Revogação pré-início | Snapshot revogado ou stale rejeita sem reservar efeito. |
| `REVOKE-02` | Revogação em voo | Bloqueia o próximo passo e ativa recuperação. |
| `COMMIT-01` | Falha explícita após commit | Primeiro passo comprometido e segundo falho produzem `FAILED_PARTIAL`, sem retry. |
| `UNKNOWN-01` | ACK ausente | `UNKNOWN_OUTCOME`, `RECOVERY_REQUIRED`, sem zero fabricado. |
| `UNKNOWN-02` | Recibos conflitantes | Não resolver por maioria; permanece bloqueado. |
| `REC-01` | Recuperação para committed | Encerra sem retry. |
| `REC-02` | Recuperação para não comprometido | Só nova autorização externa com cadeia nova inicia outra tentativa. |
| `REC-03` | Recuperação parcial | Somente sequência residual autorizada pode ser executada. |
| `REC-04` | Recuperação ausente/conflitante | Reset, reboot e bind não limpam a trava. |
| `RETRY-01` | Retry automático | Nenhuma segunda chamada após falha, timeout, expiração ou revogação. |
| `RETRY-02` | Novo nonce sem recuperação | `RETRY_BLOCKED`, zero efeito. |
| `RETRY-03` | Retry pós-recuperação | Exatamente uma tentativa nova, cobrada e encadeada. |
| `MONO-01` | Monotonicidade | Sequência, custos, passos, reservas e recuperações nunca diminuem. |
| `ORACLE-01` | Independência | Oráculo isolado; `runtime_reason` e `oracle_reason` não se misturam. |
| `VAR-01` | Permutação | Renomear fixture, ação, campo, ordem e unidade preserva o comportamento. |

Mutações obrigatórias e teste que deve eliminá-las:

| Mutação | Falha que deve ser detectada |
|---|---|
| Aceitar `TransferProposal` como autorização | `SEP-01`, `AUTH-01`. |
| Assinar envelope no runtime | `AUTH-03`, `ORACLE-01`. |
| Ignorar mismatch de host, ação ou proposta | `AUTH-02`. |
| Reservar nonce depois do despacho ou permitir replay | `NONCE-01`, `NONCE-02`. |
| Validar TTL ou revogação somente no início | `TTL-02`, `REVOKE-02`. |
| Marcar completo após o primeiro commit | `COMMIT-01`, `UNKNOWN-01`. |
| Inferir commit por `expected_final` ou contagem de chamadas | `COMMIT-01`, `UNKNOWN-01`. |
| Transformar timeout em `FAILED_PARTIAL` ou `COMMITTED` | `UNKNOWN-01`, `UNKNOWN-02`. |
| Limpar ledger em `reset`, bind ou reboot | `REC-04`, `MONO-01`. |
| Aceitar recuperação emitida pelo próprio executor | `REC-01`, `REC-04`, `ORACLE-01`. |
| Fazer retry automático ou aceitar novo nonce sem recuperação | `RETRY-01`, `RETRY-02`, `RETRY-03`. |
| Repetir o plano inteiro após recuperação parcial | `REC-03`. |
| Chamar executor dentro da proposta | `SEP-01`. |
| Tratar custo desconhecido como zero | `MONO-01` e teste de orçamento. |
| Reconhecer IDs do holdout ou copiar `oracle_reason` | `VAR-01`, `ORACLE-01`. |

Testes gerados devem explorar prefixos de eventos `ACCEPT`, `DISPATCH`, `COMMIT`, `FAIL`, `TIMEOUT`, `REVOKE`, `EXPIRE`, `RECOVER` e `RETRY`. Para todo prefixo, contadores devem ser não negativos e monotônicos; nenhum `UNKNOWN_OUTCOME` pode virar sucesso sem recibo e recuperação compatíveis.

## 10. Riscos de circularidade e controles

**Autoridade circular.** Se runtime, host e oráculo compartilham a chave ou o emissor, a autorização apenas confirma a própria decisão. Exigir `authority_id`, `authority_key_id`, processo emissor, raiz conhecida e proveniência distintos. Digest não é autenticidade.

**Circularidade do digest de fechamento.** Um digest calculado por `Effect.from_states()` ou pelo próprio runtime compromete uma projeção, não demonstra completude nem ausência de efeitos ocultos. A atestação deve vir de fixture/oráculo/processo independente, e sua proveniência deve ser auditável.

**Circularidade da recuperação.** O executor que perdeu o ACK não pode declarar sozinho que nada ocorreu. A reconciliação deve consultar canal ou fonte independente; divergência permanece `RECOVERY_CONFLICT`.

**Nonce volátil.** Se o registro de nonce for perdido em reboot, replay pode produzir duplicação. A reserva deve ser durável ou vinculada a um piso monotônico externo. Sem essa prova, bloquear execução.

**Relógios incompatíveis.** Relógios de parede ou `clock_id` desconhecido podem aceitar envelope expirado. Usar relógio monotônico injetável, janela explicitamente limitada e falha fechada quando a comparação não for possível.

**Commit confundido com resposta.** ACK, mudança pública, contagem de chamadas e `expected_final` podem ser incompletos. Só uma cadeia de recibos por passo prova commit; ausência de recibo não é zero efeito.

**Idempotência fictícia.** `operation_id` repetido não torna ação não idempotente segura. Novo nonce impede replay do envelope, mas não autoriza duplicação sem reconciliação e escopo residual.

**Confusão entre probe e execução.** Sondas de `discover()` podem ser mutantes, embora não sejam execução autorizada. Manter contadores e canais separados; o modo estrito deve bloquear sonda de risco desconhecido.

**Compatibilidade permissiva.** Consumidores antigos podem interpretar `True` ou uma proposta como comando. Manter tipos e semântica legados sem transformá-los em autorização, emitir telemetria de depreciação e exigir o adaptador externo para qualquer efeito.

**Sobreajuste ao holdout.** Reconhecer `m7a-partial-01`, `UNKNOWN_OUTCOME` ou nomes de campos não demonstra a propriedade. Usar fixtures renomeadas, topologias alteradas e mutações estruturais.

## 11. O que não pode ser alegado

Este desenho não permite alegar que o runtime tem autoridade física, que um digest prova verdade do host, que nonce prova idempotência, que TTL prova ausência de efeito, que revogação desfaz commits, que `reset()` recupera um mundo parcial ou que `expected_final` é recibo do mundo. Também não permite alegar que `SUPPORTED`, uma proposta construída ou um `COMMITTED` do executor de teste demonstram segurança física geral.

Mesmo uma campanha aprovada demonstra no máximo um **mecanismo host-only delimitado**, sob um protocolo e uma autoridade de teste específicos. Ela não demonstra simbiose útil, benefício humano, redução de erro em usuário real, privacidade de produto, acessibilidade, robustez robótica, segurança operacional geral, consciência ou inteligência geral. O contrato de utilidade exige tarefa humana identificável, baseline convencional, condição sem simbionte e medições próprias; nenhum desses elementos é fornecido por esta frente. [7]

Também não se pode alegar independência apenas porque há processos separados. A independência precisa incluir autoridade, chave, oráculo, tabela de transição, canal de recuperação e critérios sem compartilhamento circular. Se qualquer hard limit falhar, o resultado correto é `not_proven`, não uma média positiva nem uma alegação de segurança.

## Referências

[1]: ../../../docs/53-BENCHMARK-HOLDOUT-ADVERSARIAL.md "Etapa 4 — benchmark holdout adversarial"
[2]: ../stage5_plan/03-partial_failure_executor.md "Stage 5 — envelope externo, executor e falha parcial"
[3]: ../stage5_plan/00-synthesis.md "HERUS Etapa 5 — síntese arquitetural mínima"
[4]: ../stage5_plan/05-regression_and_mutations.md "Stage 5 — testes de regressão e mutações"
[5]: ../../../firmware/core/assurance.h "HERUS assurance — fronteira de handoff e autoridade"
[6]: ../stage5_plan/04-budget_cost_model.md "Stage 5 — modelo de custo e orçamento"
[7]: ../../../docs/52-DEFINICAO-SIMBIOSE-UTIL.md "Definição congelada de simbiose útil"
[8]: ../../symbiont_v2/core.py "HERUS Symbiont v2 research runtime"
[9]: ../../symbiont_v2/stage5.py "Tipos Stage 5 de decisão e ledger de orçamento"
[10]: ../stage4_plan/00-synthesis.md "Síntese de integração da etapa 4"

<!-- Relatório produzido como desenho implementável. Nenhum arquivo de código foi alterado nesta frente. -->
