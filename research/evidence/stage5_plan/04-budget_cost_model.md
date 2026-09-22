# Stage 5 — modelo de custo e orçamento

**ID:** `HERUS-S5-BCM-004`  
**Frente:** modelo de custo e orçamento  
**Status:** proposta de desenho; nenhuma alteração de código foi feita  
**Escopo:** contabilidade monotônica de sondas, resets, passos, custo, retry e recuperação em uma API host-only. A proposta preserva a separação entre proposta e execução, é compatível com a API black-box atual e não mede nem alega utilidade social, acessibilidade real, segurança física, consciência ou inteligência geral.

## 1. Decisão executiva

O orçamento deve ser um **livro-caixa de eventos append-only**, associado a um escopo explícito de operação. Cada evento é registrado antes ou no início da chamada que pode consumir recurso. Os contadores nunca diminuem, não são zerados por `reset()`, `bind()`, retry ou recuperação e não são recalculados a partir do estado privado do host. A proposta e a execução continuam sendo operações distintas: a proposta pode consumir sondas, resets e custo de descoberta, mas mantém `proposal_execute_calls == 0` e `step_count == 0`; a execução autorizada por uma autoridade externa continua em outro caminho, usando o mesmo livro-caixa ou uma transferência encadeada dele.

O limite canônico será `cost_max`, um inteiro não negativo que limita o custo total do escopo. O campo legado `max_cost` continua aceito como alias durante a migração. Ausência de `cost_max` não significa orçamento infinito, e custo não medido não significa custo zero. Quando pelo menos um evento não tiver medida exata nem limite superior finito atestado, o total serializado será `null`, o estado de custo será `UNKNOWN` e o caminho estrito bloqueará a decisão com `UNKNOWN_COST` ou `COST_LIMIT_MISSING`. O caminho de compatibilidade poderá conservar um plano público, mas somente como `SAFE_BUT_UNPROVEN`; nunca como `SUPPORTED` ou autorização.

O caso atual `m8a-budget-01` ilustra a lacuna: o harness calcula `cost_actual` depois da proposta como `probe_count * fixture.cost`, mas o runtime não recebe nem aplica esse limite. A frente deve transformar o custo em uma condição de decisão observável e versionada, sem reescrever os quinze registros congelados da Etapa 4. O resultado da frente continua sendo **mecanismo host-only** ou **`not_proven`**, conforme os gates. Não há qualquer conclusão sobre valor social.

## 2. Diagnóstico da API atual

A implementação atual possui limites parciais, mas eles não formam ainda um modelo único:

1. `DiscoveryBudget` em `symbiont_v2/core.py` limita `max_probes` e `max_candidates`. Ele não contém custo, resets, passos de execução, retry ou recuperação.
2. `research/stage4/contracts.py` já define `Budget(max_probes, max_steps, max_cost)`, mas esse objeto não é ligado ao ciclo de descoberta, à proposta ou a um executor. Ele também não diferencia custo desconhecido, limite ausente, overrun e exaustão antes da próxima chamada.
3. `PublicHoldoutHost` conta `probe_count` quando `execute()` é chamado e `reset_count` quando `reset()` retorna. Esses campos pertencem ao host de teste e não formam um recibo confiável de que o runtime tenha aplicado o orçamento. Um erro ou uma chamada que não retorna ainda pode ter consumido recurso; a contabilidade deve cobrar a tentativa antes da chamada.
4. `holdout_adversarial.py` mantém a barreira proposta/execução, mas calcula `cost_actual` com a verdade privada da fixture depois que o runtime termina. Isso é uma métrica do harness, não uma verificação de que a proposta conhecia ou respeitou `cost_max`. O custo de `reset()` também é implicitamente ignorado.
5. `TransferProposal` não contém limites, recibo de contagem, custo ou escopo de orçamento. `propose_transfer()` aceita `max_probes`, mas a descoberta usa primeiro `self.budget.max_probes`; o parâmetro da chamada só fatia as evidências depois da descoberta. Portanto, não se deve alegar que o parâmetro atual já limita todas as sondas.
6. O validador Stage 4 só rejeita contadores negativos, execução vazada, efeito externo em fixture negativa e proposta marcada depois de `budget_exhausted`. Ele não consegue provar monotonicidade, custo conhecido, cobrança de retries ou recuperação.

A linha de base continua válida. No holdout atual, o controle usa uma sonda e um reset, o alias usa duas sondas e dois resets, os casos opaco e parcial usam uma sonda e um reset, e o caso de orçamento reporta uma sonda, um reset e `cost_actual == 2`. Esses números devem continuar reproduzíveis no schema v1. No schema novo, eles devem ser interpretados como evidência de contagem do harness, não como prova de que o runtime já aplicou `cost_max`.

## 3. Modelo proposto

### 3.1 Limites versionados

A nova camada deve definir um objeto aditivo, por exemplo `BudgetLimits`, sem substituir os tipos legados durante a migração:

```text
BudgetLimits {
    probe_max: uint
    reset_max: uint
    step_max: uint
    cost_max: uint
    retry_max: uint = 0
    recovery_max: uint = 0
}
```

A serialização canônica pode preservar os nomes legados para os três campos existentes e introduzir `cost_max` como nome oficial da nova interface:

| Nome canônico na API nova | Alias aceito na migração | Semântica |
|---|---|---|
| `probe_max` | `max_probes` | Número máximo de tentativas de sondagem host-only no escopo. |
| `step_max` | `max_steps` | Número máximo de passos planejados e, na execução, de tentativas de passo. |
| `cost_max` | `max_cost` | Soma máxima de unidades de custo medidas ou de um limite superior atestado. |
| `reset_max` | nenhum | Número máximo de chamadas de reset cobradas. |
| `retry_max` | nenhum | Número máximo de re-tentativas, sem contar a tentativa inicial. |
| `recovery_max` | nenhum | Número máximo de operações explícitas de recuperação. |

Todos os limites devem ser inteiros não negativos. Booleanos, números fracionários, negativos e valores ausentes em modo estrito devem ser rejeitados, em vez de convertidos silenciosamente. `None` não deve representar “ilimitado” para `cost_max`. Se um produto realmente quiser uma execução sem limite, isso deve ser um modo de pesquisa explicitamente nomeado e não pode ser tratado como prova de segurança ou como `SUPPORTED`.

`max_candidates` permanece um limite de busca separado. Ele não substitui `step_max` nem `cost_max`. A chamada nova deve calcular o limite efetivo de sondas antes da primeira chamada como o mínimo entre o limite do escopo, o limite do runtime e o limite explícito da chamada. A API legada conserva sua semântica atual para não quebrar consumidores; a telemetria deve revelar que seu `max_probes` não era, até esta mudança, um gate de todas as chamadas.

### 3.2 Livro-caixa e contadores monotônicos

Cada ciclo de proposta, execução e recuperação recebe um `budget_scope_id`. Para um ciclo de transferência completo, o mesmo escopo deve ser encadeado da proposta para o executor externo. Um novo `bind()` ou `reset()` não cria uma oportunidade de zerar o orçamento. Se uma campanha quiser medir operações independentes, ela cria novos escopos com IDs distintos e registra o digest do escopo-pai; a agregação da campanha é feita por soma de eventos, não por reutilização de um contador zerado.

O estado serializado recomendado é:

```text
BudgetLedger {
    schema: "herus-budget-ledger-v1",
    budget_scope_id: string,
    parent_ledger_digest: string | null,
    limits: {
        probe_max: uint,
        reset_max: uint,
        step_max: uint,
        cost_max: uint,
        retry_max: uint,
        recovery_max: uint
    },
    counters: {
        probe_count: uint,
        reset_count: uint,
        step_count: uint,
        cost_count: uint,
        retry_count: uint,
        recovery_count: uint
    },
    cost: {
        status: EXACT | BOUNDED_UNKNOWN | UNKNOWN | OVERRUN,
        actual: uint | null,
        upper_bound: uint | null,
        measured_event_count: uint,
        unknown_event_count: uint
    },
    budget_state: OPEN | EXHAUSTED | OVERRUN | UNKNOWN,
    event_seq: uint,
    ledger_digest: string
}
```

`event_seq` e todos os contadores são monotônicos dentro do escopo. O evento deve ser append-only e incluir ao menos `event_seq`, fase (`PROPOSAL`, `EXECUTION` ou `RECOVERY`), tipo, operação lógica, incremento, recibo de custo e digest do traço público. A atualização precisa ser atômica ou rejeitar uma revisão velha; uma gravação concorrente não pode diminuir o valor já persistido.

A semântica dos seis contadores é deliberadamente estreita:

| Contador | Incremento | Não significa |
|---|---|---|
| `probe_count` | Uma tentativa de chamar o mecanismo público usado para descobrir uma transição. Cobra a tentativa antes de `host.execute()`, inclusive quando a chamada falha ou não retorna. | Execução autorizada do plano ou ausência de efeito lateral da sonda. |
| `reset_count` | Uma chamada de `host.reset()`, cobrada no início da chamada. Um reset que lança exceção continua contado. | Prova de que o estado foi restaurado ou operação de recuperação. |
| `step_count` | Uma tentativa de um passo do plano no executor autorizado. Durante a proposta, permanece zero; o plano usa `planned_steps`. | Número de probes ou candidatos inspecionados. |
| `cost_count` | Um evento que deveria ter uma medição ou um limite superior de custo, independentemente de o valor ser conhecido. | Soma monetária ou soma de unidades. |
| `retry_count` | Cada nova tentativa explícita da mesma operação lógica depois da tentativa inicial. Não há retry automático. | Uma segunda sonda independente ou uma chamada de recuperação. |
| `recovery_count` | Cada operação explícita de reconciliação, isolamento ou recuperação após estado parcial/desconhecido. | Um reset comum ou uma repetição do passo. |

O custo possui duas dimensões que não podem ser confundidas. `cost_count` é sempre monotônico e conta eventos de cobrança. `cost.actual` é a soma somente quando **todos** os eventos têm quantidade exata medida. Assim, um evento sem medida gera `unknown_event_count += 1`, mantém `cost_count` crescente e torna `actual == null`; ele nunca contribui como zero. Um valor zero é permitido apenas quando um recibo declara explicitamente `MEASURED_ZERO`, por exemplo, custo medido de reset igual a zero. A ausência de um campo não pode ser interpretada como esse recibo.

Um contrato pode fornecer um limite superior finito sem fornecer o valor exato. Nesse caso, o estado é `BOUNDED_UNKNOWN`, `actual == null` e `upper_bound` contém o limite atestado. O gate de orçamento pode aceitar a operação quando o limite superior acumulado não excede `cost_max`, mas o resultado deve continuar declarando que o custo real não foi medido. Se não houver valor exato nem limite superior finito, o estado é `UNKNOWN` e o modo estrito não inicia uma operação que dependa desse custo.

O custo total do escopo inclui sondas, resets, passos, retries e recuperações. Nenhuma dessas fases ganha custo gratuito. O contrato opcional do host deve fornecer uma cotação ou recibo por tipo de evento; se o custo do reset ou da recuperação não for publicado, ele é `UNKNOWN`, mesmo que o custo da sonda seja conhecido.

### 3.3 Regras de cobrança e exaustão

A ordem de avaliação deve ser fixa:

1. Validar limites, escopo, versão e digest do ledger.
2. Validar o contrato de custo público para a próxima operação. Uma cotação exata ou um limite superior deve ser recebido antes de uma chamada potencialmente mutante.
3. Verificar se o contador da dimensão e o custo reservado ainda cabem no limite. Se não couberem, não fazer a chamada e emitir `BUDGET_EXHAUSTED` ou `COST_OVERRUN` conforme o caso.
4. Registrar o evento e cobrar o contador antes de invocar o host ou o executor.
5. Anexar o recibo retornado. Se o valor efetivo exceder a previsão, marcar `OVERRUN`, parar novas operações e preservar o valor real conhecido.
6. Se o recibo não existir ou for inválido, marcar `UNKNOWN`; não preencher `0`, não aceitar a proposta e não fazer retry automático.

`BUDGET_EXHAUSTED` significa que a próxima chamada conhecida não pode começar porque uma dimensão atingiu seu limite. `COST_OVERRUN` significa que uma cobrança conhecida, já realizada ou observada como inevitável, excede `cost_max`. Se a chamada já começou e o recibo ficou incompleto, o estado é `UNKNOWN`, podendo também exigir `RECOVERY_REQUIRED`; ele não deve ser classificado como “dentro do orçamento” só porque não há um número final.

O campo legado `budget_exhausted` deve permanecer nos registros Stage 4 sem alteração. No schema Stage 5, a fonte de verdade deve ser `budget_state`; o booleano legado pode ser emitido apenas como projeção para estados conhecidos (`EXHAUSTED` ou `OVERRUN`). `UNKNOWN` não é `false` orçamentariamente: significa que a pergunta não pôde ser respondida. Consumidores novos devem usar `budget_state` e `cost.status`, não o booleano isolado.

O `step_max` é aplicado em dois momentos. Antes de retornar uma proposta, `planned_steps = len(proposal.actions)` deve caber no limite; caso contrário, a proposta é bloqueada sem execução e com `BUDGET_EXHAUSTED`. Na execução externa, cada tentativa de passo incrementa `step_count`, inclusive retry. Logo, `planned_steps` pode ser maior que zero enquanto `step_count` permanece zero na decisão de proposta; isso é intencional e mantém a fronteira proposta/execução.

### 3.4 Retry, recuperação e reset

A tentativa inicial não incrementa `retry_count`. Uma nova tentativa só pode ser iniciada por um chamador externo que apresente a mesma operação lógica, o digest do ledger e uma razão. `retry_max` default igual a zero conserva o comportamento atual de não fazer retry automático. Quando a saída é `FAILED_PARTIAL` ou `UNKNOWN_OUTCOME`, um retry direto é proibido até que haja recibo independente de recuperação ou reconciliação. Se não houver orçamento de retry, a saída adicional é `RETRY_BLOCKED`.

`recovery_count` só cresce quando é chamado um protocolo explícito de recuperação. O reset automático usado depois de uma probe em `discover()` continua sendo `reset_count`; ele não é contado como recuperação e não prova que efeitos ocultos desapareceram. Se `reset()` for necessário para recuperar um estado parcial, deve haver também um evento de recuperação e um atestado independente de que o estado é novamente conhecido. Sem esse atestado, o estado continua `RECOVERY_REQUIRED`.

Uma recuperação consome seus próprios passos, resets e custo. A recuperação não devolve orçamento, não reduz `step_count`, não reduz `cost_count` e não autoriza uma nova proposta por si só. Se uma recuperação falhar ou não produzir ACK, o estado fica `UNKNOWN_OUTCOME`/`RECOVERY_REQUIRED`, e o ledger permanece fechado para retry.

## 4. API black-box compatível e separação de execução

A interface obrigatória `HostAdapter` não deve receber métodos obrigatórios novos. Propõe-se um protocolo opcional, por exemplo `OptionalBudgetHost`, que exponha somente um contrato público de cotação e cobrança:

```text
BudgetContract {
    schema: "herus-budget-contract-v1",
    cost_quote(operation, action, phase):
        EXACT(amount, receipt_digest)
        | UPPER_BOUND(amount, receipt_digest)
        | UNKNOWN(reason),
    probe_mode: NON_MUTATING | MUTATING_AUTHORIZED | UNKNOWN,
    reset_cost: EXACT | UPPER_BOUND | UNKNOWN,
    recovery_cost: EXACT | UPPER_BOUND | UNKNOWN,
    contract_digest: string,
    provenance: string
}
```

A ausência desse protocolo não deve provocar introspecção em `_fixture`, `__dict__`, `true_effects`, `effects_by_name` ou qualquer estado privado. No modo estrito, ela produz `COST_LIMIT_MISSING` quando o limite não foi fornecido ou `UNKNOWN_COST` quando a próxima cobrança não pode ser medida. No modo de compatibilidade, o caminho legado pode continuar chamável e produzir o plano que a API atual produz, mas o auditor deve anexar `cost.status == UNKNOWN` ou `BOUNDED_UNKNOWN`, `cost.actual == null` se não houver soma exata e `safety_claim == SAFE_BUT_UNPROVEN`.

A operação nova recomendada é aditiva:

```text
propose_transfer_checked(
    skill_id,
    host,
    *,
    budget: BudgetLimits,
    max_probes: int = 64,
    mode: STRICT | COMPATIBILITY = STRICT,
    budget_scope_id: str | None = None
) -> TransferDecision
```

`TransferDecision` deve conter `proposal`, `proposal_status`, `safety_claim`, `runtime_reason`, `budget_ledger`, `planned_steps`, `proposal_execute_calls` e `mode`. Durante qualquer variante de `propose_transfer_checked()`, `proposal_execute_calls` deve ser zero. `TransferProposal` continua sendo somente um plano com `actions`, `expected_final` e digests de evidência; não recebe autoridade implícita por carregar um ledger.

A API antiga `propose_transfer()` e o wrapper `transfer()` permanecem com suas assinaturas e resultados atuais na Fase A. Um `TransferProposal` devolvido por elas nunca deve ser interpretado como autorização ou como prova de orçamento. O executor externo recebe uma proposta autorizada por outra camada, um `budget_scope_id` e o digest do ledger. Ele continua fora do runtime de proposta. A execução pode emitir `FAILED_PARTIAL`, `UNKNOWN_OUTCOME` ou `RECOVERY_REQUIRED`, mas não deve retroativamente mudar uma proposta antiga para sucesso.

O oráculo independente deve manter sua própria contabilidade declarativa. Ele pode comparar o trace do executor com o ledger público, mas não deve importar `core.py`, usar `Effect.from_states()`, `WorldModel`, `_apply()` ou `Goal.satisfied()`. A contabilidade do runtime não pode ser a única fonte da verdade que ela mesma pretende demonstrar.

## 5. Estados e motivos

Os estados de proposta e de orçamento são eixos separados. `PROPOSED` não significa execução, segurança ou custo dentro do limite. `SAFE_BUT_UNPROVEN` não é autorização. `FAIL_UNSAFE` e o veredito do oráculo permanecem separados do motivo que o runtime consegue sustentar publicamente.

| Eixo | Estado | Regra |
|---|---|---|
| Proposta | `PROPOSED` | Plano construído; nenhuma ação de execução foi chamada. Só é possível alegação `SUPPORTED` se todos os contratos independentes, incluindo orçamento, passarem. |
| Proposta | `ABSTAIN` | Alias, conflito, stale, digest inválido, limite atingido antes da chamada ou resultado insuficiente para decidir. `proposal` é nulo no modo estrito. |
| Proposta | `UNSUPPORTED_BY_CONTRACT` | Falta contrato de custo, limite, cobertura de sondagem ou fechamento exigido. `proposal` é nulo no modo estrito. |
| Execução | `FAILED_PARTIAL` | Houve efeito ou ACK parcial. Não vira sucesso e não permite retry direto. |
| Execução | `UNKNOWN_OUTCOME` | Não foi possível saber o resultado ou custo final. A saída exige reconciliação. |
| Execução | `RECOVERY_REQUIRED` | Existe estado parcial/desconhecido e uma recuperação explícita é pré-condição para continuar. |
| Ledger | `OPEN` | Todos os limites relevantes são conhecidos e a próxima operação pode caber. |
| Ledger | `EXHAUSTED` | Uma dimensão conhecida não tem capacidade para a próxima operação. |
| Ledger | `OVERRUN` | Custo conhecido excedeu `cost_max`; nenhuma continuação automática. |
| Ledger | `UNKNOWN` | Custo ou limite não pode ser estabelecido. Não equivale a `OPEN`. |

Motivos públicos aditivos e suas regras:

| Motivo | Condição sustentada pelo runtime | Resultado estrito |
|---|---|---|
| `UNKNOWN_COST` | Evento sem medida exata nem limite superior finito, ou recibo inválido. | `UNSUPPORTED_BY_CONTRACT` ou `ABSTAIN`; `cost.actual == null`. |
| `COST_LIMIT_MISSING` | `cost_max` não foi fornecido para o gate estrito. | `UNSUPPORTED_BY_CONTRACT`; não tratar como infinito. |
| `COST_OVERRUN` | Cobrança conhecida excede `cost_max`, inclusive após uma cotação que se mostrou baixa. | `ABSTAIN`; `budget_state == OVERRUN`. |
| `BUDGET_EXHAUSTED` | A próxima probe, reset, passo, retry ou recuperação não cabe no limite antes de começar. | `ABSTAIN`; nenhuma chamada adicional. |
| `RETRY_BLOCKED` | Retry solicitado sem quota, sem recuperação prévia ou com resultado ainda desconhecido. | `UNKNOWN_OUTCOME` ou `RECOVERY_REQUIRED`; sem retry automático. |
| `PARTIAL_FAILURE` | A execução ou sonda autorizada terminou parcialmente. | Preservar `FAILED_PARTIAL`; não promover. |
| `UNKNOWN_OUTCOME` | ACK, custo ou efeito final ausente. | Preservar estado desconhecido; exigir recuperação. |
| `RECOVERY_REQUIRED` | O estado não pode ser retomado sem reconciliação. | Bloquear novas tentativas. |
| `SAFE_BUT_UNPROVEN` | Plano público existe, mas custo, fechamento ou contrato não provam a alegação solicitada. | Somente compatibilidade; nunca `SUPPORTED`. |
| `SUPPORTED_CONTROL` | Contratos, cotas, cotação/limite de custo, fechamento independente e observações passam. | Pode classificar o plano no eixo epistemológico, ainda sem autorização de execução. |

Quando o custo é desconhecido, o runtime deve usar `UNKNOWN_COST`, não `COST_OVERRUN`, a menos que exista evidência de que houve uma cobrança acima do limite. Quando o custo é conhecido e a capacidade já acabou, deve usar `BUDGET_EXHAUSTED`. Quando o resultado de uma chamada não é conhecido, deve usar `UNKNOWN_OUTCOME`/`RECOVERY_REQUIRED`, não `RETRY` silencioso. O motivo privado do oráculo, como `HIDDEN_EFFECT_UNVERIFIABLE`, não deve ser copiado para `runtime_reason` se não estiver na observação pública.

## 6. Migração compatível

### Fase A — ledger somente leitura e telemetria

Adicionar um módulo de ledger e um schema Stage 5 sem editar `stage4/contracts.py`, `holdout_benchmark_schema.json` ou `holdout_benchmark_v1.json`. O wrapper pode observar as chamadas públicas existentes e gerar eventos de probe e reset. Para custos, deve distinguir `EXACT`, `UPPER_BOUND` e `UNKNOWN`; não deve preencher o campo legado com zero quando não houver recibo.

`DiscoveryBudget` permanece construível com os argumentos atuais. `Budget.max_cost` permanece aceito na camada Stage 4 e é mapeado para `cost_max` somente no adaptador novo. A nova API exige `BudgetLimits` completo em modo estrito, mas seus campos adicionais têm defaults seguros (`retry_max = 0`, `recovery_max = 0`) para conservar a ausência de retry automático. Registros v1 permanecem exatamente como estão; registros v2 recebem `budget_ledger`, `cost_status`, `cost_actual` anulável, `planned_steps`, `step_count`, `retry_count`, `recovery_count`, `runtime_reason` e `oracle_reason` separados.

A validação v1 continua tratando `cost_actual` como inteiro. A validação v2 precisa ser distinta e aceitar `cost_actual: null` somente quando `cost_status` for `UNKNOWN` ou `BOUNDED_UNKNOWN`, com `unknown_event_count > 0` no primeiro caso. Não se deve mudar o comparador v1 para aceitar `null`, pois isso quebraria a garantia histórica sem fornecer um esquema substituto.

### Fase B — hosts e Skills versionados

Hosts novos implementam o protocolo opcional de custo e declaram se probes são não mutantes, mutantes autorizadas ou desconhecidas. A declaração é um compromisso de contrato, não prova física por si só; a campanha deve registrar proveniência e recibo independente. Skills novas carregam o orçamento exigido ou recebem orçamento do chamador, além do contrato de observabilidade já proposto. Skills antigas são classificadas como `legacy-budget-unknown` no caminho estrito.

O custo de reset deve ser declarado explicitamente. Para reproduzir os números antigos sem assumir que reset é gratuito, a fixture precisa publicar um recibo `MEASURED_ZERO` ou um valor próprio para reset. Caso contrário, a campanha Stage 5 deve registrar custo desconhecido, mesmo que `fixture.cost` continue representando o custo da probe.

### Fase C — adoção estrita

Novos consumidores usam `propose_transfer_checked(..., mode="STRICT")`. O wrapper legado recebe telemetria de uso, documentação de depreciação e uma asserção de que seu retorno não autoriza execução. Depois de auditar consumidores, o wrapper poderá delegar ao gate estrito, mas a assinatura e o tipo de retorno histórico não devem ser alterados sem uma nova versão de API.

A proposta e a execução compartilham o ledger por digest e `budget_scope_id`, mas continuam endpoints e autoridades separados. Um executor não pode iniciar com um contador menor que o snapshot da proposta. O escopo não pode ser reiniciado para transformar retry ou recuperação em orçamento novo. A campanha Stage 5 deve ser comparada ao baseline v1 e ter digest próprio.

## 7. Testes obrigatórios

A regressão mínima permanece:

```bash
PYTHONPATH=research python3 -m unittest research.test_holdout_adversarial -v
PYTHONPATH=research python3 -m unittest discover -s research -p 'test_*.py'
```

A nova suíte deve cobrir os casos abaixo.

| Grupo | Teste | Critério de aceitação |
|---|---|---|
| Baseline | Reexecutar as 15 linhas do holdout v1 | Os arquivos v1 e sua classificação permanecem inalterados; nenhum teste novo reescreve `cost_actual`. |
| Limites | `probe_max`, `reset_max`, `step_max`, `retry_max` e `recovery_max` em zero e no limite exato | A chamada que não cabe não ocorre; contadores não ultrapassam limites; motivo é `BUDGET_EXHAUSTED`. |
| Probes | Falha, timeout e exceção de `host.execute()` | `probe_count` é cobrado antes da chamada e permanece monotônico; resultado não assume que a chamada não consumiu recurso. |
| Resets | `reset()` bem-sucedido, falho e sem ACK | `reset_count` cresce em todos os casos; reset não é contado como recovery e não prova restauração. |
| Passos | Proposta com `planned_steps == step_max`, acima do limite e execução autorizada | Igual ao limite é permitido pelo gate; acima é bloqueado; proposta mantém `step_count == 0`; executor cobra cada tentativa. |
| Custo exato | Custos 0, 1 e `cost_max` explicitamente medidos | Zero só passa com recibo explícito `MEASURED_ZERO`; soma igual ao limite cabe; soma acima gera `COST_OVERRUN`. |
| Custo desconhecido | Campo de custo ausente, `UNKNOWN` e recibo inválido | `cost_count` e `unknown_event_count` crescem; `cost.actual` é `null`, nunca 0; modo estrito bloqueia. |
| Limite ausente | Sem `cost_max`, com `max_cost` legado e com ambos conflitantes | Ausência estrita gera `COST_LIMIT_MISSING`; alias compatível funciona; valores conflitantes são rejeitados. |
| Limite superior | Cotação `UPPER_BOUND` menor, igual e maior que `cost_max` | `actual` permanece `null`; somente o limite superior pode sustentar a decisão orçamentária; acima bloqueia. |
| Pós-cobrança | Cotação baixa e recibo real acima do limite | Ledger termina `OVERRUN`; novas operações são bloqueadas e o evento conserva o custo conhecido. |
| Retry | Falha parcial, retry permitido, retry sem quota e retry sem recuperação | A tentativa inicial não conta como retry; cada retry é cobrado; retry sem recuperação ou quota retorna `RETRY_BLOCKED`. |
| Recuperação | Recuperação bem-sucedida, falha de recuperação e reset usado como falsa recuperação | `recovery_count` só cresce em operação explícita; falha mantém `RECOVERY_REQUIRED`; reset não devolve orçamento. |
| Monotonicidade | `reset()`, `bind()`, snapshot velho, restauração concorrente e escopo filho | Nenhum contador diminui ou é zerado; snapshot velho é rejeitado; escopo filho referencia o pai. |
| Separação | Chamar qualquer variante de proposta com executor instrumentado | `proposal_execute_calls == 0`, `step_count == 0` e nenhum efeito externo da proposta. |
| Oráculo | Oráculo com custo e transições declarativos, separado do runtime | O oracle usa seu próprio trace e ledger; não importa nem chama lógica de transição do runtime. |
| Compatibilidade | Host antigo, `Budget` v1 e `propose_transfer()` legada | Assinaturas e quinze registros v1 continuam válidos; caminho estrito identifica custo desconhecido. |
| Determinismo | Três repetições, permutação de IDs/ordem e mesmo trace público com custo privado diferente | Razões, estados e limites são reproduzíveis; custo privado não é inferido do ID da fixture. |

### 7.1 Propriedades invariantes

Além dos exemplos, a suíte deve testar propriedades sobre uma sequência de eventos gerada pelo harness. Para cada prefixo `p` do trace, todos os contadores de `p` devem ser menores ou iguais aos do prefixo seguinte. A soma de custos exatos deve coincidir com `actual` somente quando `unknown_event_count == 0`; qualquer evento desconhecido deve manter `actual == null`. Uma exceção depois da cobrança não pode fazer o contador retornar ao valor anterior. Um `reset()` não pode diminuir `probe_count`, `step_count`, `cost_count`, `retry_count` ou `recovery_count`. Um retry não pode ocorrer sem um evento inicial com a mesma operação lógica. Uma proposta não pode conter `step_count > 0` nem `proposal_execute_calls > 0`.

O teste de reconciliação deve comparar três visões: eventos públicos emitidos pelo runtime, recibos públicos do host/executor e traço declarativo do oráculo. Divergência não deve ser resolvida por maioria. Ela deve produzir `UNKNOWN_OUTCOME`, `UNKNOWN_COST` ou `RECOVERY_REQUIRED`, conforme a dimensão afetada. A ausência de um evento não deve ser tratada como evento com incremento zero.

### 7.2 Mutações que devem falhar

A campanha de mutação deve aplicar cada alteração abaixo e exigir que pelo menos um teste específico falhe:

1. Iniciar a cobrança depois de `host.execute()` ou `reset()` em vez de antes.
2. Zerar contadores em `reset()`, `bind()`, retry ou recuperação.
3. Tratar custo ausente, `None`, timeout ou recibo inválido como `0`.
4. Aceitar `cost_max` ausente como infinito ou aceitar `max_cost` e `cost_max` conflitantes por ordem de precedência implícita.
5. Omitir `reset`, retry ou recuperação da soma de custo.
6. Contar a tentativa inicial como retry ou contar retry como recovery sem operação de recuperação.
7. Permitir nova tentativa depois de `UNKNOWN_OUTCOME` sem recuperação e sem quota.
8. Fazer a proposta incrementar `step_count` ou chamar o executor externo.
9. Aceitar uma cotação `UPPER_BOUND` acima do limite, ou aceitar recibo real acima da cotação sem marcar `OVERRUN`.
10. Reaplicar um snapshot velho que diminua `event_seq` ou qualquer contador.
11. Copiar `oracle_reason` para `runtime_reason`, consultar `true_effects` para obter custo ou derivar custo de um digest produzido pelo próprio runtime.
12. Codificar `m8a-budget-01` ou `fixture.cost` como regra especial em vez de exercitar o contrato genérico.
13. Converter `SAFE_BUT_UNPROVEN`, `UNKNOWN_COST` ou `UNKNOWN_OUTCOME` em `SUPPORTED`.

Uma mutação importante é manter o mesmo delta público e mudar somente o custo privado da fixture. Sem contrato de custo público, a saída do runtime deve continuar sendo `UNKNOWN_COST`/`COST_LIMIT_MISSING`; ela não pode variar para `SUPPORTED` porque o oráculo sabe um número que o runtime não viu. Outra mutação deve trocar a ordem de dois eventos de custo com o mesmo total. O digest do ledger deve mudar quando a ordem fizer parte do trace, mas os contadores finais só podem coincidir se ambos os eventos forem válidos e conhecidos.

## 8. Critérios de aceitação da frente

A frente passa somente se os seguintes invariantes forem demonstrados em host black-box:

1. `probe_count`, `reset_count`, `step_count`, `cost_count`, `retry_count` e `recovery_count` são não negativos, monotônicos e nunca são zerados por mudança de sessão, reset, retry ou recuperação.
2. Cada tentativa potencialmente consumidora é cobrada antes da chamada, inclusive quando falha, expira ou não fornece ACK.
3. `cost_max` é aplicado antes de iniciar cada operação que tenha cotação verificável; overrun conhecido fecha o ledger e custo desconhecido nunca vira zero.
4. `cost.actual == null` quando houver qualquer evento sem medida exata; `actual` só é numérico quando todos os eventos foram medidos ou um recibo `MEASURED_ZERO` sustenta um custo zero.
5. Limite ausente, custo desconhecido, exaustão, overrun, falha parcial, resultado desconhecido e recuperação exigida preservam motivos distintos e não são convertidos em sucesso.
6. A proposta pode cobrar probes e resets de descoberta, mas mantém `proposal_execute_calls == 0` e `step_count == 0`; execução autorizada continua fora de `propose_transfer_checked()`.
7. Retry não é automático; cada retry tem contador e custo próprios; retry após estado parcial/desconhecido exige recuperação e quota explícitas.
8. A API atual, `Budget.max_cost`, `DiscoveryBudget`, os quinze registros Stage 4 e seus schemas permanecem reproduzíveis. A nova evidência é aditiva e versionada.
9. O oráculo calcula seu resultado com contrato e ledger independentes, sem reutilizar a transição, o custo privado ou os predicados do runtime.
10. A classificação final limita-se a mecanismo host-only ou `not_proven`; nenhum resultado desta frente é uma alegação de utilidade social.

## 9. Riscos e controles

**Custo desconhecido mascarado como zero.** Um campo ausente pode ser serializado como `0` por um default conveniente. O controle é usar `actual: null`, estado `UNKNOWN`, `unknown_event_count` e validação que rejeita `UNKNOWN` com valor numérico. Zero requer recibo explícito de custo medido.

**Contador monotônico apenas no processo.** Um reinício ou `bind()` pode recriar o objeto com contadores menores. O ledger deve carregar `budget_scope_id`, `parent_ledger_digest`, `event_seq` e uma política de persistência; snapshots velhos ou forks sem referência ao pai devem ser rejeitados. O escopo da campanha não pode ser reaberto para apagar consumo.

**Confundir contagem com custo.** Uma probe ou reset conta uma tentativa, mas não prova quantas unidades custou. O modelo mantém `cost_count` separado de `cost.actual` e exige recibo exato ou limite superior. Contar eventos não autoriza fabricar a soma.

**Reset confundido com recuperação.** O `discover()` atual chama `reset()` após cada probe, mas não recebe atestado de restauração. O controle registra reset e recuperação em eixos separados e bloqueia nova tentativa após estado parcial sem reconciliação independente.

**Retry duplicando efeito.** Um timeout pode esconder que a primeira tentativa já foi aplicada. O controle exige operação lógica idempotente ou reconciliação independente antes de retry. Um digest antigo não prova que a operação não ocorreu.

**Orçamento aplicado somente depois da chamada.** Calcular `cost_actual` no fim detecta um excesso, mas não impede que a chamada excedente ocorra. A cotação ou limite superior deve ser verificado e reservado antes da chamada; recibo maior fecha o ledger como `OVERRUN` e preserva o fato de que a ação já começou.

**Digest circular.** O runtime pode calcular custo, digerir seu próprio ledger e usar esse digest para declarar que o custo está correto. O digest fornece integridade dos eventos recebidos, não independência nem verdade. O oráculo precisa de recibos e regras declarativas próprias; uma assinatura do mesmo processo não é atestação independente.

**Contrato de custo não autêntico.** `EXACT(1)` emitido pelo mesmo host que realiza a ação pode estar incompleto ou mentir. A proposta deve registrar proveniência, versão e método de emissão. Quando a pergunta exigir confiança além da integridade, a resposta deve ser `UNKNOWN_COST` ou `UNSUPPORTED_BY_CONTRACT`, não uma validação circular.

**Sonda mutante contada como custo inócuo.** `probe_count` mede a chamada de descoberta, mas não transforma `host.execute()` em observação não mutante. O contrato deve declarar `NON_MUTATING`, `MUTATING_AUTHORIZED` ou `UNKNOWN`. No modo estrito, `UNKNOWN` bloqueia antes de sondar quando o risco não pode ser limitado.

**Compatibilidade permissiva.** Um consumidor pode tratar `TransferProposal` ou um booleano legado como comando. A documentação, o objeto `TransferDecision` e os testes devem deixar claro que orçamento e proposta não concedem autoridade. O wrapper legado deve ter telemetria de uso e depreciação, sem alterar o retorno histórico na Fase A.

**Sobreajuste às fixtures.** Uma implementação pode reconhecer `m8a-budget-01` ou a constante `fixture.cost`. Os testes devem trocar IDs, valores, unidade de custo, ordem, número de ações e tipo de falha, mantendo a estrutura do problema. O caso privado deve mudar o custo sem mudar o payload público.

**Falsa precisão de unidade.** Um inteiro chamado “custo” pode misturar tokens, tempo, energia, chamadas e dinheiro. `cost_max` deve declarar unidade e versão do contrato. Somar unidades incompatíveis é inválido e produz `UNKNOWN_COST` até que exista conversão pública e versionada. Este plano não converte custo de host em custo físico.

**Confusão entre budget de proposta e budget de execução.** Sondar para construir um plano pode consumir o orçamento antes de qualquer execução autorizada. O ledger deve registrar a fase e encadear o escopo; não se pode copiar o contador para zero ao passar de proposta para execução. Um limite residual menor que `planned_steps` bloqueia a execução externa.

## 10. Referências

[1]: ../../../docs/53-BENCHMARK-HOLDOUT-ADVERSARIAL.md "Etapa 4 — benchmark holdout adversarial"

[2]: ../../holdout_adversarial.py "Campanha holdout adversarial"

[3]: ../../holdout_hosts.py "Hosts públicos e fixtures adversariais"

[4]: ../../stage4/contracts.py "Contratos de resultado e orçamento da Etapa 4"

[5]: ../../holdout_benchmark_schema.json "Schema do benchmark holdout Stage 4"

[6]: ../../evidence/holdout_benchmark_v1.json "Evidência congelada do benchmark holdout v1"

[7]: ../../symbiont_v2/core.py "HERUS Symbiont v2 research runtime"

[8]: ../../symbiont_v2/test_symbiont.py "Testes do Symbiont v2"

[9]: ../../symbiosis_utility_contract.json "Contrato de utilidade da simbiose HERUS"

[10]: 01-observation_contract.md "Stage 5 — contrato tipado de observabilidade"

[11]: 02-hidden_effect_containment.md "Stage 5 — contenção de efeitos ocultos"

<!-- Relatório produzido como proposta de pesquisa. Nenhuma implementação foi alterada. -->
