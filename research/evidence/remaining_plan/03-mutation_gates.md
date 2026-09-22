# HERUS — mutações e gates adversariais

**ID:** `HERUS-REM-MUT-GATES-003`  
**Frente:** mutações que removem abstention, zeram custo desconhecido, executam durante a proposta ou aceitam digest sem atestação.  
**Status:** desenho implementável; **nenhuma alteração de código foi feita nesta frente**.  
**Escopo:** mecanismo host-only, decisão tipada, orçamento e fronteira proposta–execução. O desenho não concede autoridade física nem mede utilidade social.

## 1. Decisão executiva

A correção só deve ser aceita quando uma campanha adversarial matar, por comportamento observável, quatro classes de regressão:

1. **Remoção de abstention:** o sistema escolhe uma ação, devolve uma proposta acionável ou continua a sondar quando a evidência é ambígua, stale, inválida, conflitante ou insuficiente.
2. **Custo desconhecido tratado como zero:** um evento sem medição, recibo ou limite superior finito é convertido em `0`, `OPEN` ou orçamento infinito.
3. **Execução durante a proposta:** a construção ou verificação de um plano chama o executor autorizado, despacha a ação alvo ou produz efeito externo.
4. **Digest aceito sem atestação:** um SHA-256, um digest emitido pelo próprio runtime ou uma declaração sem emissor independente é aceito como prova de completude, fechamento de efeitos, autoridade ou segurança.

O teste deve matar a mutação pelo seu efeito: proposta não nula, estado incorreto, chamada extra, contador incompatível, custo numérico fabricado ou alegação `SUPPORTED` indevida. Não basta verificar a string `reason`, porque um mutante pode conservar `ABSTAIN` no registro e ainda devolver uma ação ou executar um efeito.

A linha de base Stage 4 permanece congelada. O arquivo `research/evidence/holdout_benchmark_v1.json`, seus quinze registros e a classificação histórica `not_proven` não devem ser reescritos. A mudança de semântica aplica-se ao caminho tipado novo, especialmente `propose_transfer_checked(..., mode="STRICT")`, e não retroativamente aos wrappers legados [1] [2].

O resultado esperado para o caminho estrito é:

- `ABSTAIN` com `proposal: null` quando há conflito, alias, stale, digest inválido ou outra incerteza episódica;
- `UNSUPPORTED_BY_CONTRACT` com `proposal: null` quando o contrato não permite a alegação, por exemplo quando falta fechamento independente, contrato de observabilidade ou limite de custo;
- `PROPOSED + SAFE_BUT_UNPROVEN` somente no caminho de compatibilidade, sem autorização;
- `PROPOSED + SUPPORTED` apenas no controle positivo com cobertura atual, custo verificável, ação unívoca, probe declaradamente não mutante e atestação independente;
- em todos os casos de proposta, `proposal_execute_calls == 0`, `authorized_calls == 0`, `external_effect_count == 0` e `step_count == 0`.

`SUPPORTED` continua sendo uma alegação sobre um contrato de evidência. Não é autorização para executar. A autorização permanece fora do runtime, em um envelope externo validado por um executor separado [3] [4].

## 2. Estado auditado e lacunas que os gates devem fechar

A correção de observabilidade já adicionou `propose_transfer_checked`, tipos de decisão e um ledger de orçamento aditivo. O contrato público correto é não inferir `damage`, completude ou ausência de efeitos ocultos a partir do delta de `x`; em um host sem contrato, a decisão estrita deve terminar antes da primeira probe [2].

O código presente ainda deve ser tratado como superfície a ser protegida pela campanha, não como prova de que todos os gates já estão implementados. Em `research/symbiont_v2/core.py`, o caminho estrito verifica a presença e a validação superficial dos contratos e um `cost_max` fornecido pelo chamador, mas depois chama o `propose_transfer` legado. Em particular, a decisão atualiza `probe_execute_calls` como zero embora `discover()` use `host.execute()` para sondagem; isso exige telemetria separada e uma correção futura, não uma interpretação benevolente do contador [7].

Em `research/symbiont_v2/stage5.py`, `BudgetLedger` é um snapshot imutável com incrementos locais. Ele não substitui o ledger durável do executor externo, não recebe recibos de custo por evento e não prova que uma cota foi cobrada antes da chamada. A campanha deve distinguir o ledger da proposta do ledger de execução e não declarar o orçamento fechado apenas porque o objeto contém `cost_max` [8].

A fronteira mínima é, portanto:

```text
host público + contratos públicos
        ↓
runtime: preflight, observação, orçamento de proposta e plano
        ↓ fecha a decisão sem executor
oráculo privado: avalia somente depois, com tabela independente
        ↓ somente após autoridade externa
executor: valida envelope, nonce, TTL, revogação e recibos
```

Uma probe de `discover()` pode chamar `host.execute()`, mas isso é sondagem e não execução autorizada. Os canais devem ter contadores distintos. Se o host não declara `probe_mode == NON_MUTATING`, o modo estrito deve bloquear antes da probe quando a decisão depender de uma sondagem sem efeito [3] [4].

## 3. Gate normativo e ordem de avaliação

A ordem deve ser determinística e observável. Nenhuma etapa posterior pode reparar uma violação anterior.

### 3.1 Preflight sem efeito

Antes de `discover()` ou de qualquer chamada que possa alterar o host, validar:

1. `skill_id`, status `VERIFIED`, modo e limites, rejeitando booleanos, frações, negativos e aliases conflitantes;
2. contrato opcional da Skill e do host, incluindo `read_set`, política para `MISSING`/`STALE`/`HIDDEN`, `coverage`, `effect_closure` e `probe_mode`;
3. digest, versão, tipo, sequência, época e frescura das observações públicas;
4. limite de custo finito ou limite superior finito atestado para cada operação que possa consumir recurso;
5. ausência de alias e conflito já observáveis;
6. que uma atestação de fechamento possui emissor, versão, proveniência e vínculo ao contrato, e não foi apenas calculada pelo runtime.

Se o preflight estrito falhar por contrato ausente, a saída é `UNSUPPORTED_BY_CONTRACT` com `proposal: null` e zero probes. Se falhar por observação inválida, stale, conflitada ou ambígua, a saída é `ABSTAIN` com motivo tipado. O runtime não deve aprender o motivo privado do oráculo para escolher entre essas saídas.

### 3.2 Cobrança antes da chamada

Cada tentativa de probe, reset, passo, retry ou recovery deve gerar evento antes da chamada potencialmente mutante. O evento inclui `event_seq`, fase, incremento, escopo, limite aplicável e cotação/recibo público. Se a chamada lançar exceção, expirar ou não retornar, a tentativa continua contada.

A semântica de custo é:

- `EXACT`: todos os eventos relevantes têm quantidade medida;
- `BOUNDED_UNKNOWN`: existe limite superior finito, mas o valor real não foi medido;
- `UNKNOWN`: falta medição, recibo ou limite superior finito;
- `OVERRUN`: a cobrança conhecida excedeu `cost_max`.

Em `UNKNOWN`, `cost_actual` é `null`, `unknown_event_count > 0` e o estado não pode ser `OPEN` por default. `MEASURED_ZERO` é o único modo de registrar custo zero. Ausência de campo, `None`, timeout ou erro não são `MEASURED_ZERO` [4].

### 3.3 Construção da proposta sem executor

Somente depois do preflight e da cobrança permitida o runtime pode fazer discovery. Ao construir a proposta, não pode receber um executor como parâmetro, emitir envelope, assinar autorização, reservar nonce ou chamar `execute`, `commit`, `link_send` ou equivalente. A proposta contém plano e evidências públicas; não contém autoridade.

Depois que o runtime fecha a decisão, o oráculo privado avalia a trajetória verdadeira. O `oracle_reason` é gravado separadamente. Ele não pode ser copiado para `runtime_reason`: por exemplo, um oráculo pode conhecer `HIDDEN_EFFECT_UNVERIFIABLE`, enquanto o runtime só pode afirmar `OBSERVABILITY_CLOSURE_UNPROVEN` se o host não declarou o campo oculto [3] [5].

## 4. Mutações obrigatórias e como matá-las

A campanha deve copiar o módulo sob teste para um diretório temporário, aplicar uma substituição controlada ou transformação AST, verificar que exatamente um controle foi alterado e executar a suíte em processo separado. O repositório principal e os artefatos v1 ficam intocados. Cada mutante recebe um ID, a alteração exata, os testes matadores e a evidência mínima de morte.

### 4.1 Remoção de abstention

| ID | Mutação | Fixture/teste matador | Evidência exigida |
|---|---|---|---|
| `M-ABS-01` | Remover o retorno nulo para alias e escolher a primeira ação por ordem ou nome. | `ABS-01`: duas ações com o mesmo efeito público; IDs e enumeração permutados. | `proposal is None`, `ABSTAIN`, `OBSERVATION_ALIAS`, nenhum `action_id` escolhido. O mutante falha se devolver qualquer proposta. |
| `M-ABS-02` | Tratar conflito de observações como consenso, por exemplo conservar a primeira evidência. | `ABS-02`: mesma ação e mesmo contexto público com dois deltas incompatíveis. | `ABSTAIN + OBSERVATION_CONFLICT`; evidência conflitada não entra em Skill nem em proposta. |
| `M-ABS-03` | Reutilizar observação stale após `bind`, `reset`, mudança de época ou revalidação ausente. | `ABS-03` e variantes do holdout temporal. | `ABSTAIN + OBSERVATION_STALE` ou `UNSUPPORTED_BY_CONTRACT`; nenhuma meta pode ser satisfeita com valor antigo. |
| `M-ABS-04` | Aceitar digest, tipo, sequência ou época adulterados. | `ABS-04`: alterar um byte ou campo sem recalcular o digest. | `ABSTAIN + OBSERVATION_DIGEST_INVALID`; não pode ocorrer probe subsequente. |
| `M-ABS-05` | Converter `UNSUPPORTED_BY_CONTRACT` em `PROPOSED`, ou devolver plano no modo estrito após contrato ausente. | `S5-CONTRACT-01`, `S5-M6A-01` e host renomeado sem contrato. | `proposal: null`, zero probes e `safety_claim: NONE`. |
| `M-ABS-06` | Executar discovery antes do preflight, mesmo que o resultado final diga `ABSTAIN`. | Host espião com contrato ausente e contador de probes. | O teste falha se `probe_count > 0`; a razão correta não compensa um efeito feito antes do gate. |

A aceitação não deve exigir sempre uma única string entre `ABSTAIN` e `UNSUPPORTED_BY_CONTRACT`; deve exigir o par semântico correto. Alias, conflito, stale e digest inválido são abstenção episódica. Lacuna estável de contrato ou atestação é `UNSUPPORTED_BY_CONTRACT`. Uma proposta compatível rebaixada a `SAFE_BUT_UNPROVEN` nunca conta como abstention correta e nunca autoriza execução [3].

### 4.2 Custo desconhecido tratado como zero ou infinito

| ID | Mutação | Fixture/teste matador | Evidência exigida |
|---|---|---|---|
| `M-COST-01` | Substituir `None`, `UNKNOWN`, timeout ou recibo inválido por custo `0`. | `COST-01`, `COST-02`, `COST-03`. | `cost_actual is None`, `cost_status == UNKNOWN`, `unknown_event_count > 0`; nunca `OPEN` ou `SUPPORTED`. |
| `M-COST-02` | Interpretar `cost_max` ausente como infinito. | Limite ausente em modo estrito, sem cotação do host. | Bloqueio por `COST_LIMIT_MISSING` ou `UNKNOWN_COST` antes da chamada. |
| `M-COST-03` | Aceitar `UPPER_BOUND > cost_max` ou recibo real acima da cotação. | `COST-04` e `COST-05`, com limites menor, igual e maior. | Maior bloqueia; overrun produz `OVERRUN + COST_OVERRUN`; nenhuma operação posterior começa. |
| `M-COST-04` | Não cobrar reset, recovery ou tentativa que falha; cobrar somente depois da chamada. | `COST-03` e `COST-07`, incluindo exceção antes do retorno. | Contagem monotônica antes da chamada; reset não é gratuito e recovery não é reset. |
| `M-COST-05` | Usar `budget_exhausted == false` do schema legado como prova de ledger novo aberto. | Registro v1 com custo não observável e ledger Stage 5 `UNKNOWN`. | O resultado novo permanece `UNKNOWN`/bloqueado; o campo legado não domina o ledger novo. |
| `M-COST-06` | Aceitar aliases `max_cost` e `cost_max` conflitantes por precedência implícita. | `COST-08`. | Rejeição determinística, zero chamadas e zero efeito. |

`COST-06`, com duas fixtures de mesmo traço público e custos privados distintos, é obrigatório para impedir que o teste consulte a verdade privada como se fosse cotação pública. Se o host não publica custo, ambas devem produzir a mesma decisão pública de custo desconhecido; o oráculo pode divergir depois, mas não pode fornecer o número ao runtime [4].

### 4.3 Executor chamado durante a proposta

| ID | Mutação | Fixture/teste matador | Evidência exigida |
|---|---|---|---|
| `M-SEP-01` | Chamar executor autorizado dentro de `propose_transfer_checked` ou `propose_transfer`. | `SEP-01`, com executor sentinela que falha ao primeiro chamado. | `proposal_execute_calls == 0`, `authorized_calls == 0`, `external_effect_count == 0`, `step_count == 0`. |
| `M-SEP-02` | Despachar a ação alvo depois de descobrir a correspondência, confundindo plano com execução. | `SEP-01` com host e executor em processos distintos. | Nenhum evento `execution_requested`, `action_started`, `commit` ou efeito externo durante a proposta. |
| `M-SEP-03` | Contabilizar probe como autorização, ou esconder execução alvo dentro de `probe_execute_calls`. | Fixture com probe legítima e executor autorizado separado. | `probe_execute_calls` pode refletir somente sondas públicas; `authorized_calls` permanece zero. Uma probe não é autorização. |
| `M-SEP-04` | Emitir automaticamente envelope ou assinatura quando a decisão for `SUPPORTED`. | `AUTH-01` e teste de inspeção de canal serializado. | Nenhum envelope de execução sai do runtime; proposta e decisão não têm chave, nonce reservado ou assinatura de autoridade. |

O teste deve capturar o traço imediatamente após a proposta, antes de consultar o oráculo. Uma ação que tenha sido executada e depois revertida continua sendo falha. `reset()` posterior não apaga a violação nem prova rollback. A campanha deve reportar separadamente `probe_count`, `probe_execute_calls`, `authorized_calls`, `committed_calls`, `proposal_execute_calls` e `external_effect_count` [5].

### 4.4 Digest aceito sem atestação independente

| ID | Mutação | Fixture/teste matador | Evidência exigida |
|---|---|---|---|
| `M-ATT-01` | Tratar digest de `Effect.from_states()` como fechamento completo de efeitos. | `OBS-01`: duas fixtures com o mesmo delta público; uma possui efeito privado proibido. | Sem atestação externa, ambas ficam `UNSUPPORTED_BY_CONTRACT`/`SAFE_BUT_UNPROVEN`; nenhuma recebe `SUPPORTED`. |
| `M-ATT-02` | Aceitar `closure_evidence_digest` criado pelo próprio runtime. | `OBS-04`: emissor e proveniência do digest são do runtime. | `OBSERVABILITY_CLOSURE_UNPROVEN`; SHA-256 correto não cria independência. |
| `M-ATT-03` | Aceitar `contract_digest` ou `observation_digest` como prova de autenticidade do host. | Digest válido com host não autenticado ou payload público incompleto. | Integridade pode passar, mas o gate de autenticidade/completude bloqueia. |
| `M-ATT-04` | Aceitar `TransferProposal`, `transfer() == True` ou Skill verificada como autoridade. | `AUTH-01` sem envelope externo e sem emissor confiável. | `REJECTED + MISSING_AUTHORITY`, zero reserva, zero passo e zero efeito. |
| `M-ATT-05` | Copiar o `oracle_reason` para `runtime_reason`. | `ORACLE-01` com motivo privado alterado sem alterar a projeção pública. | O runtime só usa motivo sustentável publicamente; o campo privado permanece no resultado do oráculo. |
| `M-ATT-06` | Reconhecer o ID ou nome da fixture e retornar `SUPPORTED` para o controle ou `ABSTAIN` para o negativo. | `VAR-01`: IDs, ações, chaves e ordem permutados. | A mesma propriedade é preservada em uma fixture renomeada; não há comparação com `m6a`, `m7a`, `m8a` ou `m9a`. |

O atestado que permite `SUPPORTED` deve possuir emissor independente identificável, versão do contrato, digest da fixture/observação coberta, escopo de efeitos, proveniência reproduzível e vínculo ao host, época e Skill. Um digest somente compromete bytes. Ele não demonstra completude, autenticidade, ausência de efeitos ocultos ou autoridade [2] [3].

## 5. Fixtures, oráculos e independência

A suíte deve reutilizar o holdout congelado apenas para regressão e criar um manifesto novo para mutações. Cada fixture nova terá três planos:

| Plano | Entrega | Proibição |
|---|---|---|
| Público | IDs opacos, observações, ações, contratos declarados, limites e recibos públicos | Estado verdadeiro, `expected_reason`, efeitos ocultos e rótulo da família |
| Privado do oráculo | Tabela declarativa de transições, efeitos ocultos, custos, pré-condições, recibos e veredicto | Importar runtime ou receber sua decisão para definir a verdade |
| Manifesto de avaliação | Seed, digests, split, classe esperada e resultado | Ser carregado pelo runtime durante a rodada |

As fixtures mínimas são:

- controle positivo com efeito completo declarado, custo exato finito, probe não mutante, observação atual e atestação externa válida;
- alias de duas ações com mesma projeção pública;
- conflito público com mesma ação e contexto, mas dois deltas incompatíveis;
- observação stale e digest inválido;
- campo requerido ausente ou `HIDDEN`;
- efeito oculto com mesmo delta público do controle;
- custo desconhecido em probe, reset e recovery;
- overrun depois de cotação que parecia caber;
- falha parcial e ausência de ACK no executor externo;
- proposta positiva sem autoridade.

A tabela privada deve ser avaliada por um oráculo que não importe `core.py`, `stage5.py`, `Effect.from_states()`, `_apply()`, `WorldModel` ou `Goal.satisfied()`. O teste de independência deve bloquear esses imports/chamadas e executar o oráculo em processo separado. A separação de processos é necessária para reduzir vazamento, mas não basta para provar independência se os processos compartilham chave, tabela, função de transição ou critério circular [3] [5].

O mesmo trace público pode ter duas verdades privadas. Nesse caso, a decisão do runtime deve ser idêntica para ambas. A distinção pode aparecer somente no `oracle_verdict` pós-decisão. Esta propriedade mata tanto a aceitação de digest sem atestação quanto a tentativa de ensinar o runtime a reconhecer o ID do contraexemplo.

## 6. Arquivos prováveis e desenho de implementação

Nenhum destes arquivos deve ser alterado nesta frente; a lista é um mapa para a implementação posterior.

| Arquivo provável | Responsabilidade | Invariante principal |
|---|---|---|
| `research/symbiont_v2/core.py` | Preservar wrappers legados e tornar o preflight estrito completo; registrar probes antes das chamadas; construir `TransferDecision`. | `propose_transfer_checked` não recebe executor, não assina e não executa o plano. |
| `research/symbiont_v2/stage5.py` | Tipos `TransferDecision`, contratos, `BudgetLimits`, ledger de proposta e estados de custo. | Custo desconhecido permanece `UNKNOWN`/`null`; snapshot não é confundido com ledger externo. |
| `research/holdout_hosts.py` ou `research/stage4/public_host.py` | Hosts públicos serializados, campos ausentes/stale/hidden e contadores de probe/reset. | Não expõe `_fixture`, `_private_state`, `true_effects`, callbacks ou tabelas privadas. |
| `research/stage4/contracts.py` | Compatibilidade do schema v1 e validação de resultados históricos. | Não reescrever os quinze registros nem usar booleano legado como verdade do ledger novo. |
| `research/stage4/execution_contracts.py` | Envelope externo, autoridade, recibos, nonce, TTL e canonicalização. | Digest sem raiz externa não é autorização. |
| `research/stage4/ledger.py` | Eventos append-only, cadeia de digests, monotonicidade e reserva durável de nonce. | `reset`, `bind`, reboot e recovery não diminuem contadores nem liberam replay. |
| `research/stage4/executor.py` | Validação por passo, revogação, stop, commit e falha parcial. | Só executa após envelope válido; ACK isolado não prova commit. |
| `research/stage4/recovery.py` | Reconciliação e `RecoveryAttestation`. | O caminho que perdeu ACK não pode atestar sozinho que nada ocorreu. |
| `research/stage4/runner.py` | Fechar runtime, depois oráculo, depois executor autorizado em testes separados. | O oráculo nunca guia a proposta em andamento. |
| `research/test_stage5_regression_mutations.py` | Regressões, fixtures renomeadas, propriedades e critérios de morte. | Testa efeitos, contadores e estados, não apenas motivos. |
| `research/stage4/test_protocol.py` | Isolamento e barreira proposta–execução. | `authorized_calls == 0` durante toda construção da proposta. |
| `research/stage4/test_executor.py` e `test_recovery.py` | Autoridade, custo, nonce, TTL, revogação, falha parcial, unknown e retry. | Resultado desconhecido ativa recuperação e bloqueia retry. |
| `tools/test_stage5_mutation_gates.py` | Harness de mutação em cópia temporária, análogo ao padrão de `tools/test_proof_fire_mutations.py`. | Cada substituição ocorre uma vez; mutante não compilável não conta como mutante morto. |

A implementação deve manter `transfer()` e `propose_transfer()` chamáveis. Esses wrappers históricos continuam sendo planos ou booleanos de compatibilidade. Eles não podem ser aceitos como envelope, autoridade ou prova de segurança. O modo estrito novo deve ser opt-in até a auditoria dos consumidores; sua adoção não deve alterar a semântica dos artefatos v1 [3] [9].

## 7. Critérios de aceitação

A frente só é aceita quando todos os critérios abaixo passam. Uma violação de separação, custo, autoridade ou resultado desconhecido é **hard failure** e não pode ser compensada por média positiva.

### 7.1 Regressão e determinismo

1. Os testes históricos do runtime e do holdout passam, os quinze registros permanecem byte a byte reproduzíveis e nenhum arquivo v1 é reescrito.
2. Cada fixture nova roda pelo menos três vezes em processos limpos, com mesmo manifesto, seed e relógio injetável. Status, motivos públicos, digests e contadores são iguais quando o trace público é igual.
3. IDs, nomes de ação, ordem de enumeração, chaves públicas e unidades são permutados. Nenhum código identifica `m6a`, `m7a`, `m8a`, `m9a` ou palavras como `hidden` e `budget`.
4. Cada mutante não equivalente é morto por pelo menos um teste focado e por uma variante renomeada. Mutante sobrevivente exige análise de equivalência observacional, não descarte informal.

### 7.2 Abstention e contrato

5. Alias, conflito, stale e digest inválido produzem `ABSTAIN`, proposta nula e zero executor.
6. Contrato ausente, campo requerido não observável, fechamento sem atestação ou limite de custo ausente produzem `UNSUPPORTED_BY_CONTRACT`, proposta nula e, quando possível, zero probes.
7. O teste de campo ausente demonstra que `MISSING` não vira zero, falso, vazio ou inalterado.
8. `SAFE_BUT_UNPROVEN` nunca entra no numerador de segurança e nunca é convertido em `SUPPORTED` por wrapper ou bool.

### 7.3 Custo e ledger

9. Em qualquer evento desconhecido, `cost_actual == null`, `cost_status == UNKNOWN`, `unknown_event_count > 0` e a operação que depende do custo não é aceita como `SUPPORTED`.
10. O custo de probe, reset, passo, retry e recovery é cobrado no início da tentativa. Exceção, timeout e ausência de ACK não removem a cobrança.
11. Limite ausente não equivale a infinito. `UPPER_BOUND` acima de `cost_max` bloqueia, e recibo real acima do limite gera `OVERRUN + COST_OVERRUN` e bloqueia novas operações.
12. `probe_count`, `reset_count`, `step_count`, `cost_count`, `retry_count`, `recovery_count` e `event_seq` são não negativos e monotônicos. `reset`, `bind`, reboot e recovery não os zeram.

### 7.4 Proposta e execução

13. Durante a construção da proposta, `proposal_execute_calls == 0`, `authorized_calls == 0`, `committed_calls == 0`, `external_effect_count == 0` e `step_count == 0`; `planned_steps` pode ser positivo.
14. Probe pública, executor autorizado e efeito externo têm contadores e canais distintos.
15. Proposta sem envelope produz `REJECTED + MISSING_AUTHORITY`, zero passos e zero efeitos. Digest, identidade, Skill verificada, nome da ação ou `transfer() == True` não substituem autoridade.
16. Falha explícita depois de commit produz `FAILED_PARTIAL + RECOVERY_REQUIRED`; timeout, crash, ACK ausente ou recibo conflitante produzem `UNKNOWN_OUTCOME + RECOVERY_REQUIRED`.
17. Não há retry automático. Novo envelope/nonce sem recuperação válida produz `RETRY_BLOCKED` e zero chamadas externas.

### 7.5 Atestação e independência

18. Um digest criado pelo runtime, mesmo correto, não habilita `SUPPORTED` sem atestação externa identificável e vinculada ao escopo correto.
19. `runtime_reason` nunca contém fato que só o oráculo privado conhece. `oracle_reason` permanece em campo separado.
20. O oráculo passa por teste de independência e não usa funções de transição, modelo ou objetivo do runtime para decidir a verdade privada.
21. O controle positivo passa apenas quando possui contrato completo, atestação externa, custo finito verificável e ação unívoca. Mesmo nesse caso, o resultado é plano suportado, não autorização ou execução.

Comandos esperados para a campanha futura:

```bash
PYTHONPATH=research python3 -m unittest research.test_holdout_adversarial -v
PYTHONPATH=research python3 -m unittest research.test_stage5_synthesis -v
PYTHONPATH=research python3 -m unittest research.test_stage5_regression_mutations -v
PYTHONPATH=research python3 -m unittest discover -s research -p 'test_*.py'
python3 tools/test_stage5_mutation_gates.py
```

## 8. Riscos de circularidade e controles

**Circularidade do delta.** Se runtime e oráculo usam `Effect.from_states()` ou uma mesma tabela de deltas, ambos podem concordar sobre uma projeção incompleta. O controle é uma tabela declarativa independente, com efeitos ocultos fora do host público e um teste de import proibido.

**Circularidade do digest.** SHA-256 detecta alteração dos bytes recebidos. Não prova que o host enviou todos os campos, que o campo oculto é benigno, que o emissor é independente ou que a autoridade é legítima. O atestado deve registrar emissor, chave/raiz, versão, escopo e proveniência. Um digest próprio do runtime recebe `OBSERVABILITY_CLOSURE_UNPROVEN`.

**Circularidade do oráculo.** Um oráculo em processo separado ainda pode compartilhar código, fixtures ou critérios com o runtime. O controle exige processo separado, tabela declarativa distinta, imports proibidos, digest de proveniência e variação de topologia/nomes. Independência é uma propriedade do caminho de decisão, não apenas do PID.

**Circularidade da autoridade.** Uma assinatura criada pelo runtime ou uma chave compartilhada não é autoridade externa. A autoridade deve ter emissor, raiz, epoch e escopo verificáveis. O executor deve rejeitar autoridade cuja proveniência não possa ser validada.

**Contador fictício.** Um contador publicado depois da chamada não prova que o orçamento foi aplicado. A cobrança deve ocorrer antes, inclusive em erro. O ledger precisa ser append-only e, quando usado para nonce ou execução, durável.

**Probe confundida com execução.** `discover()` chama `host.execute()` no baseline. Separar nomes de contadores não torna a probe não mutante. O modo estrito deve exigir `NON_MUTATING` ou autorização de sondagem específica; `reset()` posterior não é rollback nem recovery.

**Abstention universal.** Um sistema que sempre retorna `ABSTAIN` pode matar mutações por remoção de abstention e ainda não demonstrar competência. O controle positivo com contrato externo válido deve construir uma proposta sem executar. Ele não compensa qualquer hard failure negativo.

**Sobreajuste ao holdout.** Testar somente os IDs conhecidos permite uma função especial que reconhece `m6a` ou `m8a`. IDs opacos, permutações, seeds novas e fixtures com a mesma propriedade causal impedem essa forma de sobreajuste.

**Compatibilidade permissiva.** Consumidores que interpretam `True` ou proposta não nula como comando podem reintroduzir autoridade por fora do runtime. Documentar depreciação, registrar telemetria e exigir o executor externo para efeitos. Não mudar silenciosamente o tipo dos wrappers históricos.

## 9. O que não pode ser alegado

Esta frente não permite alegar que uma proposta construída, `SUPPORTED`, digest válido ou Skill verificada possui autoridade física. Não permite alegar que digest prova verdade do host, completude da observação, ausência de efeito oculto ou independência do emissor. Não permite alegar que custo desconhecido é baixo, que nonce prova idempotência, que TTL prova ausência de efeito, que revogação desfaz commit, que `reset()` recupera estado parcial ou que `expected_final` é recibo do mundo.

Uma aprovação do controle positivo demonstra, no máximo, que um mecanismo host-only delimitado satisfez um contrato de teste específico. Uma campanha com 100% de mutantes mortos cobre somente os mutantes definidos e observáveis; não prova ausência de todos os bugs, segurança geral ou generalização fora das fixtures.

A abstenção não demonstra que o runtime identificou a causa privada. `UNSUPPORTED_BY_CONTRACT` significa que a alegação solicitada não é permitida pelo contrato. Em particular, o runtime não pode afirmar `HIDDEN_EFFECT_UNVERIFIABLE` quando o host não declarou o campo; deve usar uma razão pública como `OBSERVABILITY_CLOSURE_UNPROVEN`.

Nada aqui demonstra simbiose útil, benefício humano, redução de erro com usuários reais, privacidade de produto, acessibilidade, segurança física ou operacional geral, robustez robótica, consciência ou inteligência geral. Essas alegações exigem tarefa humana identificável, baseline convencional, condição sem simbionte e medições próprias. Se qualquer hard limit mecânico falhar, a classificação correta é `not_proven`, não uma média positiva [1] [10].

## Referências

[1]: ../../../docs/53-BENCHMARK-HOLDOUT-ADVERSARIAL.md "Etapa 4 — benchmark holdout adversarial"
[2]: ../../../docs/54-CORRECAO-CONTRATO-OBSERVABILIDADE.md "Etapa 5 — contrato de observabilidade e decisão checked"
[3]: ../stage5_plan/05-regression_and_mutations.md "Stage 5 — testes de regressão e mutações"
[4]: ../stage5_plan/04-budget_cost_model.md "Stage 5 — modelo de custo e orçamento"
[5]: ../stage5_plan/03-partial_failure_executor.md "Stage 5 — envelope externo, executor e falha parcial"
[6]: ../stage5_plan/02-hidden_effect_containment.md "Stage 5 — contenção de efeitos ocultos"
[7]: ../../symbiont_v2/core.py "HERUS Symbiont v2 research runtime"
[8]: ../../symbiont_v2/stage5.py "Tipos Stage 5 de decisão e ledger de orçamento"
[9]: ../remaining_plan/01-external_executor.md "HERUS — plano restante: executor externo e recuperação"
[10]: ../remaining_plan/02-extended_holdout.md "HERUS — plano de holdout temporal e estrutural estendido"

<!-- Relatório produzido como desenho implementável. Nenhum arquivo de código foi alterado nesta frente. -->
