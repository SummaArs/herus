# Auditoria adversarial de segurança do executor

**Escopo:** nonce, replay, TTL, relógio, revogação, assinatura e autoridade, ledger, falhas parciais, `UNKNOWN_OUTCOME`, recovery, rollback, orçamento, holdout e circularidade do oráculo. Foram lidos os documentos mestre e das etapas 3–5, `research/symbiont_v2`, `research/stage4`, `research/stage5.py`, holdouts e protocolo social. **Nenhum código foi editado.**

**Commit auditado:** `fbf7471c4903fd6af285f9e6e6a0e10f7cb2a967`.

## Veredicto

**REPROVADO para qualquer alegação de execução autorizada, segurança de execução, `SUPPORTED` ou recuperação confiável.** O repositório tem uma separação conceitual correta entre proposta e execução, e a documentação reconhece que nonce, TTL, revogação e recovery ainda seriam responsabilidades externas [1] [2]. Contudo, o adaptador entregue apresenta uma API com aparência de executor seguro sem implementar as propriedades que seus nomes sugerem. Ele aceita envelopes não assinados e não vinculados a uma proposta, ignora o limite de custo, não revalida TTL ou revogação durante a execução, permite retry após falha parcial sem recovery, resolve atestações de operações alheias e mantém um ledger que pode ser apagado ou adulterado.

A conclusão defensável é **`not_proven`**. Os testes atuais passam porque verificam contratos simplificados e contadores sintéticos; não demonstram autenticação, vínculo de autoridade, persistência, execução do plano ou reconciliação independente.

## Evidência de reprodução

A suíte específica passou: executor **5/5**, holdout **5/5** e Stage 5 **4/4**. A suíte documentada completa passou com **142 testes, 1 skipped e 0 falhas**. Isso é uma linha de base de testes, não uma aprovação de segurança.

As seguintes sondas foram executadas sem alterar o checkout:

| Sonda | Resultado observado | Propriedade que deveria valer |
|---|---|---|
| `RecoveryAttestation('foreign', 99, 'RESOLVED_COMMITTED', 'random', 'foreign-ledger', 999)` em executor novo | `RECOVERY_RESOLVED` | Recovery desconhecido deve permanecer `RECOVERY_REQUIRED` |
| Envelope com `max_steps=2`, `max_cost=1` | `COMMITTED`, `effects=2` | Overrun deve bloquear antes do segundo efeito |
| Envelope com `expires_at=1`, três passos e relógio inicial zero | `COMMITTED`, relógio final 3 | Expiração entre passos deve interromper e produzir estado parcial/desconhecido |
| Falha parcial no passo 1 seguida de tentativa 2 com nonce novo, sem recovery | `FAILED_PARTIAL` seguido de `COMMITTED` | Retry deve ser `RETRY_BLOCKED` |
| Envelope com `proposal_digest`, `action_sequence_digest`, `host_context_digest`, `operation_id` e tentativa arbitrários | `COMMITTED` | Campos devem ser comparados a objetos registrados e à autoridade externa |
| Ledger após mutação do evento, mutação pública de `events` e `events.clear()` | Digest muda; limpeza permitida | Ledger deve ser imutável, encadeado e durável |
| Envelope inválido com nonce `999` e `max_steps=0`, seguido de nonce `1` | Primeiro `REJECTED`; segundo também `REJECTED` | Falha de validação não deve consumir nonce antes de validar limites |

## Achados críticos e altos

### E1 — Não existe assinatura nem autenticação verificável da autoridade

**Severidade: crítica.** `ExecutionEnvelope` contém apenas strings de identidade e limites; não possui `authority_signature`, `authority_key_id`, epoch autenticado, digest de revogação, `not_before`, `clock_id` ou `parent_ledger_digest` [3]. `ExternalExecutorAdapter.execute()` compara somente `envelope.authority_id` com uma string local e aceita `no_automatic_retry=True` [4]. Qualquer processo que conheça a string `external-authority` pode fabricar um envelope aceito.

O problema também contamina o modo estrito: `SkillObservabilityContract.valid_for_strict()` e `HostObservabilityContract.valid_for_strict()` exigem somente nomes de schema e strings não vazias. Elas não recomputam digest, não verificam assinatura, emissor, escopo, host, skill, época ou validade temporal [5]. Assim, `SUPPORTED` pode ser obtido com contratos textuais forjados, inclusive quando o efeito oculto do holdout continua presente.

**Impacto.** A fronteira de autoridade é nominal, não criptográfica nem independente. O runtime, um chamador malicioso ou um host comprometido pode promover sua própria proposta a autorização.

**Mudanças necessárias.** Adicionar envelope versionado com assinatura verificável por uma raiz externa. A assinatura deve cobrir, no mínimo, `herus_id`, `host_id`, digest do contexto, `skill_id`, digest completo da proposta, sequência de ações, operação, tentativa, nonce, janela temporal, limites, epoch da autoridade, snapshot de revogação e política de retry. Separar `CONTRACT_PRESENT`, `ATTESTED` e `SUPPORTED`; campos não vazios nunca devem bastar para `SUPPORTED`.

**Testes de reabertura.** Alterar cada campo normativo depois da assinatura deve falhar. Trocar `host_id`, `host_context_digest`, `skill_id`, proposta, chave, epoch, snapshot de revogação ou autoridade deve produzir `REJECTED` e zero efeitos. Contrato forjado deve retornar `UNSUPPORTED_BY_CONTRACT` ou `SAFE_BUT_UNPROVEN`, nunca `SUPPORTED`.

### E2 — O executor não valida nem despacha a proposta descrita pelo envelope

**Severidade: crítica.** O loop de `executor.py` não recebe `TransferProposal`, lista de ações, dispatcher ou host. Ele incrementa `effects`, cria um recibo sintético e retorna `COMMITTED` [4]. `proposal_digest`, `action_sequence_digest` e `host_context_digest` nunca são comparados com bytes canônicos registrados. Portanto, o componente não demonstra execução autorizada; demonstra apenas contabilidade de passos fictícios.

**Impacto.** É possível obter `COMMITTED` para uma operação, host e plano inexistentes. O estado terminal não prova que a ação correta foi chamada, na ordem correta, com argumentos corretos, no host correto.

**Mudanças necessárias.** Receber apenas proposta serializada e dispatcher externo registrado. Validar todos os digests contra registros imutáveis antes da reserva do nonce. Despachar exatamente a ação registrada por passo. Se a implementação continuar sem dispatcher real, renomear o componente para simulador de bookkeeping e remover qualquer alegação de execução externa.

**Testes de reabertura.** Registrar um dispatcher que falha se receber ação inesperada. Exigir uma chamada por ação, na ordem exata. Digest da proposta ou da sequência alterado deve resultar em zero chamadas. Um `COMMITTED` só pode ocorrer com recibo terminal válido para cada passo realmente despachado.

### E3 — `max_cost` é decorativo e permite overrun

**Severidade: alta.** O executor apenas exige `max_cost >= 1`; não debita custo e não compara o custo acumulado ao limite [4]. A sonda reproduziu `max_steps=2`, `max_cost=1` como `COMMITTED` com dois efeitos.

O mesmo defeito existe na decisão Stage 5: o ledger é criado, mas `propose_transfer_checked()` não chama `with_probe()`, `with_reset()` ou qualquer contador de passo/custo [6]. Uma host com limite de uma probe executou três probes e recebeu ledger com `probe_count=0`, `event_seq=0` e estado positivo.

**Mudanças necessárias.** Consumir permissão antes de cada `observe`, probe, reset, passo, retry e recovery. Cobrar custo observado por passo. Custo desconhecido deve bloquear; custo acima do teto deve produzir `COST_OVERRUN`, sem converter o resultado em sucesso. O orçamento precisa ser persistente por `budget_scope_id`, não recriado a cada chamada.

**Testes de reabertura.** Orçamento zero deve impedir qualquer chamada ao host. `actual_cost > max_cost` deve impedir o efeito excedente. Os contadores retornados devem ser derivados da trilha e iguais aos contadores observados no host e no executor. Uma segunda operação no mesmo escopo deve consumir o saldo restante ou ser bloqueada.

### E4 — TTL usa relógio inadequado e é verificado somente uma vez

**Severidade: alta.** O envelope possui apenas `expires_at`; não há `not_before`, `clock_id` ou política de TTL máximo [3]. O executor testa `expires_at <= clock` apenas antes do loop. Como o relógio avança dentro do loop, uma autorização já expirada durante a execução continua produzindo commits. A sonda com `expires_at=1` e três passos terminou `COMMITTED` com relógio 3.

O campo `clock` é um inteiro mutável do objeto. Não há relógio monotônico identificado, proteção contra retrocesso, injeção determinística de clock ou persistência de último tempo aceito.

**Mudanças necessárias.** Usar relógio monotônico identificado e injetável. Validar `not_before < expires_at`, TTL máximo e clock conhecido. Revalidar antes da reserva, antes de cada passo e imediatamente antes de cada commit. Expiração com passo em voo deve resultar em `UNKNOWN_OUTCOME` e `RECOVERY_REQUIRED`, nunca em rollback implícito ou `COMMITTED`.

**Testes de reabertura.** Cobrir expiração antes da reserva, entre passos, durante passo em voo e após ACK tardio. Fazer o relógio retroceder e exigir rejeição ou estado seguro. Testar TTL acima do máximo, `expires_at <= not_before`, clock desconhecido e snapshot stale.

### E5 — Revogação é apenas um contador local e não domina execução em voo

**Severidade: alta.** `revoke()` apenas incrementa `revocation_epoch`; `execute()` compara o epoch uma única vez antes do primeiro passo [4]. Não há snapshot assinado, digest comprometido pelo envelope, lista de objetos revogados ou verificação entre passos. Revogar depois do primeiro commit não impede o segundo passo.

**Mudanças necessárias.** A autoridade deve emitir uma visão versionada de revogação. O envelope deve comprometer o digest dessa visão. O executor deve verificar a visão antes da reserva e novamente antes de cada passo. Revogação antes do início deve rejeitar sem efeito; revogação em voo deve bloquear o próximo passo e conservar `FAILED_PARTIAL` ou `UNKNOWN_OUTCOME` conforme a qualidade do recibo.

**Testes de reabertura.** Revogar envelope, operação, host, epoch e autoridade antes da execução e entre passos. Exigir zero efeito no primeiro caso e nenhum passo posterior no segundo. Snapshot ausente, adulterado ou stale deve ser rejeitado.

### E6 — Recovery resolve qualquer operação alheia

**Severidade: crítica.** `recover()` rejeita somente `source_id == self.authority_id` e verifica se `conclusion` pertence a um conjunto de strings [4]. Não compara `operation_id`, `attempt_no`, nonce, digest do ledger, passos comprometidos, recibos ou existência de uma tentativa ativa. A atestação estrangeira da tabela acima produziu `RECOVERY_RESOLVED` em executor novo.

**Impacto.** Um atacante pode limpar uma trava, declarar uma operação como concluída ou liberar o caminho para retry sem provar o que aconteceu.

**Mudanças necessárias.** Manter contexto durável de cada tentativa. Aceitar somente atestação assinada por fonte independente autorizada e vinculada à operação, tentativa, nonce, digest do ledger, epoch, recibos e contagem de commits. Atestação desconhecida, conflitante, replayada ou com passos incompatíveis deve permanecer `RECOVERY_REQUIRED`.

**Testes de reabertura.** Mutar cada campo da atestação; testar replay, operação inexistente, tentativa errada, ledger errado, nonce errado, contador maior que recibos e dois atestadores conflitantes. Nenhum caso pode retornar `RECOVERY_RESOLVED` sem vínculo completo.

### E7 — Retry com nonce novo é aceito sem recovery e rompe a máquina de estados

**Severidade: crítica.** Depois de `FAILED_PARTIAL`, o adaptador não conserva uma operação ativa nem bloqueia novas tentativas. A sonda executou uma falha parcial e, sem qualquer `RecoveryAttestation`, aceitou uma tentativa 2 com nonce novo como `COMMITTED`. Isso viola diretamente a regra de retry bloqueado descrita no plano de executor [7].

**Mudanças necessárias.** Implementar estado por `operation_id`: `REJECTED → AUTHORIZED → IN_FLIGHT → COMMITTED`, ou `FAILED_PARTIAL/UNKNOWN_OUTCOME → RECOVERY_REQUIRED`. Qualquer novo envelope para operação não resolvida deve retornar `RETRY_BLOCKED`, mesmo com nonce novo. Só uma atestação válida pode permitir nova autorização externa com tentativa maior, nonce novo, TTL novo e sequência residual explícita.

**Testes de reabertura.** Repetir o mesmo envelope, usar nonce novo, reiniciar o adaptador, chamar `bind()`/`reset()` e tentar novo processo. Todos devem bloquear antes de qualquer chamada externa enquanto não existir recovery válido. Depois de `RESOLVED_NOT_COMMITTED`, ainda exigir novo envelope assinado; recovery não pode autoautorizar.

### E8 — Nonce não é uma reserva durável, escopada ou canônica

**Severidade: alta.** O nonce é convertido para inteiro e comparado a um único `nonce_floor` volátil [4]. Não há reserva atômica em ledger durável, namespace por autoridade/operação/tentativa, entropia mínima ou digest do envelope. A validação também atualiza `nonce_floor` antes de validar `max_steps` e `max_cost`; a sonda mostrou que um envelope inválido com nonce 999 impede depois o uso de nonce 1. Isso permite negação de serviço e deixa a semântica de consumo ambígua.

**Mudanças necessárias.** Definir nonce canônico de alta entropia, com formato estrito. Reservá-lo atomicamente somente depois de validar schema, assinatura, escopo, janela, revogação, proposta e limites, mas antes do primeiro efeito. Persistir a reserva e vinculá-la a `(authority_id, operation_id, attempt_no, nonce, envelope_digest)`. Reboot não pode apagar consumo; alias textual não pode criar duas representações do mesmo nonce.

**Testes de reabertura.** Cobrir replay exato, nonce equivalente em representação diferente, nonce reutilizado em outra operação, nonce menor/maior após reboot, falha entre reserva e dispatch e tentativa inválida que não deve consumir nonce. Corrida de duas reservas deve aceitar no máximo uma.

### E9 — Ledger “append-only” é mutável, apagável e não verificável

**Severidade: alta.** `AppendOnlyLedger` expõe `events` como lista pública, faz cópia rasa ao inserir e ao gerar snapshot, e não oferece verificação de cadeia [8]. Alterar um dicionário aninhado depois do append muda retroativamente o digest. Alterar `ledger.events[0]` também muda a cadeia, e `ledger.events.clear()` apaga todo o histórico sem erro.

**Impacto.** `parent_digest` e `ledger_digest` não são evidência estável. Rollback de estado, adulteração de recibos e remoção de reservas de nonce ficam triviais.

**Mudanças necessárias.** Congelar bytes canônicos de cada evento, fazer cópia profunda, registrar `event_seq`, `previous_event_digest` e `event_digest`, e expor `verify_chain()` fail-closed. Persistir com CAS ou armazenamento transacional que rejeite sequência regressiva, snapshot velho, parent incorreto e truncamento. O adaptador deve bloquear se a durabilidade não puder ser demonstrada.

**Testes de reabertura.** Mutar evento de entrada, evento público, snapshot, sequência, parent, payload e ordem. Cada mutação deve ser rejeitada ou detectada por `verify_chain()`. Limpar ou substituir o histórico deve falhar. Reboot deve preservar digest e monotonicidade.

### E10 — Recibos não são atestações de commit

**Severidade: alta.** `ExecutionReceipt` contém operação, tentativa, passo, nonce, evento, `committed`, custo e parent digest, mas não inclui host, digest da proposta, sequência de ações, epoch da autoridade, clock, snapshot de revogação ou assinatura [3]. Seu digest é apenas SHA-256 de dados fornecidos pelo próprio processo. Não existe validação de recibos contra dispatcher ou ledger.

**Mudanças necessárias.** Vincular cada recibo a envelope, proposta, host, contexto, autoridade, nonce, passo canônico, relógio e evento anterior. Exigir atestação do executor/atuador e validar a cadeia antes de declarar `COMMITTED`. ACK isolado, número de chamadas e `expected_final` não são recibos terminais.

**Testes de reabertura.** Alterar cada vínculo do recibo, remover recibo de um passo, duplicar passo, reordenar recibos e fornecer recibos conflitantes. O resultado deve ser `UNKNOWN_OUTCOME + RECOVERY_REQUIRED`, nunca sucesso.

### E11 — Holdout de partial failure, overrun e oracle é cosmético/circular

**Severidade: alta.** O holdout declara m7 como partial e m8 como overrun, mas `run_case()` nunca chama o executor para fixtures negativas; ele só chama o executor no controle, quando o oráculo retorna `SUPPORTED` [9]. `cost=2` é apenas metadado e `cost_actual` é calculado por fórmula do harness. Não existe ACK ausente, dispatch parcial ou orçamento aplicado.

Além disso, `IndependentOracle.evaluate()` não calcula a verdade privada: copia `fixture.expected_negative` e `fixture.oracle_reason` [9]. Alterar esses rótulos pode transformar a fixture opaca em `SUPPORTED`, mesmo mantendo `true_effects` com `damage += 1`. A autoridade do oráculo é, portanto, circular com a própria fixture.

**Mudanças necessárias.** Separar expectativa de teste da verdade privada. Fazer o oráculo comparar a proposta real com uma tabela/atuador independente e imutável. Modelar no fixture `max_cost`, plano e falha de ACK/commit. Executar m7 e m8 no adaptador externo, registrar recibos e emitir `PARTIAL_FAILURE`, `UNKNOWN_OUTCOME` ou `COST_OVERRUN` somente quando a condição ocorrer.

**Testes de reabertura.** Mutar `expected_negative`, `oracle_verdict` e `oracle_reason` sem alterar a verdade; o veredicto não pode mudar. Exigir traços distintos do controle para ACK ausente, partial e overrun. Validar que os contadores sejam derivados do trace e que a proposta continue com zero execução externa.

### E12 — Validador de evidência aceita adulteração material

**Severidade: alta.** `validate_result()` verifica presença de campos, contadores não negativos, ausência de execução na proposta e ausência de efeito em negativos [10]. Não recompõe `raw_trace_digest`, não deriva custo/probes do trace, não valida relações entre status e oracle, nem cobertura e repetição. Status, oracle, custo, contadores, digest e `repeat` podem ser alterados sem violation.

**Mudanças necessárias.** Validar primeiro o trace canônico e derivar todos os campos. Impor matriz de estados por fixture, três repetições únicas, `split`, digest de fixture, commit, versão de contrato, seed e comando. Rejeitar qualquer digest que não seja recomputável.

**Testes de reabertura.** Suite de mutação para cada campo material. Alterar status, oracle, custo, contadores, trace, digest, repeat, fixture e ordem deve falhar. Um resultado extra, repetido ou ausente deve bloquear a campanha.

## Achados de cobertura e exagero documental

1. O documento da etapa 5 diz que o adaptador “exige autoridade externa, rejeita replay de nonce, verifica TTL e época de revogação” [2]. O código só compara uma string de autoridade, usa um piso de nonce em memória, verifica TTL uma vez e não possui revogação em voo. A documentação deve chamar essa implementação de **simulador experimental incompleto** até os gates acima existirem.
2. A etapa 5 afirma que há 142 testes host-only, mas a aprovação mede apenas as propriedades codificadas nos asserts. Não há um teste de assinatura ausente, digest forjado, clock que avança durante o passo, revogação em voo, retry bloqueado, atestação alheia, rollback de ledger ou persistência após reboot.
3. `recovery`, `nonce`, `TTL` e revogação são explicitamente deixados para adaptador externo na correção de observabilidade [2]. Isso é honesto como limitação, mas incompatível com a linguagem de “agora possui” um adaptador seguro em [2]. A interface atual não deve ser usada para inferir segurança física, autorização ou sucesso terminal.
4. O protocolo social permanece `protocol_ready_no_human_data` e exige zero execução externa não consentida [11]. Ele não fornece evidência que corrija qualquer achado desta auditoria.

## Plano de correção priorizado

1. **Bloqueio imediato:** remover ou desabilitar qualquer caminho que produza `SUPPORTED` ou `COMMITTED` sem assinatura, proposta registrada, dispatcher real, ledger verificável e recovery vinculado.
2. **Protocolo canônico:** implementar envelope versionado assinado, digest completo de proposta e ações, autoridade/epoch/chave verificáveis, nonce de alta entropia, clock monotônico, TTL completo e snapshot de revogação.
3. **Máquina de estados:** persistir estado por operação e bloquear retry após `FAILED_PARTIAL`, `UNKNOWN_OUTCOME`, expiração em voo ou revogação até atestação independente válida.
4. **Ledger:** substituir lista pública por cadeia imutável, profunda, durável e verificável; rejeitar rollback e parent incorreto.
5. **Execução real:** separar dispatcher/atuador do runtime e produzir recibos atestados por ação; se isso não existir, renomear o módulo e rebaixar suas alegações.
6. **Orçamento efetivo:** consumir probe, reset, passo, custo, retry e recovery antes da operação; tornar o escopo persistente e monotônico.
7. **Holdout honesto:** executar realmente partial, ACK ausente, revogação, stale clock, overrun, replay, ordem não comutativa e recovery conflitante. Substituir oracle baseado em rótulos por avaliação privada independente.
8. **Mutação e CI:** adicionar os testes de reabertura abaixo e exigir que qualquer falha mantenha `not_proven`.

## Testes mínimos obrigatórios antes de reabrir a alegação

- Envelope ausente, sem assinatura, com assinatura errada e com cada digest alterado: rejeição e zero efeitos.
- Reuso do mesmo envelope/nonce, nonce equivalente, nonce de outra operação, corrida de reserva e reboot entre reserva e dispatch.
- TTL inválido, expiração antes do início, expiração entre passos, clock retrocedente, clock desconhecido e ACK tardio.
- Revogação antes da reserva e entre passos para envelope, operação, host, epoch e autoridade.
- `max_steps`, `max_cost`, custo desconhecido, custo excedente e limites residuais após recovery.
- Falha explícita depois de commit, ACK ausente, queda entre dispatch e commit e recibos conflitantes.
- Retry com nonce novo sem recovery, recovery de operação errada, atestação replayada, fonte não permitida e conflito entre fontes.
- Mutação de evento, snapshot, parent, sequência e truncamento do ledger; `verify_chain()` deve detectar tudo.
- Dispatcher que verifica ação, ordem, host e argumentos; `COMMITTED` sem chamada real deve ser impossível.
- Mutação de `expected_negative`, `oracle_verdict`, `oracle_reason`, status, custo, contadores, repeat e trace; o benchmark deve rejeitar toda adulteração.
- Propriedade global: construção de proposta nunca chama executor autorizado; apenas um envelope externo validado pode produzir qualquer efeito.

## Conclusão

A arquitetura escrita distingue corretamente proposta, autorização, execução e recovery, mas a implementação auditada ainda não realiza essa separação com garantias de segurança. O nonce é volátil e pode ser consumido por envelope inválido; TTL e revogação não são reavaliados durante a execução; o custo não é aplicado; o retry não é bloqueado; recovery é aceito para operação alheia; o ledger pode ser reescrito; recibos não provam commit; e o holdout atribui falhas sem executar as condições correspondentes. Até que essas falhas sejam corrigidas e os testes de mutação passem, não há base para `SUPPORTED`, execução autorizada, segurança física ou “simbiose útil”.

## Referências

[1]: `../../../docs/54-CORRECAO-CONTRATO-OBSERVABILIDADE.md` "Etapa 5 — contrato de observabilidade e decisão checked"
[2]: `../../../docs/55-ETAPAS-FINAIS-EXECUTOR-HUMANO.md` "Etapas finais — execução externa, holdout estendido e prova humana"
[3]: `../../stage4/execution_contracts.py` "Contratos de envelope, receipt e recovery"
[4]: `../../stage4/executor.py` "Adaptador experimental de executor externo"
[5]: `../../symbiont_v2/stage5.py` "Contratos Stage 5 e ledger de orçamento"
[6]: `../../symbiont_v2/core.py` "Runtime simbiont e propose_transfer_checked"
[7]: `../remaining_plan/01-external_executor.md` "Plano restante para executor externo e recuperação"
[8]: `../../stage4/ledger.py` "Ledger append-only experimental"
[9]: `../../holdout_adversarial.py` "Campanha holdout adversarial e oracle"
[10]: `../../stage4/contracts.py` "Validação dos registros do holdout"
[11]: `../../social/utility_protocol_v1.json` "Protocolo social de utilidade humana"

**Reprodução principal:**

```bash
cd /home/ubuntu/herus
PYTHONPATH=research python3 -m unittest research.test_external_executor -v
PYTHONPATH=research python3 -m unittest research.test_holdout_adversarial -v
PYTHONPATH=research python3 -m unittest research.test_stage5_synthesis -v
PYTHONPATH=research python3 -m unittest discover -s research -p 'test_*.py' -q
```

**Sondas adversariais:** resultados registrados no processo de auditoria; nenhum arquivo de código do repositório foi modificado.
