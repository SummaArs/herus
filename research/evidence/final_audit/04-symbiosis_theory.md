# Auditoria adversarial da hipótese ASA e da simbiose

**Escopo auditado.** Foram revisados `docs/00-HERUS-MASTER.md`, `docs/51-API-SIMBIONTE-V2.md`, `docs/52-DEFINICAO-SIMBIOSE-UTIL.md`, `docs/53-BENCHMARK-HOLDOUT-ADVERSARIAL.md`, `docs/54-CORRECAO-CONTRATO-OBSERVABILIDADE.md`, `docs/55-ETAPAS-FINAIS-EXECUTOR-HUMANO.md`, `research/symbiont_v2`, `research/symbiont_v2/stage5.py`, `research/stage4`, `research/holdout*`, `research/extended_holdout.py`, `research/social`, os contratos machine-readable e os artefatos de evidência. O commit observado foi `fbf7471` (`research: complete executable final stages`). Nenhum código foi editado.

## Veredicto

**Reprovado para a alegação forte de ASA/simbiose; aprovado somente como protótipo host-only de uma hipótese estreita e ainda não provada em avaliação independente. A classificação correta permanece `not_proven`.**

Há um mecanismo plausível e parcialmente testado: uma identidade persistente pode conservar uma Skill discreta, observar um novo host, inferir um contrato de efeito público e propor um plano sem enviá-lo ao executor. Isso é uma propriedade de software delimitada. Não é ainda uma demonstração de simbiose útil, nem de adaptação geral, nem de segurança de execução.

O ponto decisivo é que o repositório mistura três níveis que precisam permanecer separados:

1. **Mecanismo:** reancoragem de uma competência finita entre hosts com nomes de ação diferentes, sob observabilidade e orçamento explícitos.
2. **Garantia:** capacidade de saber quando o mecanismo não tem informação suficiente, abster-se e não mutar o mundo durante a proposta.
3. **Impacto:** uma pessoa real conclui uma tarefa definida com menos erro, tempo, esforço ou dependência de interface do que com uma baseline congelada.

O primeiro nível tem uma regressão pública útil. O segundo ainda contém falhas metodológicas e de implementação. O terceiro não tem nenhum dado humano: `research/social/runner.py` retorna `human_results: None`, embora o status seja `READY_FOR_EXTERNAL_REVIEW`. Portanto, a avaliação externa não deve ser mudada por mais uma demo de transferência entre brinquedos. A demonstração que mudaria a avaliação é uma campanha nova, independente e cega, seguida de uma tarefa humana estreita com CB/NS/H, mas somente depois de fechar o gate mecânico.

## Mínimo falsificável

A hipótese ASA mínima deve ser reescrita assim:

> **Dado um contrato de Skill finita congelado, uma identidade persistente e dois hosts com vocabulário operacional diferente, o runtime consegue construir a mesma proposta sem ler a memória privada do host, sem depender dos nomes das ações e sem executar ou mutar o alvo durante a proposta; quando a superfície pública não prova fechamento, validade temporal, pré-condições, custo ou autoridade, o runtime abstém-se ou retorna `SAFE_BUT_UNPROVEN`, nunca `SUPPORTED`.**

Essa hipótese é falsificável. Ela exige, em um split secreto ou gerado após o congelamento:

- transferência causal em hosts com ações renomeadas e estados permutados;
- `ABSTAIN`/`UNSUPPORTED_BY_CONTRACT` em alias, conflito, stale, pré-condição falsa, ordem não comutativa, recurso insuficiente, custo desconhecido e efeito oculto;
- zero `SUPPORTED` em pares de hosts **observacionalmente equivalentes** durante todas as probes, mas diferentes no primeiro commit autorizado;
- zero mutação privada durante qualquer probe de proposta;
- zero execução autorizada, efeito externo, replay, retry automático ou orçamento excedido silencioso;
- rastreio reproduzível de proposta, oracle, executor, ledger, seed e digests, com processos separados.

A taxa de transferência não é suficiente. A métrica principal deve ser uma matriz de decisão por caso: `SUPPORTED` somente em controles com atestação autenticada e fechamento causal; `ABSTAIN`/`UNSUPPORTED` em todos os casos não identificáveis; nenhum falso consenso conhecido ou descoberto. Um resultado positivo sem negativos, sem permutações e sem um oracle independente é apenas uma demonstração de que o programa encontrou o caminho feliz.

O mínimo falsificável de **simbiose útil** é mais forte:

> **Em uma tarefa humana estreita, reversível e não médica, H (HERUS) reduz uma dimensão pré-especificada de custo ou dependência de interface contra NS (sem simbionte), não piora erro contra CB (baseline convencional) além da margem congelada, registra falhas e mantém consentimento, acessibilidade, parada e privacidade válidos.**

Isso só pode ser classificado como `useful_symbiosis` para aquela tarefa, população, interfaces e protocolo. Não implica inteligência geral, consciência, segurança física, eficácia clínica ou impacto social amplo.

## Achados críticos

### F-01 — O “oráculo independente” não calcula a verdade da proposta

**Severidade: crítica.** Em `research/holdout_adversarial.py:20–26`, `IndependentOracle.evaluate()` decide a partir de `fixture.expected_negative` e devolve `fixture.oracle_reason`. Não verifica ações, ordem, pré-condições, efeitos, custo ou autorização da proposta. Esses rótulos ficam na mesma estrutura que contém a verdade privada. Mutar somente `expected_negative` muda `FAIL_UNSAFE` para `SUPPORTED`; mutar somente `oracle_reason` muda a explicação. O teste que passa `object()` apenas prova que o método não chama o runtime, não que seja semanticamente independente.

**Consequência.** A campanha não mede um oracle independente; mede consistência com labels fornecidos pelo próprio harness. Os nove `hard_failures` do artefato v1 preservam uma conclusão prudente, mas não validam a independência alegada.

**Mudança exigida.** Remover labels decisórios da entrada do oracle. Entregar a ele somente proposta serializada e tabela privada de transição mantida fora do módulo do runtime. O veredicto deve ser derivado de ações, estado, pré-condições, efeitos proibidos, custo e autoridade. `expected_negative` e `oracle_reason`, se mantidos, devem ser expectativas pós-hoc para comparação, nunca entradas de decisão.

### F-02 — O holdout é público, previsível e não é holdout cego

**Severidade: crítica.** `holdout_hosts.py:109–116` publica os cinco IDs e todas as propriedades causais. `holdout_adversarial.py` importa diretamente `holdout_fixtures()`. Os prefixos (`m6a`, `m7a`, `m8a`, `m9a`, `c0a`) revelam a classe do caso e `host_id` é derivado de `fixture_id[:8]`. O mesmo checkout contém runtime, fixtures, labels, oracle e resultados. As três repetições são a mesma fixture determinística, com o mesmo estado, ordem, orçamento e sem variação.

**Consequência.** O runtime pode ser ajustado para os IDs, prefixos ou padrão `q7 -> x += 1`; não há evidência de generalização causal. O resultado atual é uma suíte pública de regressão, não uma avaliação independente.

**Mudança exigida.** Separar runtime, host proxy, oracle e verdade privada em processos distintos. Usar tokens opacos e IDs permutáveis. Registrar digest do runtime, oracle, conjunto privado, schema, seed e comando. O runtime não pode importar nem abrir o arquivo da verdade. O release público deve conter apenas smoke tests; a conclusão deve depender de holdout secreto ou de geração independente após o freeze.

### F-03 — A proposta não executa o plano, mas a descoberta executa probes mutantes

**Severidade: crítica.** A documentação afirma corretamente que construir `TransferProposal` não envia o plano ao executor. Porém `core.py:374–379` chama `discover(host)`, e `discover()` chama `host.execute(action)` para cada probe. Em `PublicHoldoutHost.execute()`, a probe altera `_private_state`; em `m6a-opaque-01`, o `damage += 1` oculto pode ocorrer durante a descoberta. O artefato registra `proposal_execute_calls=0` e `external_effect_count=0`, mas esses contadores pertencem ao executor instrumentado, não ao estado privado do host.

**Consequência.** “Zero execução da proposta” é verdadeiro em um sentido estreito e enganoso: houve uma chamada mutante durante a construção da proposta. O caminho `COMPATIBILITY` continua vulnerável. A correção Stage 5 bloqueia o caso sem contrato no caminho `STRICT`, mas não transforma `execute()` em observação não mutante.

**Mudança exigida.** Separar `probe()` de `commit()`. A probe deve ter garantia externa de não mutação; sem essa garantia, deve ser tratada como execução autorizada e exigir envelope, orçamento, recibo e recovery. Registrar digest independente de estado privado antes/depois, `probe_mutation_count`, `authorized_calls`, `committed_calls` e `world_mutation_count`.

### F-04 — `STRICT` aceita atestação textual forjada como `SUPPORTED`

**Severidade: crítica.** `SkillObservabilityContract.valid_for_strict()` e `HostObservabilityContract.valid_for_strict()` em `stage5.py:139–166` exigem somente enums válidos e strings não vazias. `propose_transfer_checked()` então retorna `SUPPORTED` sem verificar assinatura, emissor, escopo, época, validade, revogação, vínculo ao `skill_digest`, vínculo ao `host_digest` ou fechamento independente. Um digest não vazio é integridade de bytes, não atestação de verdade.

**Consequência.** Um contrato falso pode produzir `PROPOSED/SUPPORTED` para um host opaco. O próprio documento 54 reconhece essa limitação, mas o estado positivo ainda é emitido. A correção é, portanto, cosmética enquanto consumidores interpretarem `SUPPORTED` como segurança.

**Mudança exigida.** Separar `CONTRACT_PRESENT`, `ATTESTED` e `SUPPORTED`. O último estado só pode ser emitido por verificador externo que valide assinatura, emissor confiável, escopo, versão, `skill_digest`, `host_digest`, proposta, orçamento, época e revogação. Atestação ausente, stale, mismatch ou forjada deve retornar `UNSUPPORTED_BY_CONTRACT` ou `SAFE_BUT_UNPROVEN`, nunca `SUPPORTED`.

### F-05 — `m7` e `m8` são labels, não execuções das condições anunciadas

**Severidade: alta.** Em `run_case()`, o executor só é chamado para `oracle_status == SUPPORTED`; isso acontece no controle, não nas fixtures de falha parcial e overrun. Não há `partial=True`, `ack=False`, envelope de custo ou comparação de custo real com limite. Para `m8`, `budget_exhausted` é apenas `fixture.cost > 1`. Para `m7`, `PARTIAL_FAILURE` é uma string da fixture.

**Consequência.** O resultado colapsa `UNSUPPORTED`, `FAIL_UNSAFE`, `UNKNOWN_OUTCOME` e `COST_OVERRUN`. A suíte passa porque testa os rótulos gerados, não os eventos.

**Mudança exigida.** Executar m7 no adaptador externo com `partial_at` e ausência de ACK, gerando recibos reais. Executar m8 com ledger real e custo acima do limite. Derivar `execution_status` da trilha do executor, nunca de `oracle_reason`. O schema deve separar `oracle_verdict`, `proposal_status`, `execution_status`, `safety_claim`, eventos observados, contadores derivados e resultado esperado.

### F-06 — O protocolo humano está pronto no papel, não no experimento

**Severidade: alta.** `utility_protocol_v1.json` declara `protocol_ready_no_human_data`; `runner.py` valida apenas três condições, três nomes de métricas e o status. `main()` sempre retorna `human_results: None`. Não há máquina de estados de ensaio, randomização/contrabalanço, schema de observações, dados por participante, consentimento operacional, critérios de exclusão, validação de thresholds ou análise estatística. O protocolo detalhado em `remaining_plan/04-human_utility_protocol.md` é bom como plano, mas não é evidência.

**Consequência.** O status `READY_FOR_EXTERNAL_REVIEW` pode ser lido apressadamente como resultado ou experimento executável. Nenhuma alegação de benefício humano é sustentada. O classificador não deve aceitar um relatório com campos preenchidos manualmente como observação.

**Mudança exigida.** Separar `protocol_validate`, `study_runner` e `report_validator`. Criar schema de eventos por participante, consentimento separado, retirada, intervenção, acessibilidade, desvios e dados ausentes. Congelar seed, ordem, versão e digests antes da coleta. Impedir qualquer classificação de benefício sem raw results, negativos, denominadores e trilha de intervenção.

### F-07 — O classificador de utilidade verifica presença de campos, não verdade dos dados

**Severidade: alta.** `research/symbiosis_utility.py:19–84` checa chaves e alguns limites, mas não valida tipos, intervalos, unidades, denominadores, vínculo a resultados brutos, digest real ou autenticidade de `code_commit`, `raw_results` e `negative_results`. `classify()` não comprova o benefício; apenas classifica um relatório que o validador considera completo. Um relatório sintético com placeholders em todos os campos de reprodutibilidade e controles declarados passa como elegível a `useful_symbiosis` quando as chaves e valores booleanos esperados são fornecidos; a função não abre nem rederiva os artefatos.

**Consequência.** O contrato é fail-closed quanto a campos ausentes, mas não quanto a evidência fabricada, tipo inválido ou métrica semânticamente absurda. `useful_symbiosis` significa “o objeto passou no validador”, não “a campanha humana aconteceu”.

**Mudança exigida.** Tornar o classificador um verificador de pacote: schema estrito, tipos numéricos e intervalos, denominadores, cálculo rederivado a partir de raw results, digest e commit verificáveis, comparação estatística pré-especificada e autenticação do pacote. O estado positivo deve exigir dados presentes e verificáveis; caso contrário, `not_proven`.

### F-08 — O holdout estendido é uma reserva de labels, não uma campanha

**Severidade: alta.** `research/extended_holdout.py` cria nove registros sintéticos com `proposal_status: UNSUPPORTED_BY_CONTRACT`, sem executar runtime, host proxy, oracle ou executor. `extended_holdout_v1.json` contém `reason`, `expected_negative` e `budget_exhausted` ausentes. Isso demonstra uma decisão conservadora codificada, não resistência a deriva temporal, ordem, pré-condições ou recursos.

**Mudança exigida.** Implementar as famílias em processos separados, com fixtures privadas, ações e estados permutados, transcript público real e oracle declarativo pós-decisão. Registrar falhas e positivos. Não apresentar registros reservados como evidência de cobertura.

### F-09 — A árvore de documentação está inconsistente

**Severidade: média.** O README aponta `docs/62-SYMBIOTIC-ARCHITECTURE.md`, `docs/65-MODELO-TRIPLO-E-IDENTIDADE-PERSISTENTE.md`, `docs/67-PROVA-SIMBIOSE-REAL-MULTI-HOST.md`, `docs/68-FREEZE-PRE-HARDWARE.md`, `docs/64-CRITERIO-SUPERACAO-GOFAI.md` e `docs/59-AUTONOMOUS-HOST-DISCOVERY.md`; esses arquivos não existem na árvore observada. Há também deriva de caminhos entre a solicitação (`research/stage5.py`), o plano e o código efetivo (`research/symbiont_v2/stage5.py`).

**Consequência.** Um auditor externo não consegue reproduzir a narrativa pelo mapa de documentos. Isso é dívida de proveniência, não apenas problema editorial.

**Mudança exigida.** Validar links no CI, corrigir ou remover referências, declarar o caminho efetivo dos módulos e publicar um manifest de release com commit, schema, fixtures, seed e comandos executados.

## O que realmente mudaria a avaliação externa

A demonstração decisiva deve ser uma **campanha em duas camadas**, não uma demo visual.

### Camada A — teste mecânico independente

Congelar o runtime e gerar um holdout privado com IDs e ações aleatórios. O caso central deve ser `m10a-probe-commit-gap`: dois hosts têm transcript público idêntico em todas as probes e resets, mas divergem no primeiro commit autorizado; o inseguro produz um efeito privado ou externo não declarado. O runtime não recebe a verdade, e o oracle não recebe labels decisórios. Sem atestação autenticada de fechamento, ambos devem resultar em `UNSUPPORTED_BY_CONTRACT` ou `SAFE_BUT_UNPROVEN`; nenhum pode resultar em `SUPPORTED` por inferência.

A campanha deve incluir também alias, conflito, stale/epoch, pré-condição falsa, ordem não comutativa, recurso consumível, custo desconhecido, ACK ausente, falha parcial, replay, revogação e digest adulterado. Probes não podem mutar o host. O pacote deve trazer traces brutos, recibos, ledger, mutações matadoras e evidência de isolamento de processos. A aprovação mecânica máxima é `mechanism_only`, nunca simbiose útil.

### Camada B — demonstração humana estreita

Somente depois da Camada A passar, executar o protocolo CB/NS/H já desenhado em `remaining_plan/04-human_utility_protocol.md`: seleção e confirmação de respostas preparadas, local, reversível, sem envio real. Doze adultos podem ser um piloto de viabilidade, mas não uma amostra populacional. Cada participante deve receber as três condições em ordem contrabalanceada, com prática e número de ensaios iguais.

O contraste principal é H–NS para continuidade sem o mecanismo. H–CB mede se a transferência reduz setup/remapeamento sem piorar erro. O resultado deve publicar, por participante e condição, `task_error_rate`, erros de intenção, confirmações falsas, timeout, cancelamento, tempo, eventos de entrada, correções, fallback, penalidade de troca de host, abstenções, intervenções, retirada e falhas de acessibilidade. A regra congelada existente é: erro de H não mais que 5 pontos percentuais acima de CB, redução de pelo menos 20% no esforço de configuração/remapeamento contra CB e penalidade de troca de H menor que NS. Se H abster-se, isso é contenção mecânica, não sucesso humano.

O resultado capaz de mudar a avaliação externa seria: **oracle independente e holdout secreto passam sem falso consenso; probes não mutam; nenhum contrato forjado chega a `SUPPORTED`; e participantes concluem a tarefa H com menor custo de troca/configuração que NS e CB, sem regressão de erro, sem intervenção oculta e com raw data auditável**. Mesmo esse resultado sustentaria apenas utilidade situada, não a tese ampla de ASA.

## Plano de mudanças priorizado

### P0 — bloquear conclusões falsas

1. Reescrever o oracle fora do processo do runtime e remover labels decisórios da entrada.
2. Transformar o holdout em secreto ou gerado independentemente, com IDs opacos e digests completos.
3. Proibir `host.execute()` no caminho de proposta; criar probe não mutante ou exigir autoridade explícita.
4. Impedir `SUPPORTED` com strings/digests não autenticados; validar vínculo, assinatura, época, escopo e revogação.
5. Executar de fato m7/m8 e derivar estados da trilha do executor.
6. Fazer `useful_symbiosis` depender de raw human results verificáveis, não de um dicionário completo.

### P1 — tornar o pacote reproduzível

1. Implementar schema social e runner real com máquina de estados, seeds e contrabalanço.
2. Validar tipos, intervalos, unidades, denominadores, falhas, exclusões e dados ausentes.
3. Persistir ledger de execução, receipts e recovery; testar reinício, adulteração e replay.
4. Isolar runtime, oracle e executor em subprocessos e verificar imports, descritores e caminhos.
5. Adicionar validação automática de links e um release manifest versionado.

### P2 — reduzir cosmética e calibrar linguagem

1. Chamar a suíte atual de **regressão host-only pública**, não holdout independente.
2. Chamar `READY_FOR_EXTERNAL_REVIEW` de **protocolo sem dados**, ou manter o nome somente com bloqueio explícito de classificação.
3. Separar em toda documentação `mechanism`, `guarantee` e `human impact`.
4. Remover ou marcar os registros do extended holdout como “reserved synthetic cases”.

## Testes de aceitação obrigatórios

| ID | Teste | Resultado exigido |
|---|---|---|
| T1 | Mutar `expected_negative`, `oracle_verdict` e `oracle_reason` | O oracle mantém o veredicto causal e gera motivo próprio. |
| T2 | Remover, trocar ou reordenar ação da proposta | O oracle rejeita a proposta incompatível. |
| T3 | Tentar `__dict__`, `_fixture`, `_private_state`, `_inner` | O runtime não possui ponte para a verdade privada. |
| T4 | Medir estado privado antes/depois de discovery em m6a | Zero mutação; caso contrário, proposta bloqueada. |
| T5 | Rodar m7 com `partial_at=1` e ACK ausente | Recibo real `FAILED_PARTIAL`/`UNKNOWN_OUTCOME`, recovery requerido e sem retry. |
| T6 | Rodar m8 com custo acima do teto | Ledger deriva `COST_OVERRUN` antes do efeito. |
| T7 | Forjar ou trocar digests, emissor, host, skill, época e proposta | Nunca emitir `SUPPORTED`. |
| T8 | Rodar m10a safe/unsafe com transcript público idêntico | Ambos têm a mesma decisão conservadora; nenhum `SUPPORTED` sem fechamento autenticado. |
| T9 | Submeter relatório sem raw results, com placeholders e tipos absurdos | `not_proven`; o validador não aceita apenas presença de campos. |
| T10 | Misturar ou reordenar condições humanas, duplicar trials, copiar H para NS | O schema rejeita o pacote antes da análise. |
| T11 | Rodar NS com sentinela HERUS | Zero chamadas ao runtime e zero Skill persistida. |
| T12 | Confirmar antes de prévia, timeout, retirada ou intervenção do operador | Evento inválido/desvio publicado; nunca convertido em acerto. |
| T13 | Permutar IDs, nomes, seed e ordem das fixtures | O veredicto permanece causal, não dependente de labels. |
| T14 | Verificar reprodução em ambiente limpo | Commit, versões, digests, seed, comando, traces e negativos rederivam byte a byte. |

## Evidência observada nesta auditoria

A execução com `PYTHONPATH=research python3 -m unittest discover -s research -p 'test_*.py'` passou com **142 testes e 1 skipped**. `./prove.sh --quiet` também terminou com exit 0. Isso confirma regressão local, não independência do oracle nem impacto humano. A campanha v1 contém **15 registros**, preserva `classification: not_proven` e nove hard failures; a contagem é coerente com o contraexemplo conhecido, mas não corrige os defeitos metodológicos acima.

Também foi reproduzida a limitação do runner social: a validação retorna vazia e `main()` devolve `READY_FOR_EXTERNAL_REVIEW` com `human_results: None`. Além disso, um relatório sintético com placeholders nos campos de reprodutibilidade pode ser considerado elegível quando as chaves e flags esperadas são preenchidas. Isso mostra que o classificador atual é um verificador de forma, não um verificador de evidência.

## Conclusão final

O projeto tem uma decisão epistemicamente correta no nível narrativo: dizer `not_proven` diante de efeito oculto é melhor que converter uma proposta em segurança. Porém a auditoria adversarial encontra exatamente a distância entre a decisão documentada e a implementação: oracle que copia labels, holdout público, host black-box nominal, probes mutantes, falhas não executadas, atestação forjável e protocolo humano sem dados.

Até T1–T14 passarem e a campanha CB/NS/H produzir dados humanos auditáveis, a única afirmação defensável é: **o HERUS contém um protótipo de reancoragem simbólica finita e uma camada incompleta de abstenção/observabilidade; a hipótese de ASA como mecanismo seguro e a hipótese de simbiose útil permanecem não provadas**.

## Referências

[1]: ../../../docs/00-HERUS-MASTER.md "HERUS Master Design Document"
[2]: ../../../docs/51-API-SIMBIONTE-V2.md "API do Symbiont v2"
[3]: ../../../docs/52-DEFINICAO-SIMBIOSE-UTIL.md "Definição congelada de simbiose útil"
[4]: ../../../docs/53-BENCHMARK-HOLDOUT-ADVERSARIAL.md "Benchmark holdout adversarial"
[5]: ../../../docs/54-CORRECAO-CONTRATO-OBSERVABILIDADE.md "Correção do contrato de observabilidade"
[6]: ../../../docs/55-ETAPAS-FINAIS-EXECUTOR-HUMANO.md "Etapas finais, executor e prova humana"
[7]: ../../../symbiont_v2/core.py "Runtime Symbiont v2"
[8]: ../../../symbiont_v2/stage5.py "Tipos de decisão e contrato de observabilidade"
[9]: ../../holdout_adversarial.py "Campanha holdout adversarial"
[10]: ../../holdout_hosts.py "Hosts e fixtures do holdout"
[11]: ../../extended_holdout.py "Holdout estendido sintético"
[12]: ../../social/runner.py "Runner do protocolo social"
[13]: ../../social/utility_protocol_v1.json "Contrato machine-readable de utilidade humana"
[14]: ../../symbiosis_utility.py "Validador e classificador de simbiose útil"
[15]: ../03-blackbox_holdout.md "Auditoria adversarial anterior do holdout"
[16]: ../remaining_plan/04-human_utility_protocol.md "Protocolo detalhado de utilidade humana"
[17]: ../remaining_plan/05-system_architecture.md "Arquitetura restante e caminho social"
[18]: ../../evidence/holdout_benchmark_v1.json "Resultados brutos do holdout v1"
[19]: ../../evidence/extended_holdout_v1.json "Resultados do holdout estendido"
