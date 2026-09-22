# HERUS — arquitetura restante para integração final e caminho social

**ID:** `HERUS-REM-SYS-ARCH-005`  
**Frente:** integração das etapas 6/7, separação entre prova host-only e evidência humana, e controle de alegações.  
**Status:** auditoria e desenho implementável; **nenhuma alteração de código foi feita nesta frente**.

## 1. Decisão executiva

A menor sequência implementável não deve conectar o runtime diretamente a hardware, usuários ou atuadores. Ela deve fechar primeiro a fronteira mecânica da etapa 6 e somente depois abrir uma etapa 7 social independente. A ordem recomendada é:

1. congelar a linha de base Stage 4/5 e o manifesto de proveniência;
2. implementar o adaptador externo de execução, o ledger append-only e a recuperação independente;
3. estender o holdout para época, stale evidence, pré-condições, ordem, recursos e custo;
4. executar mutações e três repetições em processos limpos;
5. classificar o resultado somente como `mechanism_only` ou `not_proven`;
6. se, e somente se, os hard limits mecânicos passarem, preregistrar uma tarefa humana estreita com baseline convencional e condição sem simbionte;
7. medir erro, tempo/esforço, dependência de interface, privacidade, acessibilidade e recuperação do usuário em um pacote de evidência separado.

O estado atual não autoriza saltar para a etapa 7. A suíte de pesquisa observada passa (`135` testes, `1` skipped), mas isso é regressão local, não prova de integração completa. O benchmark holdout ainda reproduz o contraexemplo `m6a-opaque-01`: o runtime legado propõe, enquanto o oráculo independente classifica `FAIL_UNSAFE`. A correção da etapa 5 bloqueia esse caso no modo estrito sem contrato, mas a implementação atual ainda delega a descoberta ao caminho legado e registra `probe_execute_calls` como zero embora `discover()` use `host.execute()` para sondagem. Portanto, a etapa 6 permanece **desenho a implementar**, e a classificação prudente é `not_proven` até a campanha integrada.

A separação normativa é:

```text
observação pública + contrato
          ↓
runtime HERUS: preflight, descoberta limitada, plano e decisão
          ↓ fecha sem executor autorizado
oráculo independente: verdade privada e avaliação pós-decisão
          ↓ somente com autoridade externa
executor: nonce, TTL, revogação, recibos, commit e recovery
          ↓ pacote host-only encerrado
protocolo humano separado: tarefa, baseline, no-symbiont, consentimento
```

`SUPPORTED` significa apenas que uma proposta satisfaz um contrato de observabilidade delimitado. Não significa autorização, execução, segurança física ou benefício humano [1] [2].

## 2. Auditoria do que existe e do que falta

| Superfície | Evidência atual | Lacuna para as etapas 6/7 |
|---|---|---|
| Holdout adversarial | `docs/53-BENCHMARK-HOLDOUT-ADVERSARIAL.md`, `research/holdout_adversarial.py` e `research/evidence/holdout_benchmark_v1.json` preservam controle, alias, falha parcial, overrun e efeito oculto. | O artefato v1 deve permanecer congelado. Falta uma campanha nova para temporalidade, estrutura e execução autorizada.
| Contrato de observabilidade | `docs/54-CORRECAO-CONTRATO-OBSERVABILIDADE.md`, `research/symbiont_v2/stage5.py` e `propose_transfer_checked()` introduzem contratos, estados tipados e ledger de proposta. | Falta validar cobertura, época, frescor, read-set, cobrança antes da chamada e atestação independente de fechamento; o snapshot de `BudgetLedger` não é ledger de execução.
| Runtime | `research/symbiont_v2/core.py` mantém wrappers legados e a barreira conceitual proposta–execução. | O modo estrito ainda chama o caminho legado depois do preflight. A telemetria de probes, resets e custo precisa ser real, monotônica e separada de chamadas autorizadas.
| Executor externo | Os relatórios Stage 5 e `remaining_plan/01-external_executor.md` definem o protocolo. Não há ainda o conjunto `execution_contracts.py`, `ledger.py`, `executor.py` e `recovery.py`. | Falta implementar envelope externo, nonce durável, TTL, revogação, recibos por passo, `FAILED_PARTIAL`, `UNKNOWN_OUTCOME`, recovery e bloqueio de retry.
| Oráculo | O benchmark atual possui `IndependentOracle`, mas a independência é uma propriedade a preservar e ampliar. | Falta oráculo declarativo para pré-condições, ordem, recursos, custos, épocas e efeitos proibidos, sem importar transições do runtime.
| Holdout estendido | O plano em `remaining_plan/02-extended_holdout.md` define manifestos público/privado, proxy serializado e três repetições. | Não existem ainda `extended_contracts.py`, `extended_fixtures.py`, `public_host.py`, `extended_runner.py`, schema v2, manifestos e resultados.
| Mutações | `remaining_plan/03-mutation_gates.md` define mutações críticas e testes matadores. | Falta harness em cópia temporária. Mutantes sobreviventes devem manter a rodada `not_proven`.
| Caminho humano/social | O contrato congelado `research/symbiosis_utility_contract.json` exige tarefa, usuários, baseline convencional, condição sem simbionte, métricas de benefício, privacidade e acessibilidade. | Não há evidência de usuário ou comparação social nesta campanha. Falta protocolo preregistrado, consentimento, critérios de inclusão, coleta minimizada, análise cega e pacote de resultados separado.

Os relatórios anteriores são planos, não evidência de implementação. Em particular, a presença de tipos no `stage5.py`, de testes que passam ou de um digest não demonstra que uma chamada externa foi autorizada corretamente.

## 3. Menor sequência implementável

### Fase 0 — congelamento e baseline

Antes de criar uma nova semântica, registrar commit, versões dos contratos, Python, comandos, seeds, digests, ambiente e os arquivos v1. Reexecutar:

```bash
PYTHONPATH=research python3 -m unittest research.test_holdout_adversarial -v
PYTHONPATH=research python3 -m unittest research.test_stage5_synthesis -v
PYTHONPATH=research python3 -m unittest discover -s research -p 'test_*.py'
PYTHONPATH=research python3 -m research.holdout_adversarial
```

Os quinze registros históricos, a classificação histórica e o schema v1 não podem ser reescritos. A saída desta fase é um pacote `baseline` que registra resultados positivos, negativos, `UNSUPPORTED`, falhas e limitações; não é uma autorização para alterar o holdout.

### Fase 1 — fechar proposta, autoridade e execução

Implementar somente os objetos necessários para uma execução de teste separada:

- `ExecutionEnvelope`, `ExecutionReceipt` e `RecoveryAttestation`, com canonicalização, versão e validação em `research/stage4/execution_contracts.py`;
- ledger encadeado, append-only e monotônico em `research/stage4/ledger.py`;
- `ExternalExecutorAdapter` em `research/stage4/executor.py`;
- reconciliação independente em `research/stage4/recovery.py`;
- orquestração pós-decisão em `research/stage4/runner.py`.

O envelope deve vincular `proposal_digest`, `action_sequence_digest`, `host_context_digest`, objetivo, host, epoch, operação, tentativa, nonce, TTL, custo, política de retry e snapshot de revogação. O runtime não recebe a chave da autoridade, não assina, não reserva nonce, não escolhe retry e não recebe o executor como argumento.

O executor deve rejeitar ausência ou adulteração de autoridade antes de qualquer efeito. Deve verificar assinatura e proveniência, escopo, relógio monotônico, revogação, limites residuais e nonce antes do despacho. A reserva do nonce é durável e ocorre antes do primeiro efeito. `reset()`, reboot, `bind()` e recovery não diminuem contadores nem liberam replay.

A matriz de execução deve distinguir:

- `FAILED_PARTIAL + RECOVERY_REQUIRED` quando um prefixo foi comprovadamente comprometido e um passo posterior falhou;
- `UNKNOWN_OUTCOME + RECOVERY_REQUIRED` quando há timeout, crash, ACK ausente ou recibo conflitante;
- `RETRY_BLOCKED` sem atestação de recovery válida;
- nova tentativa somente com envelope, nonce, TTL, `attempt_no` e escopo residual novos, encadeados ao ledger resolvido.

### Fase 2 — holdout temporal e estrutural

Criar uma versão nova e aditiva do holdout, sem tocar em `holdout_benchmark_v1.json`:

- `research/stage4/extended_contracts.py` para observação v2, epoch, reset, revalidação e contrato estrutural;
- `research/stage4/extended_fixtures.py` para fixtures declarativas e IDs opacos;
- `research/stage4/public_host.py` para proxy serializado sem `_inner`, callbacks ou estado privado;
- `research/stage4/independent_oracle.py` para tabela privada de transição;
- `research/stage4/extended_runner.py` e `extended_results.schema.json`;
- `research/evidence/extended_holdout_v1/manifest_public.json`, `manifest_private.json` e `results.jsonl`.

O runtime recebe somente o manifesto público. O oráculo recebe a verdade privada apenas depois de o runtime fechar a decisão. Os casos mínimos são: controle positivo com fechamento externo; alias; conflito; digest inválido; stale/replay; troca de binding; reset não verificado; pré-condição falsa; ordem invertida; recurso ausente ou abaixo do piso; custo desconhecido/overrun; deriva latente; falha parcial e ACK ausente. IDs, ações, campos, números, unidades e enumeração devem ser permutados.

A regra para estado latente é de não alegação: se duas verdades privadas têm a mesma superfície pública, o runtime deve produzir a mesma decisão pública. O oráculo pode divergir depois, mas o runtime não pode afirmar que detectou o campo oculto.

### Fase 3 — mutações e fechamento host-only

Adicionar testes focados, propriedades geradas e mutações em cópia temporária. A campanha deve rodar três vezes em processos limpos e comparar bytes normalizados de status, motivos públicos, digests, contadores e traços. Um mutante não compilável não conta como mutante morto.

Só após os gates da seção 5 se publica uma conclusão host-only. Se qualquer limite duro falhar, a classificação é `not_proven`. Se os gates mecânicos passarem, mas não houver evidência humana independente, a classificação máxima é `mechanism_only`.

### Fase 4 — caminho social mínimo, separado

A etapa 7 deve começar por **uma tarefa humana estreita**, não por uma tese sobre capacidades gerais. A opção de menor escopo é testar a transferência de uma rotina de comunicação ou preferência configurável entre duas interfaces diferentes, com dados mínimos e sem permitir ação externa automática. O estudo deve ter:

1. protocolo, hipótese, análise e baseline congelados antes dos resultados;
2. condição convencional, condição sem simbionte e condição HERUS, com ordem randomizada ou contrabalanceada;
3. tarefa repetível, critério de sucesso e falhas definidos antes da coleta;
4. métricas de erro, tempo ou esforço e dependência de dispositivo/interface;
5. consentimento, possibilidade de cancelamento, retenção mínima, pseudonimização e política de exclusão;
6. teste de acessibilidade e um caminho de recuperação compreensível quando o sistema se abstém ou falha;
7. separação entre `raw human observations`, análise agregada e interpretação.

Arquivos prováveis, em uma árvore nova e não misturada ao holdout:

```text
research/social/utility_protocol_v1.json
research/social/runner.py
research/social/results.schema.json
research/evidence/stage7_social_v1/protocol.json
research/evidence/stage7_social_v1/baseline_results.jsonl
research/evidence/stage7_social_v1/raw_results.jsonl
research/evidence/stage7_social_v1/analysis.md
research/evidence/stage7_social_v1/privacy_accessibility.md
```

Essa fase não deve reutilizar `oracle_reason`, fixtures privadas ou uma taxa de transferência como substitutos de opinião, desempenho ou benefício de pessoas. Uma tarefa humana bem-sucedida não corrige um hard failure de autoridade ou segurança.

## 4. Separação de evidências

| Classe | Pergunta que responde | Artefatos permitidos | Não responde |
|---|---|---|---|
| **Host-only** | O mecanismo respeitou contratos, abstention, custo, autoridade, recovery e independência no escopo congelado? | traces públicos, ledger, manifestos, digests, resultados do oráculo pós-decisão, mutações e logs de execução de teste. | Se a tarefa é útil, acessível, desejável ou melhor para uma pessoa real.
| **Oráculo privado** | Qual era a verdade declarativa da fixture após a decisão do runtime? | tabela privada, transições independentes, `oracle_verdict` e `oracle_reason` separados. | O que o runtime sabia durante a proposta; não pode corrigir seu `runtime_reason`.
| **Humana/social** | O uso em uma tarefa definida melhora um resultado humano em relação às condições congeladas? | consentimento, protocolo, baseline, no-symbiont, medidas de erro/tempo/esforço/dependência, falhas, privacidade e acessibilidade. | Segurança física geral, inteligência geral ou generalização para qualquer tarefa.

`runtime_reason` deve conter somente fatos sustentáveis pela superfície pública. `oracle_reason` permanece pós-decisão e separado. Um digest demonstra integridade dos bytes; não demonstra autenticidade, completude, fechamento de efeitos, autoridade ou benefício.

## 5. Critérios de aceitação

### 5.1 Gate mecânico da etapa 6

A etapa 6 só passa se todos os itens seguintes forem verdadeiros:

- o baseline v1 e seus quinze registros permanecem intactos e reproduzíveis;
- alias, conflito, stale, replay, digest inválido, campo ausente e fechamento sem atestação produzem a saída tipada correta, proposta nula quando exigido e zero execução autorizada;
- `UNKNOWN` nunca vira custo zero, ledger `OPEN` ou `SUPPORTED`;
- cobrança de probe, reset, passo, retry e recovery ocorre antes da chamada e é monotônica;
- overrun fecha novas operações;
- construção de proposta mantém `proposal_execute_calls == 0`, `authorized_calls == 0`, `committed_calls == 0`, `external_effect_count == 0` e `step_count == 0`;
- envelope ausente, inválido, expirado, revogado ou fora de escopo é rejeitado antes do efeito;
- nonce é único e durável; replay não produz segunda chamada;
- falha parcial e resultado desconhecido ativam recovery e não disparam retry automático;
- recuperação é independente, não é `reset()` e não declara rollback que não foi observado;
- o oráculo não importa nem chama `core.py`, `stage5.py`, `Effect.from_states()`, `_apply()`, `WorldModel` ou `Goal.satisfied()`;
- três repetições em processos limpos produzem resultados determinísticos;
- cada mutação crítica é morta por um teste focado e por uma variante renomeada;
- todos os limites duros permanecem zero: autoridade não autorizada, falso consenso conhecido, execução insegura, ação externa sem revisão, replay aceito, overrun silencioso, retry após falha parcial e violação de revogação.

### 5.2 Gate social da etapa 7

A etapa 7 não passa por uma demonstração ou por relatos de preferência. Ela exige que o protocolo registre, antes da coleta, cenário-alvo, população ou stakeholders, tarefa, baseline convencional, condição sem simbionte, análise e métricas. A análise deve reportar resultados e falhas, não somente médias positivas.

A evidência social é elegível somente se houver melhora mensurada em pelo menos uma dimensão prevista no contrato congelado, sem regressão inaceitável nas demais, e se os controles de privacidade, acessibilidade, consentimento e recuperação forem avaliados. Mesmo nesse caso, a conclusão deve ser limitada à tarefa, população, interfaces, período e protocolo observados.

O resultado `useful_symbiosis` só pode ser emitido se todas as dimensões obrigatórias e hard limits do contrato `herus-symbiosis-utility-v1` passarem. Se a etapa 6 passar sem a etapa 7, o resultado é `mechanism_only`; se faltar uma dimensão obrigatória ou houver hard failure, é `not_proven` [3].

## 6. Testes e mutações mínimos

A suíte futura deve incluir, pelo menos, os seguintes casos:

| ID | Caso | Mutação matadora |
|---|---|---|
| `BASE-01` | Regressão dos quinze registros v1 | reescrever ou reinterpretar o holdout histórico |
| `SEP-01` | Proposta não executa | chamar executor dentro de `propose_transfer_checked()` |
| `AUTH-01/02/03` | ausência, escopo adulterado e autoridade circular | aceitar Skill, `True` ou digest do runtime como autorização |
| `NONCE-01/02` | reserva antes do efeito e replay | reservar depois do despacho ou esquecer nonce em reboot |
| `TTL-01/02` | expiração antes e durante a execução | validar TTL somente no início |
| `REVOKE-01/02` | revogação antes e entre passos | ignorar revogação ou fazer rollback fictício |
| `UNKNOWN-01/02` | ACK ausente e recibos conflitantes | converter desconhecido em zero, parcial ou commit |
| `REC-01/02/03/04` | recovery committed, não committed, parcial e conflitante | aceitar recovery do próprio executor ou limpar a trava com reset |
| `COST-01..08` | custo desconhecido, aliases, exceção, overrun e cobrança pré-chamada | custo ausente como zero/infinito ou ledger resetável |
| `ABS-01..06` | alias, conflito, stale, digest inválido e preflight | remover abstention ou sondar antes do gate |
| `OBS-01/04` | mesmo delta público com verdade privada distinta | aceitar digest próprio como fechamento |
| `ORACLE-01` | independência e separação de razões | importar transição do runtime ou copiar `oracle_reason` |
| `VAR-01` | IDs, ordem, ações e chaves renomeados | reconhecer `m6a`, `m7a`, `m8a`, `m9a` ou palavras da fixture |
| `HUM-01` | protocolo com baseline e no-symbiont congelados | remover a condição de comparação ou medir apenas satisfação subjetiva |
| `HUM-02` | cancelamento, abstention e recuperação acessíveis | tratar falha como sucesso ou excluir participantes que falharam |

O teste de separação deve observar o trace imediatamente após a proposta, antes do oráculo. Um efeito executado e depois revertido continua sendo falha. A prova de independência deve verificar imports, payloads e dependências, não somente PIDs diferentes.

## 7. Riscos de circularidade e controles

**Circularidade do delta.** Runtime e oráculo podem compartilhar `Effect.from_states()` ou a mesma tabela de efeitos e concordar sobre uma projeção incompleta. Usar tabela declarativa independente, efeitos ocultos fora do host público, processo separado e mutação própria do oráculo.

**Circularidade do digest.** SHA-256 prova consistência dos bytes recebidos, não verdade, cobertura, autenticidade ou ausência de efeitos ocultos. Registrar emissor, versão, escopo, raiz de confiança e proveniência. Um digest criado pelo runtime nunca habilita `SUPPORTED` sozinho.

**Circularidade da autoridade.** Um token assinado pelo próprio runtime apenas confirma a própria decisão. A autoridade precisa de emissor, chave, epoch e política externos. O executor rejeita proveniência não verificável.

**Circularidade da recuperação.** O executor que perdeu o ACK não pode atestar sozinho que nada ocorreu. Divergência entre fontes permanece `RECOVERY_CONFLICT`/`RECOVERY_REQUIRED`.

**Confusão entre probe e execução.** `host.execute()` durante discovery pode mutar o host mesmo sem autoridade. Exigir `NON_MUTATING` ou uma autorização de sondagem específica e manter contadores separados. `reset()` não é rollback.

**Sobreajuste ao holdout.** Reconhecer IDs ou nomes de casos produz uma vitória artificial. Usar splits D/V/H congelados, IDs opacos, permutações, seeds distintas e mutações estruturais.

**Abstenção universal.** Sempre abster-se pode passar nos negativos sem demonstrar competência. Exigir o controle positivo representável, com atestação externa e sem execução. Esse controle não compensa hard failure negativo.

**Circularidade social.** Escolher apenas usuários que já acreditam na proposta, trocar o baseline depois, excluir falhas de acessibilidade ou usar apenas satisfação subjetiva pode fabricar benefício. Congelar o protocolo antes dos resultados, contrabalancear condições, registrar falhas e analisar todos os participantes elegíveis.

**Vazamento de privacidade.** Logs host-only ou sociais podem carregar áudio, transcript, identidade, localização ou conteúdo sensível. Coletar apenas IDs pseudônimos, eventos e métricas necessárias; manter conteúdo fora do log e documentar retenção, exclusão e acesso.

## 8. O que não pode ser alegado

Não se pode alegar que:

- `SUPPORTED`, uma Skill verificada, `transfer() == True`, uma proposta ou um digest possuem autoridade física;
- um digest prova a verdade do host, completude da observação, ausência de efeitos ocultos ou autenticidade de sensor;
- nonce prova idempotência, TTL prova ausência de efeito, revogação desfaz commit ou `reset()` recupera estado parcial;
- `expected_final`, ACK, contagem de chamadas ou retorno booleano provam o mundo externo;
- três repetições, alta taxa de transferência, alta taxa de abstenção ou todos os mutantes definidos provam generalização geral;
- um benchmark host-only prova utilidade social, acessibilidade real, privacidade de produto, eficácia clínica, segurança física geral, consciência, autonomia ou substituição de ferramentas convencionais;
- uma tarefa humana estreita prova benefício para outras tarefas, populações, dispositivos ou contextos;
- qualquer resultado desta frente prova **AGI**, inteligência geral, compreensão aberta, consciência ou capacidade equivalente à de uma pessoa.

A redação permitida após uma etapa 6 aprovada é: **“No escopo dos hosts, contratos, épocas, budgets, fixtures, autoridade de teste e falhas congelados, o mecanismo evitou os falsos consensos e as execuções não autorizadas especificados.”** A redação permitida após uma etapa 7 aprovada continua limitada à tarefa e à população estudadas: **“Nesta tarefa e protocolo, houve a diferença medida em relação às condições congeladas.”**

## Referências

[1]: ../../../docs/53-BENCHMARK-HOLDOUT-ADVERSARIAL.md "Etapa 4 — benchmark holdout adversarial"
[2]: ../../../docs/54-CORRECAO-CONTRATO-OBSERVABILIDADE.md "Etapa 5 — contrato de observabilidade e decisão checked"
[3]: ../../symbiosis_utility_contract.json "Contrato congelado de utilidade da simbiose HERUS"
[4]: ../stage4_plan/00-synthesis.md "Síntese de integração da etapa 4"
[5]: ../stage5_plan/00-synthesis.md "Síntese arquitetural mínima da etapa 5"
[6]: 01-external_executor.md "Plano restante — executor externo e recuperação"
[7]: 02-extended_holdout.md "Plano restante — holdout temporal e estrutural estendido"
[8]: 03-mutation_gates.md "Plano restante — mutações e gates adversariais"
[9]: ../../symbiont_v2/core.py "HERUS Symbiont v2 research runtime"
[10]: ../../symbiont_v2/stage5.py "Tipos Stage 5 de decisão e ledger de orçamento"

<!-- Relatório produzido como auditoria e desenho implementável. Nenhum arquivo de código foi alterado nesta frente. -->
