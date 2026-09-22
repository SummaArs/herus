# Auditoria adversarial de engenharia e integração

**Escopo:** qualidade de engenharia, modularidade, versionamento, testes, imports, compatibilidade, CI e proveniência do repositório HERUS, com foco em `docs/00-HERUS-MASTER.md`, `docs/52–55`, `research/symbiont_v2`, `research/stage4`, `research/symbiont_v2/stage5.py`, `research/holdout*` e `research/social`.

**Commit auditado:** `fbf7471c4903fd6af285f9e6e6a0e10f7cb2a967` (`research: complete executable final stages`).

**Conclusão executiva:** o repositório tem disciplina incomum para declarar limites e mantém uma boa suíte host-only. Entretanto, ainda não é um repositório profissional de integração. O verde atual prova principalmente que fixtures determinísticas e mocks obedecem aos próprios testes. Ele não fecha a fronteira entre proposta, autorização, executor e recuperação; não contabiliza orçamento real no caminho `checked`; não fornece isolamento independente do oráculo; tem imports que quebram quando o pacote é importado de forma normal; e o CI contém um caminho de falso-verde. O estado correto é **não pronto para release nem para qualquer alegação de segurança de execução**.

## 1. Linha de base reproduzida

Foram executados, sem editar código:

```text
./prove.sh --quiet                         -> exit 0
PYTHONPATH=research python3 -m unittest discover -s research -p 'test_*.py'
                                             -> 142 testes, OK, 1 skipped
make -C research test                      -> exit 0
python3 tools/provenance_audit.py ... --strict
                                             -> PROVENANCE MANIFEST VALID
```

O `prove.sh` reportou 74 invariantes da simulação e `ALL INVARIANTS HOLD`. Isso é um resultado útil, mas não deve ser confundido com integração completa: o CI e o script executam muitos testes de host, mocks e simulação, enquanto os defeitos abaixo passam porque não há asserções correspondentes.

Também foi reproduzido o seguinte:

```text
import research.audit_minds14_text  -> ModuleNotFoundError: fetch_minds14_sample
import research.fetch_minds14_batch  -> ModuleNotFoundError: fetch_minds14_sample
import research.semantic_ir_fuzz      -> ModuleNotFoundError: semantic_ir
```

Não existem tags Git (`git tag --list` retorna zero). O manifesto de software declara `trust_state: local_unattested` e `source_state: unattested_worktree_inputs`, portanto a própria proveniência ainda é explicitamente não atestada.

## 2. Resumo dos findings

| ID | Severidade | Finding | Estado | Impacto |
|---|---|---|---|---|
| F-01 | **Crítica** | O executor externo não autentica nem valida o envelope completo. | Reproduzido | Uma proposta com digests arbitrários pode ser executada; `max_cost`, replay semântico e vínculo operação/tentativa não são aplicados. |
| F-02 | **Crítica** | Recuperação e ledger são mutáveis, não duráveis e aceitam atestação forjada. | Reproduzido | `UNKNOWN_OUTCOME` pode ser resolvido sem verificar operação, ledger ou passos; o histórico pode ser alterado depois do append. |
| F-03 | **Crítica** | O ledger de orçamento do Stage 5 é cosmético e não acompanha `discover()`. | Reproduzido | A decisão reporta zero probes enquanto o caminho de síntese executa probes e resets; limites de custo, passos, retry e recovery não governam a operação. |
| F-04 | **Alta** | `SUPPORTED` depende de strings não vazias, não de um contrato autenticado e vinculado. | Reproduzido por inspeção | Qualquer objeto com `valid_for_strict()` verdadeiro e qualquer digest textual não vazio pode atravessar o gate estrito. |
| F-05 | **Alta** | O holdout não é independente no sentido forte e não possui isolamento de processo. | Insuficiente | Oráculo, runtime e verdade privada vivem no mesmo processo e no mesmo código de campanha; um bug de integração ou introspecção não é modelado. |
| F-06 | **Alta** | A campanha holdout não exercita os caminhos de falha que a documentação afirma medir. | Reproduzido | `m7a-partial-01` e `m8a-budget-01` são apenas rótulos retornados pelo oráculo; o executor instrumentado só é chamado para o controle positivo. |
| F-07 | **Alta** | O schema e o validador aceitam resultados semanticamente inválidos. | Reproduzido | Enums, relação entre campos, digest do trace e regras de abstention não são validados. O schema não vincula os bytes declarados ao digest. |
| F-08 | **Alta** | O pacote Python não é importável de maneira consistente. | Reproduzido | Mistura de imports absolutos de diretório e imports de pacote; scripts funcionam apenas com `PYTHONPATH` específico e quebram em uso normal. |
| F-09 | **Alta** | O workflow possui falso-verde por pipeline sem `pipefail`. | Reproduzido por análise de shell | `run: ./prove.sh | tee /tmp/prove.txt` retorna o status do `tee`, não necessariamente o do `prove.sh`. |
| F-10 | **Alta** | A proveniência cobre poucos inputs e não autentica checkout, builder, dependências ou artefato. | Declarado e quantificado | O auditor local passa, mas `research/symbiont_v2`, `research/stage4` e `research/social` não estão integralmente protegidos pelo manifesto. |
| F-11 | **Média** | Não há política de versão/release compatível com múltiplos schemas e APIs. | Reproduzido | Zero tags, versão mestre ainda `0.1 · pre-Phase-0`, `SCHEMA_VERSION=3` e vários contratos `v1`, sem matriz de compatibilidade ou changelog executável. |
| F-12 | **Média** | O protocolo social é um contrato pronto, não uma integração experimental executável. | Declarado no código | O runner só valida presença de campos e sempre retorna `human_results: null`; não há randomização, coleta, análise, exclusões ou validação de dados participante a participante. |
| F-13 | **Média** | A suíte cria confiança excessiva porque não testa as fronteiras novas. | Reproduzido por inventário | Não há testes para autenticação de envelopes, atestação de recovery, imutabilidade do ledger, contabilidade integrada de orçamento, importação como pacote ou validação JSON Schema no CI. |

## 3. Findings detalhados

### F-01 — Executor externo aceita envelope semanticamente não vinculado

`research/stage4/executor.py:20–45` verifica apenas `authority_id`, TTL, época de revogação, nonce monotônico e valores mínimos de `max_steps`/`max_cost`. Os campos `proposal_digest`, `action_sequence_digest` e `host_context_digest` são recebidos, mas nunca comparados a uma fonte confiável, assinados ou sequer validados como digests. O executor portanto aceita este caso:

```text
ExecutionEnvelope(proposal_digest="bogus", action_sequence_digest="bogus",
                  host_context_digest="bogus", max_steps=1, max_cost=1, ...)
-> COMMITTED
```

`max_cost` também não limita a execução. Um envelope com `max_steps=4` e `max_cost=1` produz quatro efeitos e quatro recibos com custo unitário. O valor é decorativo. Além disso, `nonce_floor` é avançado antes da validação de todos os pré-requisitos; um envelope inválido pode consumir nonce e produzir uma falha operacional difícil de repetir.

O controle de replay também é apenas global e numérico. A mesma `operation_id` pode ser executada duas vezes com nonces novos, e uma tentativa menor pode ser aceita depois de uma tentativa maior. Isso não implementa idempotência por operação, monotonicidade de tentativa ou prevenção de duplicação de efeitos.

**Mudanças exigidas:**

1. Separar `Proposal`, `Authorization`, `ExecutionEnvelope` e `ExecutionReceipt` como tipos diferentes. Um `TransferProposal` não pode ser convertido em envelope por cópia de campos.
2. Validar formato, versão, validade temporal, sequência de passos, custo acumulado e todos os digests contra um registro de autorização assinado por uma autoridade externa real.
3. Manter estado por `(authority_id, operation_id)` com `attempt_no` monotônico, nonce único e política explícita de idempotência. Rejeitar operação repetida, rollback de tentativa e nonce reutilizado.
4. Fazer a transição de nonce somente depois de a autorização completa passar pelo gate e registrar a decisão de forma atômica.
5. Remover o executor in-memory da alegação de integração; mantê-lo apenas como fake explicitamente nomeado em testes.

**Testes necessários:** `test_digest_mismatch_rejected`, `test_cost_ceiling_stops_before_excess_effect`, `test_operation_id_is_idempotent`, `test_attempt_cannot_rollback`, `test_invalid_envelope_does_not_consume_nonce` e um teste de assinatura/autorização usando uma implementação independente.

### F-02 — Recovery e append-only ledger não fornecem integridade

`research/stage4/executor.py:51–59` resolve uma recuperação se `source_id` for diferente do executor e se `conclusion` pertencer a um conjunto de strings. Não verifica `operation_id`, `attempt_no`, `ledger_digest`, `committed_steps`, assinatura, frescor ou relação com um `UNKNOWN_OUTCOME` existente. O probe abaixo é aceito:

```text
RecoveryAttestation(operation_id="op", attempt_no=1,
  conclusion="RESOLVED_COMMITTED", source_id="not-executor",
  ledger_digest="forged", committed_steps=999)
-> RECOVERY_RESOLVED
```

`research/stage4/ledger.py:10–15` usa uma lista mutável e `snapshot()` faz apenas cópia rasa. Alterar `snapshot["events"][0]["event"]` altera o ledger original. Como o digest é recalculado sobre a lista mutada, o histórico não é append-only na prática.

**Mudanças exigidas:**

1. Tornar eventos e snapshots imutáveis ou profundamente copiados; impedir alteração após append.
2. Persistir a cadeia em armazenamento transacional, com sequência monotônica, checksum do evento anterior, checksum do snapshot e teste de recuperação após interrupção.
3. Definir um formato assinado para `RecoveryAttestation`; verificar identidade da fonte, operação, tentativa, ledger anterior, número de passos e conclusão autorizada.
4. Não resolver recovery sem um estado pendente correspondente. `RECOVERY_CONFLICT` deve permanecer bloqueado até revisão independente.
5. Demonstrar atomicidade e durabilidade em uma implementação de backend, não apenas em dataclasses.

**Testes necessários:** mutação profunda de snapshot, alteração de evento anterior, fork da cadeia, atestação com digest errado, atestação de operação inexistente, atestação com passos incompatíveis, replay de recovery e campanha de interrupção entre gravações.

### F-03 — O ledger Stage 5 não governa a decisão

`research/symbiont_v2/core.py:443–494` cria `BudgetLedger`, mas chama `self.propose_transfer()` em seguida e retorna o ledger original. `propose_transfer()` executa `discover(host)`, que chama `host.execute()` e `host.reset()` em `core.py:263–291`. O resultado, portanto, pode declarar:

```text
probe_execute_calls = 0
budget_ledger.probe_count = 0
```

mesmo quando probes mutantes foram realizados. O teste de Stage 5 cobre apenas `BudgetLedger.with_probe()` isoladamente; não cobre a decisão integrada. O próprio design documentado em `docs/54-CORRECAO-CONTRATO-OBSERVABILIDADE.md:45` afirma um ledger append-only, mas não existe um fluxo de eventos que o atualize.

`BudgetLimits.step_max`, `cost_max`, `retry_max` e `recovery_max` não governam o caminho de síntese. `with_probe()` e `with_reset()` só incrementam uma dimensão; não existe transição para custo observado, passo consumido, retry ou recovery. `BudgetLimits.validate()` aceita `None` para `probe_max`, e o erro só aparece depois como `TypeError` em uma comparação.

**Mudanças exigidas:**

1. Passar um objeto de orçamento explícito para cada operação de probe/reset/step/retry/recovery; cada operação deve retornar um novo ledger e um evento.
2. Fazer `discover()` receber a política de orçamento e parar antes de executar a operação que excederia o limite.
3. Preencher `probe_count`, `reset_count`, `step_count`, `cost_actual` e `budget_state` com fatos observados, não com valores default.
4. Rejeitar `None` em limites obrigatórios e declarar separadamente “custo desconhecido”, “custo limitado” e “custo exato”.
5. Não emitir `SUPPORTED` se o orçamento não for fechado ou se a contabilidade não estiver completa.

**Testes necessários:** contador de decisão igual ao contador do host; zero execução após orçamento esgotado; custo 0, custo desconhecido e custo acima do teto; limite de passos; retry/recovery; reset que não zera o ledger; e teste de não mutação do ledger anterior.

### F-04 — O gate estrito aceita contratos cosméticos

`SkillObservabilityContract.valid_for_strict()` e `HostObservabilityContract.valid_for_strict()` verificam igualdade de alguns literais e apenas `bool(...)` para os digests (`stage5.py:139–166`). Não há validação de comprimento/formato SHA-256, assinatura, cadeia de confiança, schema JSON, vínculo entre digest e conteúdo ou atestação verificável. `AbstractSkill.observability_contract` é tipado como `object` (`core.py:127`). O runtime aceita qualquer objeto que exponha um método `valid_for_strict()` que retorne verdadeiro.

Isso é coerente com a limitação declarada em `docs/54:53–57`, mas é incompatível com o nome `SUPPORTED`: o código não distingue “contrato fornecido pelo chamador” de “contrato atestado por uma autoridade independente”. A string `external-host-closure` do teste é suficiente para atravessar o gate.

**Mudanças exigidas:** usar schemas versionados, tipo fechado, canonicalização, digest calculado pelo runtime, assinatura/atestação verificável e uma política de confiança explícita. O estado deveria ser `CONTRACT_PRESENT_UNVERIFIED` até a verificação independente, não `SUPPORTED`.

**Testes necessários:** digest vazio, digest não hexadecimal, digest que não corresponde ao JSON, objeto malicioso com `valid_for_strict()`, schema futuro incompatível, assinatura errada e contrato de Skill de outro host.

### F-05 — O holdout não oferece isolamento independente suficiente

`research/holdout_adversarial.py` importa `SymbiontRuntime` e a verdade das fixtures no mesmo processo (`:14–16`). `IndependentOracle` é uma classe Python no mesmo módulo de campanha (`:20–26`), e `PublicHoldoutHost` mantém `_private_state` e `_fixture` no mesmo objeto que expõe a superfície pública (`holdout_hosts.py:23–37`). O runtime não recebe esses campos por API, mas a fronteira não é de processo, não é de pacote separado e não é protegida contra introspecção ou efeitos laterais.

O teste `test_oracle_does_not_call_runtime` demonstra uma intenção importante, mas não constitui independência de implementação. Um bug comum de importação, monkey-patch, introspecção ou alteração acidental da fixture pode afetar os dois lados.

**Mudanças exigidas:** mover oráculo e verdade para um processo separado, com protocolo IPC mínimo e somente dados serializados; usar um pacote ou binário independente; carregar fixtures privadas a partir de artefato somente leitura; e comparar a saída por um validador externo. O runtime deve ser executado em processo sem acesso ao caminho do oráculo.

**Testes necessários:** sandbox que bloqueie import de `core` no oráculo, teste de tentativa de acesso a `_private_state`, execução em subprocessos separados, alteração do runtime não alterando o oráculo, e verificação de que o oráculo só recebe o identificador/resultado permitido após a decisão.

### F-06 — O runner do holdout não exercita parcial nem orçamento

Em `run_case()` (`holdout_adversarial.py:62–102`), o executor instrumentado só é chamado quando `oracle_status == "SUPPORTED"` (`:72–73`). Isso ocorre no controle positivo. Para `m7a-partial-01` e `m8a-budget-01`, o resultado é rotulado como `FAIL_UNSAFE` pelo oráculo, mas nenhum `partial_at`, `ack=False`, teto de custo ou recuperação é executado. A campanha preserva nomes e razões, não comportamento de executor.

Consequentemente, a afirmação em `docs/53:18–19` de que esses casos medem falha parcial/ACK e custo excedido é exagerada. Os testes de executor existentes validam alguns estados diretamente, mas não a integração campanha → executor → ledger → resultado.

**Mudanças exigidas:** separar campanhas de proposta, autorização, execução e recuperação. Para cada fixture, executar o caminho aplicável em ambiente controlado, registrar efeitos reais do fake, e fazer o oráculo independente validar o resultado depois. Casos negativos devem provar abstention ou rejeição no ponto correto, não apenas receber uma etiqueta final.

**Testes necessários:** campanha com ACK ausente, falha no passo 1 e no passo intermediário, custo que excede no passo N, recuperação válida e inválida, sem retry automático, e comparação do estado do ledger com `ResultRecord`.

### F-07 — Schemas e validadores são permissivos demais

`research/holdout_benchmark_schema.json:18–34` exige presença e tipos básicos, mas deixa `proposal_status`, `reason`, `oracle_verdict` e `safety_status` como strings livres. `hard_failures` também é um array sem itens tipados. Não há invariantes cruzadas no JSON Schema: uma fixture negativa pode ter `proposal_status: PROPOSED`, `external_effect_count: 0` e ainda ser formalmente válida.

`research/stage4/contracts.py:70–84` só verifica campos presentes, contadores não negativos, execução da proposta igual a zero e ausência de efeitos em negativos. `validate_result()` aceitou um registro com enums arbitrários e `raw_trace_digest: "bad"`. O digest do trace não pode ser recomputado porque o trace não está no registro nem há digest do fixture, commit, seed, comando e ambiente.

A evidência congelada coincide com a saída atual do gerador, o que é bom para regressão, mas não há uma etapa CI que valide o JSON de evidência com um validador de JSON Schema. `tools/bench_evidence_validate.py` existe, porém seu teste não aparece no `prove.sh` nem no workflow.

**Mudanças exigidas:**

1. Definir enums fechados, `minimum/maximum`, relações entre `expected_negative`, `oracle_verdict`, `proposal_status`, `safety_status` e `external_effect_count`.
2. Incluir trace canônico ou manifesto de trace, fixture digest, código commit, seed, comando, versão Python/CC, plataforma e digest do runner.
3. Validar schema e regras cruzadas no CI com uma ferramenta pinada.
4. Rejeitar registros que não possam recomputar todos os digests declarados.

### F-08 — Imports e empacotamento são inconsistentes

Os módulos `research.audit_minds14_text`, `research.fetch_minds14_batch` e `research.semantic_ir_fuzz` falham em importação normal porque usam imports de sibling como `from fetch_minds14_sample import ...` e `from semantic_ir import ...`. A suíte funciona por causa de `PYTHONPATH=research` e do modo de execução de arquivos no Makefile. Isso mascara falhas de consumidor externo, IDE, test runner e instalação.

Não há `pyproject.toml`, `requirements.txt`, lockfile ou política de versão suportada. O repositório não tem um comando único para instalar o pacote em ambiente limpo. `compileall` passa, mas isso só prova sintaxe.

**Mudanças exigidas:** escolher uma estratégia única, preferencialmente pacote instalável com `pyproject.toml`, imports relativos/absolutos consistentes, extras para dados reais e dependências pinadas. Adicionar smoke test em ambiente limpo com `pip install .` e executar todos os módulos por `python -m ...`, não por dependência de diretório no `PYTHONPATH`.

**Testes necessários:** matriz de importação de todos os módulos, instalação em virtualenv vazio, `python -m research.semantic_ir_fuzz`, execução a partir de outro diretório e teste de dependências opcionais sem contaminar o caminho mínimo.

### F-09 — CI pode marcar `prove.sh` como sucesso após falha

`.github/workflows/prove.yml:40–41` usa:

```yaml
run: ./prove.sh | tee /tmp/prove.txt
```

Sem `set -o pipefail`, o status do passo é normalmente o status do `tee`. Portanto, uma falha de `prove.sh` pode ser escondida. O probe equivalente reproduziu `false | tee` com status zero sem `pipefail` e status um com `pipefail`.

O workflow ainda usa `ubuntu-latest` e `macos-latest` sem pinagem de imagem/toolchain, Python e compilador sem versão, e não instala nem verifica um ambiente de dependências. A decisão de depender apenas da stdlib é válida como objetivo, mas não substitui a declaração de versões e uma imagem reprodutível.

**Mudanças exigidas:** usar `set -euo pipefail` no step ou `bash -o pipefail -c './prove.sh | tee ...'`; falhar se a saída final esperada não existir; publicar código de saída e logs estruturados. Piná-la matriz de toolchains ou adicionar uma imagem/container de build reproduzível. Manter uma job separada para pesquisa e uma job separada para firmware, ambas com artefatos e relatórios JUnit.

**Testes necessários:** teste de regressão do workflow com `prove.sh` que falha, lint de shell, execução local do mesmo comando CI e verificação de que um resultado sem ledger não pode publicar sucesso.

### F-10 — Proveniência local não é provenance de release

`research/software_provenance_manifest.json:1–8` declara explicitamente `local_unattested` e `unattested_worktree_inputs`. O auditor aceita digests locais de 74 inputs, mas o inventário quantificou que apenas 1 arquivo de `research/symbiont_v2` está protegido e nenhum arquivo de `research/stage4` ou `research/social` está protegido. Assim, justamente os componentes novos da integração não estão cobertos pelo manifesto.

O próprio `docs/29-PROVENIENCIA-LOCAL-BUILD.md` reconhece que faltam checkout autenticado, builder assinado/pinado, SBOM completo, build reproduzível independente, verificação artefato→fonte e auditoria independente. O finding não é que o projeto esconde isso; é que o CI ainda dá uma aparência de validação completa quando valida somente um subconjunto declarado e não autenticado.

**Mudanças exigidas:** gerar o manifesto em CI a partir de commit fixo; cobrir todos os arquivos que entram nos testes e release; registrar dependências, toolchain, imagem, comando e artefatos; produzir SBOM; assinar manifesto e artefatos; verificar em job independente; e falhar se um input usado não estiver declarado.

**Testes necessários:** adicionar/remover input não listado, alterar arquivo de Stage 4, alterar schema, alterar dependência, mismatch entre commit do checkout e manifest, SBOM incompleto e verificação independente em segundo builder.

### F-11 — Versionamento e compatibilidade não são operacionais

Não há tags ou releases. `docs/00-HERUS-MASTER.md:3` ainda identifica o projeto como `Version 0.1 · pre-Phase-0`, apesar de a documentação recente anunciar etapas finais, contratos Stage 4/5 e 142 testes. No código, `core.py` usa `SCHEMA_VERSION = 3`, Stage 5 usa strings `herus-*-v1`, o holdout usa `herus-stage4-integration-v1` e o contrato social usa `herus-social-utility-v1`. Não existe matriz que diga quais versões podem ser lidas, migradas ou rejeitadas.

`AbstractSkill` preserva chamadas posicionais adicionando o campo ao fim, mas isso não é uma política de compatibilidade. Também não há serialização versionada de `TransferDecision`, migração de snapshots, política de depreciação para `transfer()`/`propose_transfer()` ou teste de compatibilidade entre versões.

**Mudanças exigidas:** adotar SemVer ou uma política equivalente, criar tags assinadas, changelog e matriz de compatibilidade; separar versão de código, schema, fixture e protocolo; definir migrações e rejeições explícitas; e publicar um release manifest reproduzível.

### F-12 — O protocolo social não é um runner experimental completo

`research/social/runner.py:10–23` verifica somente status, conjunto de condições e três nomes de métricas. `main()` sempre retorna `human_results: None`. Não valida thresholds, randomização/ordem de condições, tamanho amostral, critérios de exclusão, identificação de protocolo congelado, consentimento real, dados participante a participante, desvios, análise estatística ou publicação de negativos.

Isso não é um defeito se for rotulado apenas como contrato pronto, e `docs/55:28–40` faz essa ressalva. Torna-se um problema de integração quando o status `READY_FOR_EXTERNAL_REVIEW` é confundido com “experimento executável”. O runner deve bloquear qualquer tentativa de classificar benefício a partir de um relatório incompleto.

**Mudanças exigidas:** separar `protocol_validate`, `study_runner` e `report_validator`; versionar randomização e seed; definir schema de observações, consentimento, retirada, intervenção e exclusão; validar análise cega/pré-registrada; e tornar impossível produzir `useful_symbiosis` sem dados e negativos completos.

### F-13 — Cobertura existente não cobre as novas fronteiras

Os 142 testes são um bom baseline host-only, mas a contagem não mede qualidade de integração. Há um teste pulado, não há cobertura/threshold, não há lint/type check no CI, e os testes novos não exercitam os principais defeitos demonstrados neste relatório. `tools/test_bench_evidence_validate.py`, `tools/test_llm_budget_check.py` e `tools/test_proof_fire_mutations.py` não são chamados pelo `prove.sh` nem pelo workflow. A lista manual de 23 testes no `research/Makefile` também pode ficar stale sem falhar quando novos testes forem adicionados.

**Mudanças exigidas:** usar descoberta automática ou uma lista gerada/verificada; registrar cobertura mínima por pacote; adicionar lint, typing e testes de contrato; executar validadores de artefato no CI; e marcar explicitamente testes `host-only`, `fake-executor`, `subprocess-isolated` e `target-pending`.

## 4. Plano de mudanças priorizado

### P0 — bloquear alegações e fechar a integração

1. Corrigir o workflow com `pipefail`.
2. Desativar qualquer interpretação de `SUPPORTED` como segurança enquanto não houver autorização assinada, verificação de envelope e executor externo real.
3. Reescrever o executor como máquina de estados testável, com validação de digests, custos, operação/tentativa, nonce, ACK, recibos e recovery.
4. Tornar o ledger imutável por API, profundamente copiado e persistível; adicionar teste de interrupção e recuperação.
5. Integrar o ledger ao caminho real de `discover()`/`propose_transfer_checked()` e fazer todos os limites governarem execução.
6. Isolar oráculo e verdade privada em subprocesso ou binário independente.

### P1 — tornar o repositório reproduzível e utilizável

1. Empacotar `research` com `pyproject.toml`, imports consistentes, dependências/optional extras pinadas e smoke test em ambiente limpo.
2. Fechar schemas e validadores; validar evidência, traces e manifests no CI.
3. Completar provenance dos módulos Stage 4/5/social, toolchain, dependências, SBOM e artefatos.
4. Substituir o Makefile manual por descoberta verificada e publicar logs JUnit/JSON.
5. Criar matriz de compatibilidade para core, Stage 4, Stage 5, contratos e fixtures.

### P2 — reduzir cosmética e manter alegações honestas

1. Renomear `READY_FOR_EXTERNAL_REVIEW` para um estado que não pareça resultado experimental, ou manter o nome somente com bloqueio explícito de classificação.
2. Atualizar `docs/00`, README e changelog com estado de release real, número de commit e limites de cada evidência.
3. Documentar quais comandos são host-only, simulation-only, fake-executor e target-pending.
4. Adicionar referências cruzadas para o módulo real `research/symbiont_v2/stage5.py`; não usar o caminho inexistente `research/stage5.py` em instruções operacionais.

## 5. Test matrix mínima para aceitar as mudanças

| Área | Teste de aceitação |
|---|---|
| Envelope | Digests errados, autoridade errada, TTL/época inválidos, custo insuficiente, passos excessivos e nonce inválido são rejeitados sem efeitos e sem consumir nonce indevidamente. |
| Idempotência | Mesma operação não produz dois commits; tentativa menor é rejeitada; retry automático é impossível. |
| Recovery | Atestação forjada, stale, de operação inexistente ou com ledger/steps incompatíveis não resolve `UNKNOWN_OUTCOME`. |
| Ledger | Snapshot é imutável; qualquer alteração de evento quebra verificação; restart conserva cadeia e sequência. |
| Stage 5 | `probe_count`, `reset_count`, `step_count` e custo da decisão coincidem com o host; o próximo probe não é executado quando o teto foi atingido. |
| Observabilidade | Só contrato canônico, schema-validado, digest-bound e atestado pode produzir estado de suporte; strings arbitrárias falham. |
| Holdout | Oráculo corre em subprocesso sem acesso ao runtime; cada família parcial, custo, alias, stale e época exerce o caminho real correspondente. |
| Resultado | Enums, regras cruzadas, trace digest e fixture digest são verificados por schema e por validador independente. |
| Packaging | `pip install .` em virtualenv limpo, importação de todos os módulos e execução por `python -m` passam a partir de qualquer diretório. |
| CI | Falha deliberada de `prove.sh` torna o job vermelho; toolchain e imagem são identificáveis; artefatos e logs têm digest. |
| Provenance | Todo input usado está no manifesto; manifesto, SBOM e artefatos são verificados em job independente. |
| Social | Sem dados humanos completos, o runner não produz classificação de benefício; com dados, reporta exclusões, negativos, seed e análise pré-registrada. |

## Veredito final

**Reprovado para release profissional de integração.** A arquitetura documental é mais honesta que a implementação média, e o holdout histórico conseguiu expor um contraexemplo real. Isso é mérito. Porém, a correção atual ainda cria gates e nomes de contrato sem enforcement equivalente. O próximo trabalho não deve adicionar mais fixtures ou texto de conclusão. Deve fechar a cadeia executável: autorização autenticada → orçamento contabilizado → executor idempotente → recibo verificável → recovery atestado → evidência validada → provenance reproduzível. Até isso ocorrer, o resultado máximo defensável é **mecanismo host-only parcialmente testado, não provado como execução segura e não demonstrado como simbiose útil**.

## Referências internas

[1]: ../00-HERUS-MASTER.md "HERUS Master Design Document"
[2]: ../52-DEFINICAO-SIMBIOSE-UTIL.md "Definição congelada de simbiose útil"
[3]: ../53-BENCHMARK-HOLDOUT-ADVERSARIAL.md "Benchmark holdout adversarial"
[4]: ../54-CORRECAO-CONTRATO-OBSERVABILIDADE.md "Contrato de observabilidade"
[5]: ../55-ETAPAS-FINAIS-EXECUTOR-HUMANO.md "Etapas finais e executor humano"
[6]: ../../research/symbiont_v2/stage5.py "Ledger e contratos do Stage 5"
[7]: ../../research/stage4/executor.py "Adaptador de executor externo"
[8]: ../../research/stage4/ledger.py "Append-only ledger"
[9]: ../../research/holdout_adversarial.py "Campanha holdout adversarial"
[10]: ../../research/holdout_benchmark_schema.json "Schema do benchmark holdout"
[11]: ../../.github/workflows/prove.yml "Workflow de CI"
[12]: ../../research/software_provenance_manifest.json "Manifesto de proveniência de software"
[13]: ../../research/social/runner.py "Runner do protocolo social"
[14]: ../../prove.sh "Ledger global de prova"

*Auditoria produzida sem editar código; somente este relatório foi criado no caminho solicitado.*
