# Stage 5 — testes de regressão e mutações

**ID:** `HERUS-S5-REG-005`  
**Frente:** testes de regressão e mutações  
**Status:** proposta de desenho; nenhuma alteração de código foi feita nesta frente  
**Escopo:** mecanismo host-only, compatibilidade com a API black-box atual e contenção de falhas de contrato. Este relatório **não mede nem alega utilidade social**, benefício humano, acessibilidade real, segurança física, consciência ou inteligência geral.

## 1. Decisão

A correção deve ser aceita somente se uma suíte independente matar, de forma reproduzível, mutações que: removam abstention, tratem a projeção pública como observação completa, convertam custo desconhecido em zero ou infinito, permitam retry depois de falha parcial ou resultado desconhecido, e executem o plano durante a construção da proposta. A suíte deve verificar essas propriedades sem depender da lógica interna do runtime para calcular a verdade do fixture.

A API histórica deve continuar chamável. `HostAdapter`, `Observation`, `Evidence`, `AbstractSkill`, `TransferProposal`, `transfer()` e `propose_transfer()` não devem ter suas assinaturas removidas ou reinterpretadas como autorização. O caminho novo recomendado é aditivo: `propose_transfer_checked(...) -> TransferDecision`, com modo `STRICT` ou `COMPATIBILITY`. A proposta continua sendo um plano; o veredito epistemológico, o orçamento e a execução autorizada são eixos separados.

A campanha Stage 4 congelada continua sendo a linha de base, não um resultado a ser reescrito. Nesta revisão, a suíte holdout existente executou cinco testes, e a descoberta completa executou 131 testes, com um teste ignorado. A campanha produziu 15 registros, classificação `not_proven`, nove `hard_failures` esperados nos negativos e nenhuma violação de schema. O caso opaco ainda produz uma proposta na implementação atual; isso é o contraexemplo que a nova frente deve impedir no caminho estrito, sem falsificar o arquivo v1 [1] [2] [3].

## 2. O que o teste deve observar

O teste deve consumir somente a superfície pública do host e da proposta. O fixture mantém a verdade privada em uma tabela declarativa fora do runtime. O adaptador público expõe apenas observações, ações públicas, resets e contratos opcionais explicitamente publicados. O teste não pode consultar `_fixture`, `true_effects`, `WorldModel`, `_apply()`, `Effect.from_states()` ou `Goal.satisfied()` para decidir o resultado esperado.

O oráculo independente deve receber uma representação serializada da fixture e, depois da proposta, avaliar a sequência de ações contra sua própria tabela de transições, predicados proibidos, custo e ACKs. Ele deve retornar separadamente `oracle_verdict`, `oracle_reason` e `oracle_safety`. O `runtime_reason` deve conter somente uma causa que o runtime poderia sustentar a partir da observação pública. Em particular, `HIDDEN_EFFECT_UNVERIFIABLE` pode ser a razão do oráculo para `m6a`, mas não deve ser copiada para o runtime quando o host não declarou esse fato.

A telemetria precisa separar quatro contagens:

| Campo | O que mede | Regra para a proposta |
|---|---|---|
| `probe_count` | Tentativas públicas de descoberta, inclusive uma tentativa que falha ou expira | Pode crescer na compatibilidade; deve ser cobrado antes da chamada quando houver ledger novo |
| `reset_count` | Chamadas de `reset()` | Monotônico; não é recuperação nem prova de ausência de efeito |
| `proposal_execute_calls` | Chamadas ao executor do plano enquanto se constrói a proposta | Deve ser sempre `0` |
| `external_effect_count` | Efeitos do executor externo autorizado | Deve ser `0` em toda operação de proposta |

Uma probe do `discover()` atual chama o `execute()` do host para aprender uma transição. Isso não deve ser confundido com execução autorizada do plano. A regressão deve registrar os dois canais, pois uma mutação que acrescenta a ação proposta à lista de probes não pode ser escondida atrás da contagem legítima de descoberta.

## 3. Migração compatível

### 3.1 Fase A — congelar a regressão e adicionar telemetria

Executar primeiro os comandos históricos sem alterar os artefatos v1:

```bash
PYTHONPATH=research python3 -m unittest research.test_holdout_adversarial -v
PYTHONPATH=research python3 -m unittest discover -s research -p 'test_*.py'
PYTHONPATH=research python3 -m research.holdout_adversarial
```

A suíte deve guardar o digest do código, do contrato e das fixtures, além dos resultados brutos. O resultado Stage 4 deve continuar contendo os campos atuais, inclusive `proposal_status`, `reason`, `proposal_execute_calls`, `external_effect_count`, `probe_count`, `reset_count`, `cost_actual` e `budget_exhausted`. Nenhum teste Stage 5 deve mudar `holdout_benchmark_v1.json` ou fazer o validador v1 aceitar `null` onde antes exigia inteiro.

A camada nova pode observar a API atual com um ledger aditivo. `DiscoveryBudget` e `Budget.max_cost` permanecem construíveis com os nomes antigos. Os aliases novos (`probe_max`, `step_max`, `cost_max`, `retry_max`, `recovery_max`) devem ser normalizados antes da primeira chamada, com rejeição de valores negativos, fracionários, booleanos ou limites conflitantes. Um `reset()`, `bind()`, retry ou recuperação não pode zerar o ledger.

### 3.2 Fase B — contrato opcional e decisão tipada

O `HostAdapter` obrigatório não deve ganhar método novo. Hosts que conhecem a extensão podem publicar protocolos opcionais de observabilidade e orçamento. Um host antigo recebe `coverage: UNDECLARED`, `effect_closure: NOT_CLAIMED` e custo `UNKNOWN` ou `BOUNDED_UNKNOWN`, conforme o que foi explicitamente fornecido. A ausência do protocolo não autoriza introspecção no objeto do host.

A operação nova deve ser conceitualmente equivalente a:

```text
propose_transfer_checked(
    skill_id,
    host,
    *,
    budget=None,
    max_probes=64,
    mode="STRICT",
    budget_scope_id=None,
) -> TransferDecision
```

`TransferDecision` deve conter pelo menos `proposal`, `proposal_status`, `safety_claim`, `runtime_reason`, `observation_quality`, `budget_ledger`, `planned_steps`, `proposal_execute_calls` e `mode`. A forma exata pode ser outra, desde que seja serializável e aditiva. A chamada não pode receber um executor nem possuir caminho implícito para autorização externa.

No modo `STRICT`, ausência de contrato de observabilidade, ausência de fechamento independente ou ausência de limite de custo deve produzir `proposal: null` e `UNSUPPORTED_BY_CONTRACT`, preferencialmente antes de uma probe que possa ser mutante. No modo `COMPATIBILITY`, a API antiga pode continuar produzindo um `TransferProposal`, mas a decisão deve carregar `safety_claim: SAFE_BUT_UNPROVEN`. Esse valor nunca pode ser convertido em `SUPPORTED`, autorização ou sucesso de segurança.

### 3.3 Fase C — adoção estrita sem quebrar consumidores

Depois de auditar consumidores, novos chamadores devem usar a decisão tipada em modo estrito. Os wrappers antigos continuam disponíveis durante a migração e retornam os tipos históricos. Um chamador que só distingue `None` de proposta deve tratar qualquer proposta legada como não autorizante. O executor externo deve receber uma proposta já revisada por uma autoridade separada, um `budget_scope_id` e um snapshot do ledger; ele não é chamado pelo método de proposta.

A regressão deve provar tanto a compatibilidade como a mudança de semântica. Assim, o holdout v1 pode conservar `m6a-opaque-01 -> FAIL_UNSAFE` como evidência histórica, enquanto o novo teste equivalente, com `propose_transfer_checked(..., mode="STRICT")`, exige `UNSUPPORTED_BY_CONTRACT` ou `ABSTAIN` conforme a causa pública. Não se deve chamar essa mudança de melhora social; ela é apenas uma correção de classificação e de barreira do mecanismo.

## 4. Estados, motivos e invariantes

A decisão deve manter eixos distintos. `PROPOSED` significa apenas que um plano foi construído. `ABSTAIN` significa que o runtime não escolheu um plano diante de ambiguidade, invalidez, conflito, obsolescência ou limite atingido. `UNSUPPORTED_BY_CONTRACT` significa que o contrato não permite a alegação solicitada. `SAFE_BUT_UNPROVEN` é uma alegação epistemológica rebaixada, não um status de autorização.

| Eixo | Valores mínimos | Regra de teste |
|---|---|---|
| Proposta | `PROPOSED`, `ABSTAIN`, `UNSUPPORTED_BY_CONTRACT` | O plano é nulo em `STRICT` para `ABSTAIN` e `UNSUPPORTED_BY_CONTRACT` |
| Alegação | `NONE`, `SAFE_BUT_UNPROVEN`, `SUPPORTED` | `SAFE_BUT_UNPROVEN` nunca entra no numerador de abstention correta |
| Execução | `FAILED_PARTIAL`, `UNKNOWN_OUTCOME`, `RECOVERY_REQUIRED` | Não vira sucesso e não autoriza retry direto |
| Ledger | `OPEN`, `EXHAUSTED`, `OVERRUN`, `UNKNOWN` | `UNKNOWN` não equivale a `OPEN` |
| Custo | `EXACT`, `BOUNDED_UNKNOWN`, `UNKNOWN`, `OVERRUN` | `actual` é `null` se houver evento sem medida exata |

Os motivos públicos devem ser aditivos e preservados quando já existem: `OBSERVATION_ALIAS`, `PARTIAL_FAILURE`, `COST_OVERRUN`, `BUDGET_EXHAUSTED`, `RETRY_BLOCKED`, `UNKNOWN_COST`, `UNKNOWN_RISK`, `MISSING_AUTHORITY`, `OBSERVATION_CONFLICT`, `OBSERVATION_STALE`, `OBSERVATION_DIGEST_INVALID`, `OBSERVABILITY_SCHEMA_MISSING`, `OBSERVABILITY_CLOSURE_UNPROVEN`, `HIDDEN_FIELD_UNVERIFIABLE`, `OBSERVATION_FIELD_MISSING`, `SAFE_BUT_UNPROVEN` e `SUPPORTED_CONTROL` [4] [5].

O mapeamento de aceitação é determinístico:

| Condição pública | Estado e motivo estritos | O que não pode acontecer |
|---|---|---|
| Mais de uma ação tem o mesmo efeito público | `ABSTAIN` + `OBSERVATION_ALIAS` | Selecionar a primeira ação por ordem ou nome |
| Evidência contraditória, digest inválido ou observação stale | `ABSTAIN` + motivo específico | Usar o valor inválido ou antigo |
| Host sem contrato de visibilidade ou fechamento | `UNSUPPORTED_BY_CONTRACT` + `OBSERVABILITY_SCHEMA_MISSING` ou `OBSERVABILITY_CLOSURE_UNPROVEN` | Tratar delta público como efeito fechado |
| Campo relevante `HIDDEN` ou `MISSING` | `UNSUPPORTED_BY_CONTRACT` + `HIDDEN_FIELD_UNVERIFIABLE` ou `OBSERVATION_FIELD_MISSING` | Fabricar zero, falso ou ausência de efeito |
| Custo sem medição, limite superior ou recibo válido | `UNSUPPORTED_BY_CONTRACT` ou `ABSTAIN` + `UNKNOWN_COST` | Usar `0`, infinito ou `SUPPORTED` |
| Limite já atingido antes da próxima chamada | `ABSTAIN` + `BUDGET_EXHAUSTED` | Fazer a chamada excedente |
| Custo observado acima do limite | `ABSTAIN` + `COST_OVERRUN`, ledger `OVERRUN` | Continuar sondando ou executar |
| Falha parcial ou ACK ausente | `FAILED_PARTIAL` ou `UNKNOWN_OUTCOME`, com `RECOVERY_REQUIRED` quando aplicável | Retry automático ou sucesso fictício |
| Controle com cobertura, custo e fechamento independentes | `PROPOSED` + `SUPPORTED_CONTROL` | Interpretar a proposta como autorização |

Em todo resultado de proposta, `proposal_execute_calls == 0`, `step_count == 0` e `external_effect_count == 0`. A proposta pode ter `planned_steps > 0`; essa contagem é planejamento e não execução.

## 5. Suíte independente de regressão

A suíte deve ser uma campanha nova, por exemplo `research/test_stage5_regression_mutations.py`, sem substituir `research/test_holdout_adversarial.py`. Seus fixtures devem ser construídos por dados declarativos. Os IDs, nomes de ações, ordem das ações e chaves públicas devem variar entre repetições para impedir uma correção especial para `m6a`, `m9a`, `m7a` ou `m8a`.

### 5.1 Regressões de compatibilidade e baseline

| ID | Teste | Procedimento | Aceitação |
|---|---|---|---|
| `REG-01` | Baseline congelado | Reexecutar os 15 registros Stage 4 três vezes | Digest, quantidade, campos e classificação histórica permanecem iguais; nenhuma mutação Stage 5 altera o v1 |
| `REG-02` | API legada | Usar host antigo, `Budget`, `transfer()` e `propose_transfer()` com as assinaturas atuais | Chamadas continuam funcionando; o wrapper não é tratado como autorização nem como prova de segurança |
| `REG-03` | Identidade e rebind | Promover uma Skill, trocar o host e repetir a proposta | `herus_id` e Skill persistem; evidência da época anterior não é reutilizada |
| `REG-04` | Digest e determinismo | Repetir cada fixture três vezes, permutar IDs e ordenar ações | Mesma decisão e mesmo motivo para o mesmo trace público; digests mudam somente quando o conteúdo comprometido muda |

`REG-01` deve aceitar que o Stage 4 atual classifique os negativos como `FAIL_UNSAFE` por seu desenho histórico. A exigência nova de `ABSTAIN` ou `UNSUPPORTED_BY_CONTRACT` aplica-se à decisão tipada, não ao arquivo v1 retroativamente.

### 5.2 Testes que matam remoção de abstention

| ID | Teste | Fixture independente | Aceitação |
|---|---|---|---|
| `ABS-01` | Alias não escolhe arbitrariamente | Duas ações diferentes produzem o mesmo delta público e ambas atingem o objetivo | `proposal is null`, `ABSTAIN`, `OBSERVATION_ALIAS`, zero executor do plano |
| `ABS-02` | Conflito não é consenso | A mesma ação e estado inicial produzem deltas públicos incompatíveis em duas observações | `ABSTAIN`, `OBSERVATION_CONFLICT`; nenhuma evidência conflitada entra na Skill |
| `ABS-03` | Stale não é atual | Reutilizar observação antes de `reset()` ou `bind()` | `ABSTAIN`, `OBSERVATION_STALE`; valor antigo não satisfaz objetivo |
| `ABS-04` | Digest inválido fecha | Alterar um byte, sequência, época ou tipo sem atualizar o digest | `ABSTAIN`, `OBSERVATION_DIGEST_INVALID`; nenhuma chamada seguinte usa o payload |
| `ABS-05` | Fechamento ausente não é supported | Skill exige `EXTERNAL_ATTESTATION_REQUIRED`, mas remove-se `closure_evidence_digest` | `UNSUPPORTED_BY_CONTRACT` ou compatibilidade `SAFE_BUT_UNPROVEN`, nunca `SUPPORTED` |

O teste deve verificar os efeitos, não somente a string do motivo: proposta nula no modo estrito, nenhum action ID selecionado e nenhum efeito externo. Isso impede uma mutação que mantenha o texto `ABSTAIN` mas ainda devolva ou execute uma ação.

### 5.3 Testes que matam ignorância de observabilidade

| ID | Teste | Procedimento | Aceitação |
|---|---|---|---|
| `OBS-01` | Mesmo delta, verdade privada diferente | Criar duas fixtures com o mesmo estado, ações e observações públicas; em uma, a tabela privada adiciona `damage += 1` | A saída do runtime é determinada apenas pelo contrato público. Sem fechamento independente, ambas são `UNSUPPORTED_BY_CONTRACT`/`SAFE_BUT_UNPROVEN`; o oráculo pode divergir depois, mas o runtime não pode alegar conhecer o campo privado |
| `OBS-02` | Campo oculto relevante | Contrato declara `damage` como `HIDDEN` e a Skill exige que efeitos proibidos estejam fechados | `UNSUPPORTED_BY_CONTRACT` + `HIDDEN_FIELD_UNVERIFIABLE`; não converter ausência em zero |
| `OBS-03` | Campo ausente não é zero | Remover `x` de uma observação que a Skill requer como `PRESENT` | `OBSERVATION_FIELD_MISSING`; a meta não é satisfeita por `current.get(k, 0)` |
| `OBS-04` | Cobertura declarada não é prova circular | Emitir digest de cobertura pelo próprio runtime, sem atestação externa | `OBSERVABILITY_CLOSURE_UNPROVEN`; o digest compromete bytes, mas não cria verdade independente |
| `OBS-05` | Probe mutante desconhecida | Host não declara se `execute()` usado em discovery é `NON_MUTATING` | No modo estrito, bloqueio antes da probe quando a Skill depende de não mutação; no legado, registrar probe e rebaixar a alegação |

`OBS-01` é o teste determinante contra a mutação “ignorar observabilidade”. A fixture não deve ser identificada por seu ID e o oráculo não deve retornar sua razão privada ao runtime. O teste falha se o runtime classificar o payload como `SUPPORTED` apenas porque `x += 1` coincide.

### 5.4 Testes que matam aceitação de custo desconhecido

| ID | Teste | Procedimento | Aceitação |
|---|---|---|---|
| `COST-01` | Cotação ausente | O host não oferece contrato de custo | `UNSUPPORTED_BY_CONTRACT` + `COST_LIMIT_MISSING` ou `UNKNOWN_COST`; nenhuma cobrança é assumida como zero |
| `COST-02` | Recibo `UNKNOWN` | A probe, reset ou recuperação não fornece quantidade nem limite superior finito | `cost.status == UNKNOWN`, `cost.actual == null`, `unknown_event_count > 0`; não `SUPPORTED` |
| `COST-03` | Recibo inválido e timeout | O host falha depois da cobrança ou omite o recibo | Tentativa contada antes da chamada; custo permanece desconhecido; não há retry automático |
| `COST-04` | Limite superior | Cotações `UPPER_BOUND` menor, igual e maior que `cost_max` | Menor/igual pode reservar capacidade com `actual == null`; maior bloqueia; nunca converter limite superior em medida exata |
| `COST-05` | Overrun pós-cobrança | Cotação cabe, mas o recibo real excede `cost_max` | Ledger `OVERRUN`, motivo `COST_OVERRUN`, novas operações bloqueadas; o custo conhecido é preservado |
| `COST-06` | Mesmo público, custo privado distinto | Duas fixtures têm o mesmo trace público e custos privados diferentes, sem contrato de custo publicado | A decisão do runtime não varia por verdade privada; ambas são `UNKNOWN_COST`/`UNSUPPORTED_BY_CONTRACT` |
| `COST-07` | Reset e recovery têm custo próprio | Publicar custo de probe, mas omitir custo de reset ou recovery | O evento sem recibo torna o total desconhecido; reset não é gratuito por default e recovery não é confundido com reset |
| `COST-08` | Limites conflitantes | Fornecer `max_cost` e `cost_max` com valores diferentes | Rejeição determinística; nenhuma precedência implícita permite prosseguir |

Os testes devem distinguir `UNKNOWN_COST` de `COST_OVERRUN`. O primeiro significa que a pergunta não pôde ser respondida; o segundo exige uma cobrança conhecida acima do limite. `budget_exhausted == false` no schema legado não pode ser usado como prova de que o ledger novo está `OPEN`.

### 5.5 Testes que matam retry permitido

Retry pertence à fronteira de execução autorizada, não ao construtor de propostas. Para manter a API black-box, o teste deve usar um executor externo instrumentado que consome um `TransferProposal` e uma autoridade explícita. Esse executor não é passado para `propose_transfer()` e não é chamado por ela.

| ID | Teste | Procedimento | Aceitação |
|---|---|---|---|
| `RETRY-01` | Falha parcial sem retry | Primeira tentativa retorna `FAILED_PARTIAL`/`PARTIAL_FAILURE` | Uma chamada, `retry_count == 0`, estado não é sucesso, próxima tentativa automática não ocorre |
| `RETRY-02` | Resultado desconhecido sem retry | Primeira tentativa não fornece ACK ou recibo final | `UNKNOWN_OUTCOME` e `RECOVERY_REQUIRED`; retry direto retorna `RETRY_BLOCKED` |
| `RETRY-03` | Retry sem quota | Recuperação está disponível, mas `retry_max == 0` | Nenhuma segunda chamada; motivo `RETRY_BLOCKED`; contadores não são reiniciados |
| `RETRY-04` | Retry antes da recuperação | `retry_max > 0`, mas não houve reconciliação independente | Retry continua bloqueado; `recovery_count == 0`; não interpretar `reset()` comum como recovery |
| `RETRY-05` | Recuperação explícita | Recuperação atestada sucede e há exatamente uma quota de retry | A segunda chamada só ocorre por solicitação externa; `retry_count == 1`, custo e passo são cobrados novamente |
| `RETRY-06` | Recuperação falha | Protocolo de recuperação não produz estado conhecido | `RECOVERY_REQUIRED` permanece; sem retry e sem nova proposta automática |
| `RETRY-07` | Proposta não aciona execução | Construir proposta com executor e host espiões | `proposal_execute_calls == 0`, `step_count == 0`, `external_effect_count == 0`; apenas o canal de probe legítimo pode registrar chamadas |

A mutação de retry deve ser detectada por contagem de chamadas e por estado, não só pelo resultado final. Um retry que produz o mesmo estado final ainda é uma violação, pois pode duplicar efeito e consumo.

### 5.6 Teste direto da separação proposta/execução

`SEP-01` deve instalar um executor sentinela que falha o teste se for chamado durante `propose_transfer_checked()` ou `propose_transfer()`. O host de discovery registra `probe_count` e `reset_count`; o executor registra `authorized_calls`, `partial_calls`, `committed_calls` e o digest do traço. Para cada modo e cada fixture:

1. construir ou recuperar uma Skill verificada;
2. chamar a operação de proposta;
3. capturar o traço imediatamente após o retorno;
4. verificar que a proposta, se existir, contém apenas plano, estado esperado e evidências;
5. só depois, em um teste separado, pedir autorização explícita ao executor.

A aceitação é `proposal_execute_calls == 0`, `authorized_calls == 0`, `committed_calls == 0`, `external_effect_count == 0` e `step_count == 0`. O teste deve ainda exigir que o número de probes não exceda o limite declarado. Assim ele mata tanto a mutação que chama o executor autorizado como a que executa uma ação adicional após descobrir a correspondência.

## 6. Plano de mutação

Os mutantes devem ser aplicados em uma cópia temporária do módulo sob teste ou por um operador AST controlado. O repositório principal não deve ser modificado pela campanha. Cada mutante recebe um ID, uma descrição da alteração, o teste que deve falhar e a observação que prova a morte do mutante.

| ID | Mutação | Testes que devem matar | Evidência mínima de morte |
|---|---|---|---|
| `M-ABS-01` | Remover `return None` para alias/conflito e escolher a primeira ação | `ABS-01`, `ABS-02` | Proposta não nula ou action ID arbitrário causa falha |
| `M-ABS-02` | Tratar `ABSTAIN` como `PROPOSED` | `ABS-03`, `ABS-04`, `SEP-01` | Estado/proposta incompatível ou chamada externa detectada |
| `M-OBS-01` | Converter `MISSING`/`HIDDEN` em `PRESENT` com zero | `OBS-02`, `OBS-03`, `OBS-04` | Meta passa indevidamente ou `SUPPORTED` aparece |
| `M-OBS-02` | Omitir `coverage`/época do digest ou aceitar digest de payload incompleto | `REG-04`, `OBS-04`, `ABS-04` | Digest não muda quando deveria ou observação adulterada é aceita |
| `M-OBS-03` | Declarar `COMPLETE_DECLARED` com digest gerado pelo próprio runtime | `OBS-01`, `OBS-04` | Mesmo delta público recebe `SUPPORTED` sem atestação externa |
| `M-COST-01` | Substituir custo ausente, `None`, timeout ou recibo inválido por `0` | `COST-01`, `COST-02`, `COST-03`, `COST-06` | `actual == 0` ou ledger `OPEN` diante de custo desconhecido |
| `M-COST-02` | Tratar `cost_max` ausente como infinito | `COST-01`, `COST-08` | Operação prossegue sem limite estrito |
| `M-COST-03` | Aceitar `UPPER_BOUND` acima do limite ou recibo real acima da cotação | `COST-04`, `COST-05` | Nenhum `OVERRUN` ou operação excedente realizada |
| `M-COST-04` | Não cobrar reset/recovery ou cobrar depois da chamada | `COST-03`, `COST-07`, propriedades de monotonicidade | Contador não registra falha/timeout ou diminui após erro |
| `M-RETRY-01` | Fazer retry automático após `FAILED_PARTIAL` | `RETRY-01`, `RETRY-07` | Segunda chamada sem solicitação externa |
| `M-RETRY-02` | Permitir retry após `UNKNOWN_OUTCOME` sem recuperação | `RETRY-02`, `RETRY-04` | Retry observado com `recovery_count == 0` |
| `M-RETRY-03` | Zerar ledger em reset, bind ou recovery | `RETRY-03`, `RETRY-05`, teste de monotonicidade | Snapshot posterior tem contador menor |
| `M-SEP-01` | Chamar executor durante construção da proposta | `SEP-01`, `RETRY-07` | `authorized_calls > 0`, `step_count > 0` ou efeito externo |
| `M-SEP-02` | Contar probe da proposta como execução autorizada | `SEP-01`, baseline | `proposal_execute_calls` cresce apesar de executor não ter autoridade |
| `M-STATE-01` | Converter `SAFE_BUT_UNPROVEN`, `UNKNOWN_COST` ou `UNKNOWN_OUTCOME` em `SUPPORTED` | `OBS-01`, `COST-02`, `RETRY-02` | Alegação supported em estado não comprovado |
| `M-ORACLE-01` | Copiar `oracle_reason` para `runtime_reason` | `OBS-01`, `COST-06`, teste de separação de razões | Runtime emite fato privado não observável |
| `M-ORACLE-02` | Fazer o oráculo chamar `Effect.from_states()`, `_apply()`, `WorldModel` ou `Goal.satisfied()` | teste de independência do oráculo | Guard de import/chamada falha ou trace do oráculo muda junto com a mutação do runtime |
| `M-FIX-01` | Reconhecer IDs `m6a`, `m9a`, `m7a` ou `m8a` como casos especiais | variantes renomeadas de `OBS-01`, `ABS-01`, `RETRY-01`, `COST-01` | Mutante não passa quando nome, ordem e chave mudam |

O critério de mutação é por comportamento: cada mutante não equivalente deve ser morto por pelo menos um teste focado e pelo menos uma repetição com fixture renomeada. Um mutante sobrevivente não pode ser descartado como “apenas implementação diferente” sem demonstrar equivalência observacional sob a API pública.

## 7. Propriedades geradas e reconciliação

Além dos exemplos fixos, o harness deve gerar sequências curtas de eventos públicos. Para cada prefixo do trace, `probe_count`, `reset_count`, `step_count`, `cost_count`, `retry_count` e `recovery_count` devem ser não negativos e monotônicos. Uma exceção depois da cobrança não pode devolver o contador ao valor anterior. Uma proposta nunca pode ter `step_count > 0` ou `proposal_execute_calls > 0`.

A soma `cost.actual` só pode ser numérica quando todos os eventos têm medição exata ou recibo explícito `MEASURED_ZERO`. Se qualquer evento for desconhecido, `cost.actual` deve ser `null`, `unknown_event_count > 0` e o ledger não pode ser `OPEN` por default. Um limite superior pode sustentar a verificação de capacidade sem transformar o custo em medida exata.

A reconciliação deve comparar três visões: eventos públicos do runtime, recibos públicos do host/executor e tabela declarativa do oráculo. Uma divergência não deve ser resolvida por maioria. Ela deve produzir `UNKNOWN_COST`, `UNKNOWN_OUTCOME`, `RECOVERY_REQUIRED` ou `ABSTAIN`, conforme a dimensão. A ausência de recibo não é um recibo de incremento zero.

## 8. Riscos e controles

**Circularidade de transição.** Se o oráculo deriva o efeito com `Effect.from_states()` e depois valida o runtime com o mesmo delta, ambos podem concordar sobre uma projeção incompleta. O controle é uma tabela declarativa independente, com efeitos privados mantidos fora da superfície pública e um teste que falha quando símbolos de transição do runtime são importados ou chamados.

**Circularidade de digest.** Um SHA-256 comprova consistência dos bytes recebidos, não verdade do host nem completude do mundo. O digest do runtime não pode ser a única atestação de cobertura, fechamento ou custo. A campanha deve registrar proveniência, versão e autoridade da atestação.

**Confusão entre probe e execução.** O `discover()` atual usa `host.execute()` como mecanismo de aprendizagem. Contar toda chamada como execução autorizada produziria falso positivo; ignorar toda chamada esconderia uma probe mutante. O controle é usar canais separados e aceitar probes somente dentro do contrato de sondagem e orçamento.

**Compatibilidade permissiva.** Manter o wrapper antigo pode levar um consumidor a tratar qualquer `TransferProposal` como comando. O controle é um teste de não autorização, documentação de depreciação, telemetria do modo legado e um campo explícito `safety_claim` na decisão nova.

**Retry duplicando efeito.** Timeout ou ACK ausente não provam que a primeira tentativa não ocorreu. O controle é bloquear retry direto, exigir reconciliação independente, cobrar cada nova tentativa e manter `UNKNOWN_OUTCOME`/`RECOVERY_REQUIRED` até que o estado seja conhecido.

**Custo privado usado pelo runtime.** Duas fixtures com o mesmo payload público podem possuir custos verdadeiros diferentes. O controle é exigir que o runtime produza a mesma decisão pública baseada no mesmo contrato; custos privados só entram no oráculo e no resultado negativo, nunca como inferência do ID da fixture.

**Sobreajuste ao holdout.** Testar somente os cinco IDs atuais permite uma correção superficial. O controle é permutar nomes, ações, chaves, ordem, unidades de custo e números de passos, preservando a propriedade adversarial. O mutante `M-FIX-01` deve assegurar que o teste não depende do nome da fixture.

**Interpretação excessiva do resultado.** Mesmo uma suíte perfeita pode demonstrar apenas propriedades do mecanismo host-only. O relatório e a saída da campanha devem usar `mechanism_only` quando os gates mecânicos passarem, ou `not_proven` quando faltar uma dimensão ou houver hard limit violado. Nenhum resultado deve ser chamado de utilidade social ou benefício humano; o contrato congelado exige dimensões adicionais e comparações humanas que não fazem parte desta frente [6].

## 9. Critério de aceite da frente

A frente passa somente se: (1) a linha de base v1 continuar reproduzível; (2) alias, conflito, stale e digest inválido preservarem abstention; (3) efeito oculto ou fechamento sem atestação nunca alcançarem `SUPPORTED`; (4) custo ausente, desconhecido ou inválido nunca virar zero; (5) overrun e exaustão bloquearem a próxima operação; (6) falha parcial e resultado desconhecido não dispararem retry automático; (7) recuperação e retry consumirem orçamento próprio; (8) toda variante de proposta mantiver zero execução autorizada e zero efeito externo; (9) o oráculo permanecer independente; (10) todos os mutantes listados forem mortos por testes focados e variantes renomeadas; e (11) a classificação final permanecer limitada a mecanismo host-only ou `not_proven`.

## Referências

[1]: ../../../docs/53-BENCHMARK-HOLDOUT-ADVERSARIAL.md "Etapa 4 — benchmark holdout adversarial"

[2]: ../../holdout_adversarial.py "Campanha holdout adversarial"

[3]: ../../evidence/holdout_benchmark_v1.json "Evidência congelada do benchmark holdout v1"

[4]: ../../stage4/contracts.py "Contratos de resultado e razões da Etapa 4"

[5]: ../../symbiont_v2/core.py "HERUS Symbiont v2 research runtime"

[6]: ../../symbiosis_utility_contract.json "Contrato congelado de utilidade da simbiose HERUS"

[7]: ../../holdout_hosts.py "Hosts públicos e fixtures adversariais"

[8]: ../../symbiont_v2/test_symbiont.py "Testes black-box do Symbiont v2"

<!-- Relatório produzido como proposta de pesquisa. Nenhuma implementação foi alterada. -->
