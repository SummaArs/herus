# Auditoria adversarial de soundness código–documentação

**Escopo:** `docs/00-HERUS-MASTER.md`, `docs/51–55`, `research/symbiont_v2`, `research/stage4`, `research/stage5.py`, `research/holdout*`, `research/extended_holdout.py`, `research/social` e artefatos de evidência associados. **Nenhum código foi editado.**

**Commit auditado:** `fbf7471c4903fd6af285f9e6e6a0e10f7cb2a967`.

## Veredicto

**REPROVADO para qualquer alegação de segurança, `SUPPORTED`, execução externa autorizada ou “simbiose útil”.** O repositório é um protótipo host-only com várias ressalvas honestas, e o holdout histórico reproduz `not_proven`; porém há APIs que emitem estados positivos a partir de strings não autenticadas, um ledger de orçamento que não contabiliza a operação, um executor que não valida nem executa a ação descrita pelo envelope, um oráculo que confia nos próprios rótulos da fixture e validadores que aceitam adulteração de resultados. Há também números físicos publicados que não correspondem à saída atual de `tools/budget.py`.

A campanha existente passar não corrige isso: `PYTHONPATH=. python3 -m unittest discover -s research -p 'test_*.py'` retorna **142 testes, 1 skipped, 0 falhas**, e o holdout reproduz **15 registros, 9 hard failures e 0 violations**; isso demonstra somente que os asserts atuais são satisfeitos. A saída regenerada de `research.holdout_adversarial` é byte-a-byte igual a `research/evidence/holdout_benchmark_v1.json`, mas o próprio gerador contém as falhas descritas abaixo.

## Resumo dos achados

| ID | Severidade | Achado | Estado atual |
|---|---|---|---|
| S1 | Crítica | `STRICT` emite `SUPPORTED` sem atestação verificável | Falso positivo reproduzido |
| S2 | Alta | Ledger de orçamento é decorativo e não limita sondas/custo | Falso negativo de contabilização reproduzido |
| S3 | Crítica | Executor não vincula envelope ao plano, ignora custo e não chama alvo | “Execução autorizada” não demonstrada |
| S4 | Alta | Recuperação aceita atestação de operação/ledger alheios | `RECOVERY_RESOLVED` falsificável |
| S5 | Alta | `AppendOnlyLedger` é mutável; digest/proveniência retroage | Append-only não é garantido |
| S6 | Alta | m7/m8 do holdout são rótulos, não cenários executados | Claims de partial/overrun cosméticos |
| S7 | Alta | Oráculo “independente” confia em `expected_negative`/`oracle_reason` | Oráculo circular/falsificável |
| S8 | Alta | Validador aceita adulteração de contadores, status, oracle e hash | Evidência bruta não é protegida |
| S9 | Alta | Classificador promove relatório incompleto a `mechanism_only` | Contradição direta do contrato congelado |
| S10 | Média | Validador social ignora quase todos os controles declarados | “Protocolo pronto” é verificação cosmética |
| S11 | Alta | Hashes não vinculam contratos, host, origem e artefato | Proveniência frágil |
| S12 | Alta | Números RF publicados divergem do orçamento atual | Documentação stale/internamente inconsistente |
| S13 | Média | Repetições do holdout não variam o experimento | Repetição cosmética, sem seed |

---

## S1 — `STRICT` aceita contratos forjados e emite `SUPPORTED` sem prova

**Evidência:** `research/symbiont_v2/core.py:465–493` chama `valid_for_strict()` e, se os campos passarem, retorna `SafetyClaim.SUPPORTED`. Em `research/symbiont_v2/stage5.py:139–166`, `SkillObservabilityContract.valid_for_strict()` e `HostObservabilityContract.valid_for_strict()` verificam apenas nomes de schema, enums e que `closure_evidence_digest`/`contract_digest` sejam strings não vazias. Não há recomputação do digest, assinatura, vínculo com o `skill_id`, vínculo com `HostModel.digest`, emissor confiável, validade temporal ou consulta a atestação independente.

**Reprodução adversarial:** um contrato de Skill com `closure_evidence_digest="FORGED"`, `contract_digest="FORGED"` e um contrato de host equivalente foi aplicado a `m6a-opaque-01`, que esconde `damage += 1`. O resultado foi `PROPOSED / SUPPORTED`, com uma sonda executada (`probes=1`). O efeito proibido continua invisível. Isso contradiz o significado operacional dado a `SUPPORTED` em `docs/54-CORRECAO-CONTRATO-OBSERVABILIDADE.md:15–24,67–72`; a ressalva de `docs/54:53–57` reconhece que digest não é autenticidade, mas o código já usa o estado positivo.

**Correção concreta:** separar `CONTRACT_PRESENT`, `ATTESTED` e `SUPPORTED`; `SUPPORTED` só pode ser produzido por um verificador externo que valide assinatura, escopo, versão, `skill_digest`, `host_digest`, fechamento de efeitos, janela/TTL e autoridade. Valores não vazios não são evidência. Contrato ausente, stale, mismatched ou não autenticado deve resultar em `UNSUPPORTED_BY_CONTRACT` ou `SAFE_BUT_UNPROVEN`, nunca em `SUPPORTED`.

**Teste necessário:** mutar cada digest para uma string aleatória, trocar o contrato de host por outro host, mudar um campo depois de assinar e executar o caso opaco em `STRICT`; exigir `proposal is None`, `safety_claim == NONE` e zero `host.execute()` antes de qualquer probe. Adicionar um teste que prove que o contrato forjado acima não promove `m6a-opaque-01`.

## S2 — `BudgetLedger` não é o orçamento da operação

**Evidência:** `core.py:437–476` cria um ledger, mas não chama `with_probe()`, `with_reset()`, `with_step()` (inexistente) nem registra custo durante `discover()`/`propose_transfer()`. `limits.cost_max` é usado somente para rejeitar `STRICT` quando é `None`; nenhum custo é comparado ao limite. `propose_transfer()` chama `discover(host)` antes de aplicar seu `max_probes` aos candidatos (`core.py:377–380`), e `discover()` usa `self.budget.max_probes` (`core.py:270`). O ledger devolvido pode permanecer `OPEN`, com zero contadores, após sondas reais.

**Reprodução adversarial:** em `COMPATIBILITY`, com `BudgetLimits(probe_max=0, reset_max=0, step_max=1, cost_max=0)`, o alvo de controle executou uma sonda (`host_probe_count=1`), enquanto a decisão devolveu ledger com `probe_count=0`, `reset_count=0`, `cost_actual=None`, `budget_state=OPEN`, `event_seq=0`. Chamadas sucessivas criam um novo ledger e não consomem um orçamento cumulativo.

**Correção concreta:** tornar o ledger um recurso de execução persistente, com identidade de escopo e estado monotônico armazenado no runtime/adaptador; consumir a permissão **antes** de cada `observe`, `execute`, `reset`, passo e recuperação; cobrar custo observado e negar quando o custo é desconhecido ou excede `cost_max`; impedir que `max_probes` seja apenas um slice posterior. O caminho `STRICT` deve rejeitar `probe_max=0` antes de qualquer chamada ao host.

**Testes necessários:** orçamento zero não pode chamar `execute`; custo `actual > max_cost` deve gerar `COST_OVERRUN`; segundo pedido no mesmo `budget_scope_id` deve ser bloqueado; `reset_max`, `step_max`, `retry_max` e `recovery_max` devem ser exercitados; os contadores da decisão devem igualar os contadores observados no host e no executor.

## S3 — Executor não valida o envelope, ignora custo e não executa o plano

**Evidência:** `research/stage4/executor.py:20–45` verifica apenas `no_automatic_retry`, `authority_id`, TTL inicial, nonce monotônico e que `max_steps/max_cost` sejam positivos. Não compara `proposal_digest`, `action_sequence_digest` ou `host_context_digest` com qualquer objeto confiável; esses campos são aceitos como texto arbitrário. O loop incrementa `effects` uma vez por passo, sem receber ou despachar qualquer ação/atuador. `max_cost` não limita o loop: um envelope com `max_steps=3,max_cost=1` retorna `COMMITTED` e `effects=3`. O TTL é verificado uma vez antes do loop, embora `clock` avance a cada passo.

Isso não satisfaz a descrição de `docs/55-ETAPAS-FINAIS-EXECUTOR-HUMANO.md:5–16`, que apresenta autoridade, TTL, recibos por passo e estados de execução como uma barreira operacional. Os testes atuais só verificam contadores sintéticos, não a ligação entre envelope e ação real.

**Correção concreta:** o executor deve receber uma proposta registrada e um dispatcher externo; validar todos os digests contra bytes canônicos e uma autoridade confiável; validar `operation_id`, sequência, host e nonce; reservar e cobrar custo por passo; revalidar expiração/revogação antes de cada passo; registrar rejeições e commits no ledger. Sem dispatcher real, nomear o componente como simulador de bookkeeping, não executor externo.

**Testes necessários:** envelope com cada digest alterado deve ser rejeitado; `max_steps=3,max_cost=1` não pode produzir três efeitos; expiração entre passos deve gerar estado parcial/unknown; uma ação registrada deve ser a única ação chamada; verificar que o recibo contém digest do plano, host, nonce e parent válido.

## S4 — Recuperação aceita atestação de operação alheia

**Evidência:** `executor.py:51–59` só rejeita atestação cuja `source_id` seja o próprio executor e só valida se `conclusion` pertence a um conjunto. Não verifica `operation_id`, `attempt_no`, `ledger_digest`, `committed_steps`, nonce, recibos ou se existe operação parcialmente concluída no estado atual.

**Reprodução:** logo após criar um executor novo, `recover(RecoveryAttestation('wrong-op', 99, 'RESOLVED_COMMITTED', 'random', 'wrong-ledger', 999))` retorna `RECOVERY_RESOLVED`.

**Correção concreta:** guardar contexto da tentativa ativa; aceitar somente atestação assinada por fonte independente permitida, com `operation_id`, `attempt_no`, digest do ledger e número de passos consistentes com recibos; atestação desconhecida ou conflitante deve permanecer `RECOVERY_REQUIRED`.

**Teste necessário:** variar cada campo da atestação, inclusive `committed_steps`, e exigir rejeição; testar conflito entre dois atestadores e replay de uma atestação antiga.

## S5 — `AppendOnlyLedger` não é append-only nem tamper-evident

**Evidência:** `research/stage4/ledger.py:10–15` deixa `events` como lista pública mutável. `append()` faz cópia rasa do evento; `snapshot()` também faz cópia rasa da lista. Eventos aninhados podem ser mutados depois do append, alterando retroativamente o digest do ledger e o digest usado como `parent`. Não existe `verify_chain()`, assinatura, armazenamento imutável ou rejeição de mutação.

**Reprodução:** depois de `append({'event': {'x': 1}})`, alterar `ledger.events[0]['event']['x']=9` muda `ledger.digest()` e também aparece no `snapshot()`. Logo, o digest não é uma prova estável da sequência observada.

**Correção concreta:** canonicalizar e congelar cada evento (estrutura imutável ou bytes canônicos), copiar profundamente na entrada/saída, manter `entry_digest` e `parent_entry_digest`, e disponibilizar verificação de cadeia que falhe fechado. O estado serializado deve ser validado antes de ser aceito.

**Teste necessário:** tentativa de mutação direta não pode alterar um evento aceito; snapshot modificado não pode modificar o ledger; `verify_chain()` deve rejeitar alteração de payload, seq, parent ou ordem; digest deve permanecer estável após a chamada de `snapshot()`.

## S6 — m7/m8 do holdout não testam partial failure nem cost overrun

**Evidência:** em `research/holdout_adversarial.py:62–101`, a proposta é construída, o oráculo é chamado, e o executor instrumentado só é acionado quando `oracle_status == "SUPPORTED"` (`:72–73`), isto é, no controle. Portanto `m7a-partial-01` nunca roda com `ack=False` nem `partial=True`; `m8a-budget-01` nunca passa por qualquer orçamento declarado. Para m8, `holdout_hosts.py:20–21,104–105` apenas define `cost=2`, e `holdout_adversarial.py:99` calcula `cost_actual=probe_count*fixture.cost`, enquanto `budget_exhausted=fixture.cost > 1`; o limite `1` é hard-coded e não existe um campo de orçamento da fixture.

Ainda assim, `run_case()` transforma qualquer fixture negativa com proposta em `FAIL_UNSAFE` (`:83–85`), e o artefato registra nove hard failures incluindo m7 e m8. Os testes verificam os rótulos (`test_holdout_adversarial.py:18–25`), não a ocorrência de ACK ausente ou de cobrança acima de um limite.

**Correção concreta:** modelar no fixture um orçamento declarado e um plano de execução; executar o caminho parcial/ACK ausente no adaptador, registrar recibos e garantir que a execução externa continue zero na etapa de proposta; emitir `PARTIAL_FAILURE`, `UNKNOWN_OUTCOME` e `COST_OVERRUN` somente após a respectiva condição ser observada. Separar `oracle_verdict`, `proposal_status` e `execution_status` em vez de mapear todos os negativos para `FAIL_UNSAFE`.

**Testes necessários:** m7 deve chamar o executor com `ack=False` ou `partial_at=1` e obter exatamente o estado correspondente; m8 deve declarar `max_cost`, observar custo maior e bloquear; exigir que a trilha seja diferente da do controle e que `proposal_execute_calls == 0` permaneça verdadeiro.

## S7 — Oráculo dito independente é alimentado pelo próprio rótulo da fixture

**Evidência:** `IndependentOracle.evaluate()` (`holdout_adversarial.py:20–26`) não inspeciona `proposal`, `true_effects`, estado privado ou qualquer relação entre ação e efeito. Se `proposal is None`, retorna `ABSTAIN` usando `fixture.oracle_reason`; caso contrário, retorna `FAIL_UNSAFE` se `fixture.expected_negative` e `SUPPORTED` caso contrário. `Fixture` já contém `expected_negative`, `oracle_verdict` e `oracle_reason` (`holdout_hosts.py:9–20`). Assim, o oracle não descobre a verdade depois da proposta; copia metadados.

**Reprodução:** copiar a fixture opaca e alterar somente `expected_negative=False` e `oracle_reason='SUPPORTED_CONTROL'` faz o oracle retornar `('SUPPORTED','SUPPORTED_CONTROL','SAFE')`, apesar de `true_effects['q7']` continuar contendo `damage += 1`.

**Correção concreta:** manter a verdade privada fora da entrada do runtime, mas fazer o oracle avaliar a proposta contra uma tabela privada imutável de efeitos/autoridade, ou contra um executor/oráculo independente que não aceite os campos de decisão do runtime. `expected_negative` deve ser expectativa para assert, não entrada da decisão.

**Teste necessário:** mutar `expected_negative`, `oracle_verdict` e `oracle_reason` sem alterar a verdade privada; o veredicto do oracle deve continuar igual. Adicionar um teste de independência que falhe se o oracle importar ou chamar lógica do runtime e outro que compare a proposta real com `true_effects`.

## S8 — `validate_result()` aceita adulteração material do benchmark

**Evidência:** `research/stage4/contracts.py:70–84` só verifica campos obrigatórios, não-negatividade, `proposal_execute_calls == 0`, uma condição de orçamento e ausência de efeito externo para negativos. Não recomputa `raw_trace_digest`, não compara status com oracle/fixture, não valida `cost_actual`, não garante que `probe_count`/`reset_count` correspondam à trilha, não exige `repeat` no domínio esperado e não valida cobertura única de fixtures.

**Reprodução:** para um registro válido, alterar isoladamente `proposal_status`, `oracle_verdict`, `cost_actual`, `probe_count`, `raw_trace_digest` ou `repeat` não produz qualquer violation. O JSON Schema (`holdout_benchmark_schema.json`) também valida apenas tipos/padrões, não invariantes entre campos.

**Correção concreta:** validar a trilha canônica antes do registro; recomputar digest; derivar contadores e custos da trilha, nunca aceitar os números declarados; impor matriz de estados permitidos por fixture; verificar três repetições por fixture, `split`, unicidade e consistência de oracle/status/expected-negative.

**Testes necessários:** tabela de mutações para cada campo; cada mutação material deve ser rejeitada. Incluir teste de digest incorreto, registro extra, repetição ausente/duplicada, custo alterado e `FAIL_UNSAFE` em controle.

## S9 — `classify()` viola o fail-closed do contrato de simbiose

**Evidência:** `research/symbiosis_utility.py:87–97` retorna `mechanism_only` sempre que as métricas de mecanismo estão presentes, salvo violation contendo a substring `hard_limit`. Ele não considera dimensão obrigatória ausente, controles de segurança ausentes, privacidade incompleta ou reprodutibilidade incompleta ao escolher `mechanism_only`.

**Reprodução:** um relatório contendo apenas as métricas de mecanismo retorna `mechanism_only` apesar de `validate_report()` listar dimensões e controles ausentes. Mesmo um relatório completo em todas as dimensões, mas com `independent_stop_or_recovery=False`, retorna `mechanism_only` em vez de `not_proven`.

Isso contradiz `docs/52-DEFINICAO-SIMBIOSE-UTIL.md:22–30` e `research/symbiosis_utility_contract.json:91–95`, que dizem que qualquer dimensão obrigatória faltante produz `not_proven`; `mechanism_only` é reservado ao caso em que mecanismo passa e valor humano ainda não foi demonstrado.

**Correção concreta:** primeiro executar validação estrutural e de todos os controles obrigatórios; qualquer dimensão/controle ausente ou hard limit desconhecido deve resultar em `not_proven`. Só retornar `mechanism_only` quando mecanismo estiver completo, segurança/privacidade/reprodutibilidade estiverem completas e apenas a demonstração de valor humano estiver ausente ou não provada.

**Testes necessários:** casos mínimos com safety ausente, privacy ausente, reproducibility ausente, um controle falso e um hard limit ausente; todos devem resultar `not_proven`. Um relatório completo de mecanismo e controles, sem `human_value`, deve resultar `mechanism_only`.

## S10 — `social.runner` não verifica os controles que a documentação diz incluir

**Evidência:** `research/social/runner.py:10–17` verifica apenas `status`, o conjunto de três condições e três métricas primárias. Remover integralmente `required_controls`, `progression_thresholds`, `analysis_rule` ou `prohibited_claims` de `utility_protocol_v1.json` ainda resulta em `validate_protocol(...) == ()`. Não há participante, coleta, análise estatística, revisão de acessibilidade ou verificação de limiares no código.

`docs/55-ETAPAS-FINAIS-EXECUTOR-HUMANO.md:28–40` está correto ao dizer que não há dados humanos, mas é exagerado tratar o arquivo como protocolo validado se o runner não impõe seus próprios controles.

**Correção concreta:** validar schema completo e tipos; exigir todos os controles, thresholds, regra de análise, proibições e versionamento; separar `READY_FOR_EXTERNAL_REVIEW` de protocolo semanticamente validado. O runner não deve declarar prontidão se o contrato foi truncado.

**Testes necessários:** remover cada campo obrigatório e alterar tipos/thresholds; todos devem bloquear. Testar que uma condição duplicada, uma condição desconhecida ou um limiar negativo também falham.

## S11 — Hashes e proveniência não vinculam o objeto que alegam proteger

**Evidência:** `AbstractSkill.digest` (`core.py:130–141`) não inclui `source_host` nem `observability_contract`; portanto adicionar, trocar ou revogar o contrato de observabilidade não altera o digest da Skill. `Evidence.provenance` (`core.py:81–99`) é apenas uma string fornecida pelo host/runtime, sem emissor, assinatura ou cadeia. `Observation.digest` autentica somente a representação local auto-declarada, não sensor, origem ou integridade do canal. `TransferProposal` não possui digest próprio, embora o envelope de execução exija `proposal_digest`, `action_sequence_digest` e `host_context_digest`.

O artefato holdout exige apenas os campos de resultado; não contém `code_commit`, digest do contrato, digest de fixtures, seed ou comando, apesar de esses serem obrigatórios no contrato de reprodutibilidade (`symbiosis_utility_contract.json:80–89`). `raw_trace_digest` pode ser trocado livremente porque o validador só exige 64 hexadecimais.

**Correção concreta:** definir uma serialização canônica única; incluir no digest todos os campos normativos, incluindo contratos e escopo; vincular Evidence a host/epoch/sequence e emissor; assinar atestações; implementar digest do proposal e verificar cada digest no executor; registrar no artefato commit, versão do contrato, digest da fixture, seed, comando, ambiente e negativos.

**Testes necessários:** alterar `observability_contract` ou `source_host` e exigir mudança do digest da Skill; alterar Evidence/provenance/epoch e exigir rejeição; alterar uma linha do trace e exigir falha de digest; reconstruir o holdout em outro checkout e verificar todas as referências de proveniência.

## S12 — Números de RF publicados não correspondem ao orçamento atual

**Evidência:** `docs/00-HERUS-MASTER.md:253–258` publica, para Band/SF9, 2.305 m aberto, 1.155 m suburbano e **650 m denso urbano**, e anuncia 1,9 km em três saltos; `:151,156` publica **487 m** para voz e 650/487 = 33%. A execução atual de `python3 tools/budget.py` imprime, com `TX_DBM=22.0` (`tools/budget.py:35`), Band/SF9 **3.653 m, 1.831 m, 1.030 m**, voz **772 m urbano**, e três saltos **3,1 km**. A razão de 33% permanece, mas os valores absolutos não.

O próprio `tools/budget.py` ainda contém duas eras: a seção principal (`:235–257`) imprime frame legado de 26 B e energia com `I_SX_TX14`, enquanto a seção corrigida (`:147–177`) usa 34 B/38 B e airtime de 246,8 ms. Isso torna a afirmação de `docs/00:3,120–143` — todos os números reproduzíveis a partir de um ledger fechado — falsa como documentação de versão única.

**Correção concreta:** escolher uma revisão física única (TX14 ou TX22), atualizar os valores do Master, remover a seção legada ou marcar explicitamente como histórico, e gerar um snapshot versionado do orçamento que o CI compare com os números publicados. Separar claramente potência legal, potência do PA e perdas/ganhos efetivos.

**Testes necessários:** CI deve executar `tools/budget.py`, falhar diante de divergência de airtime/range/energia documentada e verificar que não coexistem dois frame ledgers contraditórios. Adicionar teste de regressão para Band/SF9, voz, Tier 0.5 e três-hop.

## S13 — As três repetições do holdout são determinísticas e parcialmente cosméticas

**Evidência:** `run_campaign()` repete o mesmo fixture três vezes (`holdout_adversarial.py:105–106`), sem seed, randomização, mudança de ordem, perturbação temporal ou nova instância com cenário diferente. A única diferença garantida no `raw_trace_digest` é que `repeat` é inserido no JSON canônico (`:75–82`); o trace do executor negativo continua vazio e os campos de comportamento são iguais.

Isso pode ser uma repetição de reexecução determinística, mas não deve ser descrito como robustez estatística ou campanha adversarial independente. O schema não registra seed nem versão/digest da fixture.

**Correção concreta:** distinguir `run_digest` (comportamento observável) de `record_id` (fixture + repetição); registrar seed, versão do host e parâmetros; se a intenção for determinismo, verificar que os run digests sejam iguais; se a intenção for robustez, variar explicitamente o cenário.

**Testes necessários:** exigir seed/fixture digest no artefato; testar igualdade de traces independentemente de `repeat`; adicionar variantes de ordem, stale epoch, custo e ACK e verificar que não são apenas rótulos.

---

## Mudanças prioritárias, em ordem

1. **Bloquear a emissão de `SUPPORTED`** até existir verificação criptográfica/independente vinculada a Skill, host, proposta e executor; qualquer contrato textual deve ser não-promocional.
2. **Fazer o orçamento ser efetivo**, persistente e consumido antes da sonda/execução; remover contadores declarados que não derivam da trilha.
3. **Reescrever o executor como adapter de plano real ou renomeá-lo como simulador**; validar digests, custo, TTL por passo, nonce e recuperação vinculada.
4. **Corrigir o holdout e o oracle** para executar m7/m8 e calcular veredictos a partir de verdade privada independente, não de `expected_negative`.
5. **Tornar artefatos tamper-evident**, com digest recomputável, ledger imutável, commit/fixture/seed/comando e invariantes cross-field.
6. **Corrigir `classify()` e `social.runner`** para falhar fechado em dimensões e controles ausentes.
7. **Regenerar e versionar o orçamento físico**, eliminando a mistura de TX14/26 B com TX22/34–38 B.

## Testes mínimos para reabrir a auditoria

- Suite de mutação para contratos, proposta, host, ledger, oracle, status, contadores e hashes.
- Holdout com pelo menos: efeito oculto, alias, custo excedido, ACK ausente, partial commit, stale epoch, replay, revogação, ordem não comutativa, recurso consumível e contrato forjado.
- Propriedades: zero execução no caminho de proposta; orçamento nunca diminui; `SUPPORTED` exige atestação válida; digests mudam quando qualquer campo normativo muda; recuperação só resolve a operação correta.
- Validação semântica dos artefatos: cada contador deve ser derivado da trilha; cada digest deve ser recomputável; três repetições devem ter seed/identidade explícitos.
- Teste de contrato social removendo cada controle, threshold e campo de análise; qualquer remoção deve resultar em `BLOCKED`.
- CI de documentação que execute `tools/budget.py` e compare os números publicados com a revisão física escolhida.

## Conclusão

O material tem mérito ao preservar `not_proven` no holdout congelado e ao declarar que não há prova de benefício humano. No entanto, a auditoria adversarial encontra uma distância material entre essa cautela narrativa e o comportamento das APIs: os estados positivos não são criptograficamente nem semanticamente sustentados, os contadores não são contadores da operação, o executor não executa/valida o envelope, e os validadores permitem falsificação de evidência. Até as correções acima e os testes de mutação passarem, a única classificação defensável é **`not_proven`**, sem segurança de execução, sem `SUPPORTED` e sem simbiose útil.

## Evidências de execução

- `PYTHONPATH=. python3 -m unittest discover -s research -p 'test_*.py'`: 142 testes, 1 skipped, 0 falhas.
- `PYTHONPATH=. python3 -m research.holdout_adversarial`: 15 records, 9 hard failures, 0 violations; JSON regenerado igual ao artefato congelado.
- `python3 tools/budget.py`: valores atuais divergentes dos publicados no Master, conforme S12.
- Probes adversariais descritos em S1–S11 foram executados sem modificar o checkout.

