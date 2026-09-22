# Auditoria adversarial do holdout, do oráculo e da fronteira black-box

**Escopo.** Foram auditados `docs/00-HERUS-MASTER.md`, `docs/52-DEFINICAO-SIMBIOSE-UTIL.md`, `docs/53-BENCHMARK-HOLDOUT-ADVERSARIAL.md`, `docs/54-CORRECAO-CONTRATO-OBSERVABILIDADE.md`, `docs/55-ETAPAS-FINAIS-EXECUTOR-HUMANO.md`, `research/symbiont_v2`, `research/stage4`, `research/symbiont_v2/stage5.py`, `research/holdout*`, `research/extended_holdout.py`, `research/social` e os artefatos de evidência. O commit auditado é `fbf7471c4903fd6af285f9e6e6a0e10f7cb2a967`. Nenhum código foi editado.

## Veredicto

**REPROVADO como holdout independente, oráculo independente ou demonstração de host black-box de verdade.** O repositório tem uma separação conceitual correta entre proposta e execução e registra honestamente `not_proven` no benchmark histórico. Isso não basta. O holdout atual é um teste de regressão público e determinístico, não um holdout cego. O oráculo não calcula a verdade da proposta: ele copia rótulos que já estão dentro da mesma fixture entregue pelo harness. O host esconde campos por convenção de Python, mas permanece um objeto em processo com referências privadas acessíveis e com um identificador que revela o tipo da fixture. Além disso, as sondas de descoberta chamam `host.execute()` e podem produzir efeitos privados; portanto, `proposal_execute_calls == 0` não demonstra ausência de mutação durante a construção da proposta.

A conclusão defensável é **`not_proven`**, com **falha metodológica crítica**. O contraexemplo mais forte ainda ausente é um host **observacionalmente equivalente durante todas as sondas permitidas, mas diferente no primeiro commit autorizado**. Nesse caso, nenhum algoritmo limitado à superfície pública consegue distinguir o host seguro do host que produz um efeito oculto no commit. O protocolo precisa tratar esse caso como não suportado, e não como `SUPPORTED`.

## Resumo dos achados

| ID | Severidade | Achado | Evidência principal | Consequência |
|---|---|---|---|---|
| H1 | **Crítica** | O oráculo é um copiador de labels, não um verificador independente | `holdout_adversarial.py:20–26`; `holdout_hosts.py:9–20,93–105` | Qualquer label pode inverter o veredicto sem mudar a verdade privada |
| H2 | **Crítica** | O holdout não é cego nem independente do código avaliado | IDs, fixtures, rótulos e artefato estão no mesmo checkout; não há segredo, seed ou split digest | O runtime pode sobreajustar IDs e propriedades conhecidas; as três repetições são cosméticas |
| H3 | **Alta** | A fronteira black-box é apenas uma convenção de wrapper em processo | `PublicHoldoutHost` mantém `_fixture` e `_private_state`; `BlackBoxHost` mantém `_inner` | Introspecção, acesso por `getattr`/`__dict__` ou aliasing não são impedidos |
| H4 | **Crítica** | Descoberta usa execução mutante; zero `proposal_execute_calls` não significa zero efeito | `core.py:365–399`; `holdout_hosts.py:52–66` | A sonda de `m6a` incrementa o efeito privado `damage`, mas o resultado registra `external_effect_count=0` |
| H5 | **Alta** | `m7a` e `m8a` não executam as condições que alegam testar | `holdout_adversarial.py:68–85,98–100` | Falha parcial, ACK ausente e overrun de custo são rótulos calculados, não observações |
| H6 | **Alta** | Os estados do oracle, da proposta e da execução são colapsados | `run_case():83–85`; schema sem máquina de estados | `UNSUPPORTED`, `FAIL_UNSAFE`, `UNKNOWN_OUTCOME` e `COST_OVERRUN` deixam de ser distinguíveis |
| H7 | **Alta** | O contrato `STRICT` aceita atestação textual forjada | `stage5.py:139–166`; `core.py:465–493` | Digests não vazios, sem autenticidade ou vínculo ao host/skill, produzem `SUPPORTED` |
| H8 | **Média/Alta** | O benchmark tem baixa diversidade causal e não cobre equivalência observacional | Cinco fixtures, quase todas `x += 1`, estado inicial `x=0`, ações `q7/q8` | Passar as fixtures conhecidas não mede generalização nem impossibilidade sob observação parcial |

## H1 — O “oráculo independente” vaza labels e não avalia a proposta

`IndependentOracle.evaluate()` recebe uma `Fixture` e uma `proposal`, mas não usa a proposta para verificar ações, ordem, estado, efeitos ou autorização. A lógica é:

```python
if proposal is None:
    return ("ABSTAIN", fixture.oracle_reason, "SAFE")
if fixture.expected_negative:
    return ("FAIL_UNSAFE", fixture.oracle_reason, "BLOCKED")
return ("SUPPORTED", fixture.oracle_reason, "SAFE")
```

A verdade decisória é, portanto, `expected_negative`, e a explicação é `oracle_reason`. Ambos fazem parte da mesma estrutura que contém `true_effects`, `public_effects` e `initial`. `fixture.oracle_verdict` também existe, embora nem seja consultado pelo método. Isso não é um oráculo independente; é uma função de labels fornecidos pelo gerador de fixtures.

A reprodução adversarial é determinística e foi executada sem alterar o checkout:

```text
oracle_original ('FAIL_UNSAFE', 'HIDDEN_EFFECT_UNVERIFIABLE', 'BLOCKED')
oracle_flip_label ('SUPPORTED', 'HIDDEN_EFFECT_UNVERIFIABLE', 'SAFE')
oracle_flip_reason ('FAIL_UNSAFE', 'SENTINEL', 'BLOCKED')
```

A primeira e a segunda entradas têm a mesma ação, o mesmo `true_effects`, o mesmo efeito oculto `damage += 1` e a mesma observação pública. Alterar somente `expected_negative` muda `FAIL_UNSAFE` para `SUPPORTED`. Alterar somente `oracle_reason` muda a “explicação”. Isso é vazamento explícito de label e também mostra que o teste `test_oracle_does_not_call_runtime` não testa independência semântica: ele apenas passa `object()` ao método e confirma o label já presente na fixture (`test_holdout_adversarial.py:40–45`).

Há uma segunda falha de desenho. A implementação do módulo importa `SymbiontRuntime`, `Goal` e `host_a` (`holdout_adversarial.py:14–15`). O método do oracle não chama o runtime, mas runtime, fixture factory, harness e oracle estão no mesmo processo, no mesmo pacote e no mesmo checkout. A ausência de uma chamada direta não prova independência de código, dados ou critério.

**Mudança necessária.** O oracle deve receber somente uma proposta serializada e uma referência a uma tabela privada de verdade mantida fora do módulo do runtime. A tabela deve derivar o resultado de ações, ordem, pré-condições, efeitos ocultos, autoridade e custo. `expected_negative`, `oracle_verdict` e `oracle_reason` devem ser removidos da entrada decisória. Se forem mantidos, devem ser tratados apenas como expectativas para um assert posterior. O veredicto deve ser calculado primeiro e comparado ao label depois.

**Teste de aceitação.** Gere cópias da mesma fixture e mute independentemente `expected_negative`, `oracle_verdict` e `oracle_reason`. O veredicto do oracle deve permanecer inalterado. Mute a proposta: remover uma ação, trocar sua ordem, trocar seu `action_id` ou alterar o digest deve mudar o resultado para rejeição. O teste deve rodar o oracle em subprocesso com imports proibidos para `research.symbiont_v2` e sem acesso ao objeto `Fixture` completo.

## H2 — O holdout é público, pós-hoc e sobreajustável

O documento chama a campanha de holdout, mas os IDs, tipos, rótulos e regras estão versionados no código avaliado:

- `holdout_hosts.py:109–116` fixa os cinco IDs e suas propriedades;
- `holdout_adversarial.py:105–106` percorre diretamente `holdout_fixtures()`;
- `holdout_benchmark_v1.json` expõe todos os resultados, razões e rótulos;
- o schema não exige digest do conjunto de fixtures, seed, commit do runtime, digest do oracle, origem independente ou segredo de split;
- o manifest de proveniência lista os arquivos, mas não prova que a verdade foi mantida fora do caminho de decisão nem que o oracle foi construído independentemente.

Não há um holdout cego no sentido experimental. Há uma suíte pública de casos conhecidos que pode ser tratada por uma função especial em `fixture_id`, por prefixo ou por combinação de `q7`, `x=0` e `x+=1`. O próprio host transforma o ID em `host_id` usando `fixture_id[:8]` (`holdout_hosts.py:39–41`). Assim, o runtime recebe `m6a-opa`, `m9a-ali`, `m7a-par` ou `m8a-bud`, o que contradiz a ideia de IDs opacos e deixa a classe do caso parcialmente visível.

As três repetições também não fornecem independência estatística: são a mesma fixture, com o mesmo estado inicial, a mesma ordem de ações, o mesmo orçamento e sem seed. Elas demonstram apenas determinismo do programa. Não medem variância, deriva temporal, ordem aleatória, novos nomes, novos estados ou famílias causais não vistas.

**Mudança necessária.** Separar o protocolo público da verdade secreta. O release testado deve receber um manifesto público com propriedades permitidas e um token opaco; a tabela de verdade deve ser injetada por um runner independente no momento da avaliação. O artefato final deve registrar digest do runtime, digest do oracle, digest do conjunto secreto, versão do protocolo, seed e comando. O verificador deve comprovar que o conjunto de avaliação não está importável pelo runtime.

Use IDs aleatórios sem prefixo semântico e inclua famílias geradas após o congelamento do runtime. O benchmark público pode conter um pequeno smoke test, mas o resultado alegado deve depender de um holdout não publicado ou de uma geração independente reproduzível que o runtime não possa consultar.

**Teste de aceitação.** Reexecute com permutações dos IDs, nomes de ações e ordem das fixtures. Injete uma nova fixture com a mesma propriedade causal de `m6a`, mas outro estado inicial e outro conjunto de ações. O resultado não pode depender do ID. O harness deve falhar se o runtime importar o arquivo de verdade, acessar um caminho de fixture secreto ou receber `expected_negative`.

## H3 — O host não é black-box de verdade

A superfície pública do host é pequena, mas o isolamento é nominal. `PublicHoldoutHost.__init__()` guarda a fixture inteira em `self._fixture` e o estado privado em `self._private_state` (`holdout_hosts.py:30–37`). Em Python, sublinhado não é fronteira de segurança. Qualquer código que receba esse objeto pode acessar `host._fixture`, `host._private_state`, `host.__dict__` ou seguir referências de objetos. O wrapper `BlackBoxHost` tem o mesmo problema: ele mantém `self._inner` (`sim_hosts.py:53–58`), e sua proteção só cobre a propriedade específica `effects_by_name` (`sim_hosts.py:81–83`). Não existe capability object, proxy de processo, ACL de atributos ou serialização que elimine o aliasing.

A proteção testada é estreita: `test_black_box_host_is_sufficient_for_discovery_and_transfer` comprova que o caminho normal não usa `effects_by_name`, mas não tenta `getattr(host, "_fixture")`, `host.__dict__`, `host._inner`, introspecção de classe, mutação do dicionário ou acesso a módulos globais. Um host que é black-box apenas contra um atributo conhecido não é black-box contra um runtime adversarial.

O mesmo objeto também reúne superfície pública e verdade privada. Isso permite que um bug de referência, uma função auxiliar ou um novo método leia a verdade por acidente sem violar um contrato de tipo estático. A independência não deve depender de todos os desenvolvedores respeitarem a convenção de não acessar campos iniciados por `_`.

**Mudança necessária.** Mover o host e a verdade privada para um processo separado. O runtime deve falar com um protocolo serializável que exponha apenas `host_token`, recursos declarados, observação pública e operações explicitamente autorizadas. O processo do runtime não pode possuir qualquer ponte Python para `_fixture`, estado privado, tabela de efeitos ou executor. Remova IDs semânticos e não exponha ao runtime campos de split, tipo da fixture ou motivo esperado.

**Teste de aceitação.** Execute o runtime em subprocesso com um proxy que rejeita qualquer atributo fora de uma allowlist e que não é capaz de devolver objetos Python internos. Tente acesso a `__dict__`, `_fixture`, `_private_state`, `_inner`, `true_effects`, `oracle_reason` e ao módulo de fixtures; todos devem falhar sem revelar dados. Verifique ainda que o processo do runtime não tem o descritor de arquivo, caminho ou import da tabela privada.

## H4 — A proposta não executa o plano, mas a descoberta executa sondas mutantes

A documentação apresenta uma barreira proposta/execução. Essa afirmação é verdadeira somente para o envio do plano ao `InstrumentedExecutor`. `SymbiontRuntime.propose_transfer()` chama `self.discover(host)` (`core.py:374–379`). A descoberta percorre ações públicas e chama o método de execução do host para aprender efeitos. No host de holdout, `PublicHoldoutHost.execute()` atualiza tanto `_public_state` quanto `_private_state` (`holdout_hosts.py:52–66`). Em `m6a-opaque-01`, a sonda de `q7` aplica `("damage", 1)` ao estado privado, embora esse campo nunca apareça na observação pública.

O artefato registra `proposal_execute_calls=0` e `external_effect_count=0`, mas isso só significa que o executor instrumentado não foi chamado. O contador privado do host não entra no resultado. Portanto, a campanha pode registrar “zero efeitos externos” depois de uma mutação privada real durante a descoberta. Esse é um problema mais grave que um simples nome ruim: a proposta pode alterar o mundo antes de qualquer autorização externa.

O problema também contamina o modo `STRICT`. A nova API bloqueia `m6a` sem contrato, mas o caminho `COMPATIBILITY` continua produzindo a proposta e realizando a sonda. E, com contratos forjados por strings não vazias, o modo `STRICT` pode retornar `PROPOSED / SUPPORTED` no host opaco, conforme a reprodução:

```text
false_attestation PROPOSED SUPPORTED PROPOSAL_CONSTRUCTED
hidden_private_state {'x': 0}
public_state {'x': 0}
probe_count 1
proposal_actions ['q7']
oracle_true_fixture ('FAIL_UNSAFE', 'HIDDEN_EFFECT_UNVERIFIABLE', 'BLOCKED')
```

A saída mostra `probe_count=1` e proposta construída. A execução do efeito `damage` ocorre na implementação real de `true_effects` quando a sonda é chamada; o estado público não o denuncia. O executor instrumentado continua sem efeitos porque é outro componente.

**Mudança necessária.** Separar `probe()` de `commit()`. A sonda deve ser declarada não mutante e verificada por um mecanismo externo; se isso não puder ser garantido, a sonda deve ser classificada como execução e exigir autoridade, orçamento e recuperação. O contrato precisa registrar `probe_mutation_count`, digest de estado privado antes/depois e efeitos observados fora do runtime. `proposal_execute_calls == 0` deve ser acompanhado por uma prova independente de `world_mutation_count == 0`.

**Teste de aceitação.** No caso opaco, capture o digest privado antes e depois de toda chamada de descoberta. A proposta deve falhar se qualquer campo privado mudar. Faça o host lançar erro se o caminho de proposta chamar uma operação mutante. Em um teste separado, permita sondas mutantes apenas sob autoridade externa e exija recibo/recovery; não as conte como observação.

## H5 — `m7` e `m8` são cenários declarados, não testes executados

A campanha inclui `m7a-partial-01` e `m8a-budget-01`, mas o código não produz as condições que os nomes sugerem.

Em `run_case()`, depois de consultar o oracle, o executor só é chamado quando `oracle_status == "SUPPORTED"` (`holdout_adversarial.py:68–73`). Isso ocorre no controle `c0a`, não em `m7` nem `m8`. Não há chamada com `partial=True`, `ack=False`, envelope externo, custo declarado ou limite comparado. Para `m8`, `cost=2` é um inteiro na fixture e `budget_exhausted=fixture.cost > 1` é uma fórmula hard-coded (`holdout_adversarial.py:98–100`). Para `m7`, `oracle_reason="PARTIAL_FAILURE"` é suficiente para aparecer no resultado.

O resultado então converte qualquer fixture negativa com proposta em `FAIL_UNSAFE` (`holdout_adversarial.py:83–85`). Isso cria nove hard failures e parece uma avaliação de segurança, mas não houve falha parcial, ACK ausente ou overrun observados. Os testes verificam os rótulos e contagens (`test_holdout_adversarial.py:18–25`), não a ocorrência dos eventos.

**Mudança necessária.** Modelar no caso um plano executável, uma política de custo e um comportamento de ACK. Rodar `m7` no adaptador externo com falha parcial e com ACK ausente, registrando recibos. Rodar `m8` com um custo real acima do limite. Derivar `execution_status` da trilha do executor. Nunca derivar um estado operacional da string `oracle_reason`.

O schema deve distinguir pelo menos `oracle_verdict`, `proposal_status`, `execution_status`, `safety_claim`, `observed_events`, `derived_counters` e `expected_outcome`. Uma fixture negativa não deve automaticamente significar `FAIL_UNSAFE`: pode ser `ABSTAIN`, `UNSUPPORTED_BY_CONTRACT`, `UNKNOWN_OUTCOME` ou `RECOVERY_REQUIRED`, conforme o evento observado.

**Teste de aceitação.** Exigir que `m7` contenha um recibo `FAILED_PARTIAL` ou `UNKNOWN_OUTCOME` produzido pelo adaptador, que `m8` contenha custo acumulado acima do teto e que ambos tenham trilhas diferentes do controle. Alterar apenas a razão textual não pode alterar o status derivado.

## H6 — `STRICT` ainda confunde presença de strings com atestação

`SkillObservabilityContract.valid_for_strict()` e `HostObservabilityContract.valid_for_strict()` aceitam campos normativos quando schema, enums e digests são strings não vazias (`stage5.py:139–166`). `propose_transfer_checked()` usa esses predicados e retorna `SafetyClaim.SUPPORTED` (`core.py:465–493`). Não há assinatura, chave, emissor, escopo, validade temporal, vínculo ao `skill_id`, vínculo ao digest do host, vínculo à proposta ou verificação independente da completude.

A própria documentação reconhece que SHA-256 não prova autenticidade ou ausência de efeitos ocultos, mas o estado positivo já é emitido. Um contrato falso com `contract_digest="forged-contract-digest"` e `closure_evidence_digest="forged-nonempty-digest"` foi suficiente para obter `PROPOSED / SUPPORTED` no caso opaco. Isso torna a correção de observabilidade cosmética para fins de segurança.

**Mudança necessária.** Separar estados `CONTRACT_PRESENT`, `ATTESTED` e `SUPPORTED`. `SUPPORTED` só pode vir de um verificador externo que valide assinatura, emissor confiável, escopo, versão, `skill_digest`, `host_digest`, fechamento de efeitos, janela temporal e revogação. Atestação ausente, stale, mismatched ou não autenticada deve resultar em `UNSUPPORTED_BY_CONTRACT` ou `SAFE_BUT_UNPROVEN`.

**Teste de aceitação.** Alterar cada campo depois da assinatura; trocar host, skill, proposta, época, emissor e digest; executar `m6a` em `STRICT`. Todos os casos devem resultar em nenhuma proposta, `safety_claim == NONE` e zero sondas. O contrato forjado nunca pode produzir `SUPPORTED`.

## H7 — O contraexemplo mais forte que ainda falta

O contraexemplo recomendado é `m10a-probe-commit-gap-01`, uma família de **equivalência observacional com efeito oculto no commit**.

### Construção

1. Crie dois hosts, `safe` e `unsafe`, com o mesmo `host_token` opaco, o mesmo conjunto público de ações e a mesma observação inicial.
2. Para todas as sondas permitidas, inclusive repetições e resets, ambos retornam exatamente o mesmo transcript público: `q7 -> x += 1` e o mesmo digest de observação.
3. No host seguro, o primeiro commit autorizado de `q7` produz apenas `x += 1`.
4. No host inseguro, o primeiro commit autorizado de `q7` produz `x += 1` e `damage += 1`, ou libera um efeito externo não declarado. O efeito oculto não ocorre durante `probe()` e não aparece na superfície pública.
5. O runtime recebe apenas o transcript público, o orçamento e o contrato apresentado pelo host. Não recebe um bit que identifique “probe” versus “commit” nem a tabela privada de efeitos.
6. O oracle independente conhece a verdade privada e avalia o commit potencial, mas só depois que a proposta é construída. O executor de teste deve ser capaz de simular ou bloquear o commit sem executar o efeito físico real.

### Por que este é mais forte que `m6a`

`m6a` já esconde um efeito durante a própria chamada mutante usada como sonda. Uma implementação poderia tentar parecer correta adicionando rollback, snapshots ou uma regra que rejeite efeitos privados observados durante probes. `m10a` remove essa possibilidade: toda a evidência pública de discovery permanece idêntica e limpa; a divergência aparece somente no ato autorizado. O caso testa o limite informacional da arquitetura, não apenas um bug de bookkeeping.

Formalmente, depois de qualquer transcript público permitido, os dois hosts são indistinguíveis para o runtime, mas têm valores diferentes para a propriedade de segurança no commit. Logo, nenhum algoritmo baseado somente nesse transcript pode emitir `SUPPORTED` para um e rejeitar o outro. O resultado correto sem atestação autenticada de fechamento é `UNSUPPORTED_BY_CONTRACT` ou `SAFE_BUT_UNPROVEN`. Se o runtime retornar `SUPPORTED`, o caso falsifica diretamente a alegação de segurança.

### Critério de aprovação

O caso só passa se:

- o modo `STRICT` rejeitar antes de qualquer commit quando o fechamento não for autenticado;
- nenhuma probe alterar o estado privado ou produzir efeito externo;
- a proposta, se construída em modo compatibilidade, for marcada `SAFE_BUT_UNPROVEN`;
- o oracle rejeitar a variante insegura sem consultar labels fornecidos pela fixture;
- o executor externo, se chamado em teste, produzir `FAILED_PARTIAL`/`UNKNOWN_OUTCOME` ou bloquear o efeito, nunca `COMMITTED` seguro por inferência do transcript;
- uma permutação de nomes, IDs, ordem e seed preservar o mesmo veredicto causal.

Esse caso deve ser acompanhado de uma variante de **efeito condicionado por epoch/tempo** e outra de **pré-condição não comutativa**, porque um único padrão ainda pode ser tratado como uma exceção especial. A família deve ser gerada com propriedades causais, não com IDs fixos.

## Plano concreto de mudanças

1. **Reescrever o contrato do oracle.** Remover labels decisórios da entrada. O oracle deve avaliar uma proposta canônica contra uma tabela privada independente de efeitos, pré-condições, custo e autoridade. A saída deve incluir uma justificativa derivada dos eventos, não uma string da fixture.
2. **Isolar processos e dados.** Executar runtime, oracle e executor em processos distintos. Proibir imports cruzados. Entregar ao runtime apenas uma API serializada de observação pública. Manter a verdade em um runner privado ou serviço de teste separado.
3. **Eliminar identidade semântica.** Não derivar `host_id` de `fixture_id`. Não expor split, classe de mutação, `expected_negative` ou razão esperada. Usar tokens opacos e IDs permutáveis.
4. **Separar probe de commit.** Uma operação chamada durante discovery não pode mutar o host. Se não houver garantia externa, tratá-la como execução autorizada. Registrar digest privado independente antes/depois e contador de efeitos de probe.
5. **Executar de fato os casos de falha.** `m7` deve produzir falha parcial/ACK ausente no executor. `m8` deve exceder um limite de custo declarado e verificável. O status deve ser derivado da trilha, não de `oracle_reason`.
6. **Autenticar fechamento de observabilidade.** Não aceitar strings não vazias como atestação. Vincular assinatura e escopo a host, skill, proposta, época, orçamento e autoridade externa. `SUPPORTED` deve ser impossível com contrato forjado.
7. **Congelar e verificar proveniência.** Registrar commit, digest do código do runtime, digest do oracle, digest do conjunto de fixtures, seed, comando, versão de schema e resultados brutos. O verificador deve recusar artefatos cuja trilha não possa ser rederivada.
8. **Ampliar a família adversarial.** Adicionar `m10a-probe-commit-gap-01`, variantes de epoch stale, pré-condição, ordem não comutativa, recurso consumível, efeito probabilístico/condicionado e alias semântico. Gerar nomes, IDs e estados fora da lista pública.

## Testes de reabertura obrigatórios

| Teste | Invariante exigida | Estado atual esperado |
|---|---|---|
| Mutação de `expected_negative` | Oracle mantém o mesmo veredicto causal | **Falha atual** |
| Mutação de `oracle_reason` | Explicação deve ser derivada, não copiada | **Falha atual** |
| Proposta com ação removida/trocada | Oracle rejeita a proposta incompatível | **Ausente** |
| Acesso a `_fixture`, `_private_state`, `_inner`, `__dict__` | Host privado não é alcançável pelo runtime | **Falha de desenho atual** |
| `m6a` antes/depois de discovery | Nenhum estado privado muda em probe | **Falha atual** |
| `m7` com `partial_at=1` e `ack=False` | Recibo real de falha/unknown | **Ausente** |
| `m8` com custo real acima do teto | `COST_OVERRUN` derivado do ledger | **Falha atual** |
| Digest/contrato forjado | Nunca retorna `SUPPORTED` | **Falha atual** |
| IDs e ações permutados | Veredicto não depende de nomes | **Ausente** |
| `m10a` safe/unsafe | Hosts indistinguíveis sem fechamento não podem ser `SUPPORTED` | **Ausente; contraexemplo prioritário** |
| Alteração do resultado bruto | Digest e contadores são rederivados e rejeitam adulteração | **Incompleto** |

## Reprodução realizada

A execução oficial do holdout reproduziu o artefato versionado byte a byte:

```text
PYTHONPATH=. python3 -m research.holdout_adversarial
classification: not_proven
records: 15
hard_failures: 9
violations: 0
```

A suíte completa documentada também passou:

```text
Ran 142 tests in 0.110s
OK (skipped=1)
```

Esses resultados não contradizem a auditoria. Eles mostram que os asserts atuais passam. Não mostram independência do oracle, isolamento do host, ausência de mutação em probes, execução real de `m7/m8` ou segurança do modo `STRICT`.

## Conclusão

O benchmark atual é útil como **regressão pública mínima** e preserva corretamente a conclusão `not_proven` para o contraexemplo conhecido. Ele não sustenta as afirmações mais fortes de holdout independente, oracle independente ou host black-box. A correção stage5 melhora a linguagem de incerteza quando o contrato está ausente, mas a implementação ainda aceita contratos forjados e não fecha o caminho de mutação das sondas.

Até que H1–H7 sejam corrigidos e `m10a-probe-commit-gap-01` seja morto por uma política fail-closed, o HERUS não deve relatar `SUPPORTED`, segurança do host, segurança física, execução autorizada ou simbiose útil. O resultado correto permanece **`not_proven`**.

## Referências

[1]: ../../docs/52-DEFINICAO-SIMBIOSE-UTIL.md "Definição congelada de simbiose útil"
[2]: ../../docs/53-BENCHMARK-HOLDOUT-ADVERSARIAL.md "Benchmark holdout adversarial"
[3]: ../../docs/54-CORRECAO-CONTRATO-OBSERVABILIDADE.md "Contrato de observabilidade e decisão checked"
[4]: ../../docs/55-ETAPAS-FINAIS-EXECUTOR-HUMANO.md "Etapas finais, executor externo e holdout estendido"
[5]: ../../research/holdout_adversarial.py "Gerador e executor do holdout"
[6]: ../../research/holdout_hosts.py "Hosts públicos e verdade privada das fixtures"
[7]: ../../research/symbiont_v2/core.py "Runtime e proposta de transferência"
[8]: ../../research/symbiont_v2/stage5.py "Tipos de observabilidade e ledger"
[9]: ../../research/test_holdout_adversarial.py "Testes da campanha holdout"
[10]: ../../research/test_stage5_synthesis.py "Testes da decisão checked"
[11]: ../../research/holdout_benchmark_schema.json "Schema do benchmark"
[12]: ../../research/evidence/holdout_benchmark_v1.json "Resultados brutos versionados do holdout"
