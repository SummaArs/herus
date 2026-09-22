# Etapa 4 — deriva temporal e stale evidence

**ID do plano:** `ST4-TD-03`  
**Status:** proposta experimental; nenhuma alteração de implementação foi feita nesta frente.  
**Escopo:** hosts determinísticos que mudam depois de sondagem, `reset()` ou avanço de época; frescura de observações e evidências; revalidação; detecção de replay; contraprovas de reuso de evidência antiga.  
**Commit analisado:** `3fb3ca9d183cebe4df4d66c9cacae6c8220935c6`.  
**Artefato produzido:** este relatório de planejamento.

## 1. Decisão executiva

A próxima validação do HERUS deve tratar a evidência como válida somente dentro de um **escopo temporal explícito**. Esse escopo precisa identificar o vínculo público do hospedeiro, a época do hospedeiro, o episódio de sondagem e a versão fresca de cada observação. A identidade persistente `herus_id` pode sobreviver ao `rebind`; a evidência do hospedeiro não pode sobreviver automaticamente a uma mudança de host, `reset()`, rollover de época, alteração da tabela de ações ou sinal de observação stale.

A implementação atual já separa parcialmente identidade e contexto. `SymbiontRuntime.bind()` cria um novo `HostContext`, incrementa um contador `_epoch` local e zera `_sequence`. `discover()` também começa um contexto novo antes de sondar e chama `host.reset()` depois de cada ação. Contudo, esse `_epoch` é apenas um contador do runtime. Ele não é a época do hospedeiro. A `Observation` atual contém `sequence`, estado, última ação e digest, mas não contém época, qualidade, marcador de frescura ou prova de que um `reset()` restaurou o estado completo. `Evidence` registra somente o `host_id` textual em `provenance`. Consequentemente, uma observação pública idêntica pode gerar o mesmo digest em duas épocas privadas diferentes, e uma evidência antiga pode parecer compatível com o estado atual [4] [5].

O plano propõe três regras centrais:

1. **Evidência é local a `(binding_digest, host_epoch, episode_id)`**, não ao nome do host nem ao digest isolado do estado público.
2. **Qualquer quebra de frescura invalida primeiro e diagnostica depois.** A proposta correta é `ABSTAIN` ou `UNSUPPORTED`, não a reutilização da última evidência conhecida.
3. **Revalidação é uma nova coleta verificável.** Ela deve confirmar o vínculo público, a época, a observação inicial, a semântica de `reset()`, as transições usadas e a ausência de mistura entre épocas. Construir uma proposta não executa a proposta no alvo.

O baseline atual permanece útil para o caso estável de efeitos aditivos. Ele não deve ser reinterpretado como detector de deriva temporal. Nos casos em que a API não representa época, qualidade ou estado latente, o resultado esperado é `UNSUPPORTED_BY_CONTRACT` ou uma abstenção segura. Não se deve atribuir ao retorno `None` uma detecção que a superfície pública não tornou observável.

Esta frente mede somente um mecanismo host-only de validade temporal e contenção de stale evidence. Ela não demonstra simbiose geral, inteligência geral, segurança física, eficácia clínica ou utilidade social. A definição congelada separa explicitamente mecanismo de valor humano, exige baseline convencional para qualquer alegação de benefício e proíbe extrapolar uma transferência de simulador para impacto social [2] [3].

## 2. Arquitetura existente e limite temporal real

O ciclo normativo atual é `bind → observe → probe → record evidence → learn model → synthesize → verify → promote → rebind → transfer` [1] [4]. As partes relevantes para esta frente são as seguintes.

| Elemento atual | Comportamento observado | Lacuna temporal que o holdout deve expor |
|---|---|---|
| `SymbiontRuntime._epoch` e `HostModel.epoch` | O runtime incrementa um contador a cada `bind()`. O valor entra no digest do `HostModel`. | O contador descreve vínculos locais, não uma época ou versão do host. Dois hosts com o mesmo `host_id` podem ter tabelas de transição diferentes sem que o runtime saiba disso. |
| `Observation` | O digest cobre `sequence`, estado público e última ação. `valid()` verifica apenas a consistência desses bytes recebidos. | Não há `host_epoch`, qualidade (`fresh`, `stale`, `missing`) ou token de reset. O digest não prova completude, atualidade ou autenticidade física. |
| `Evidence` | Registra episódio, ação, estado anterior, posterior, efeito e `provenance`. O digest inclui o texto `host:<host_id>`. | Não há digest do vínculo, época do host, digest da observação inicial, atestado de reset ou escopo que permita rejeitar evidência de outra época. |
| `discover()` | Faz uma sondagem por ação, calcula o delta público e chama `reset()` após cada sonda. | Não verifica o estado observado depois do reset, não confirma que o reset restaura o host completo e pode misturar evidências se a equação mudar no meio da sessão. O retorno `None` de `reset()` não fornece uma confirmação. |
| `WorldModel.learn()` | Indexa transições por `(action_id, before.values)` e bloqueia conflito quando o mesmo par produz dois efeitos públicos diferentes. | Uma mudança nunca exercitada no mesmo estado, uma mudança apenas oculta ou uma mudança após um reset sem marcador pode deixar a evidência velha utilizável. |
| `AbstractSkill` e `PersistentMemory` | Skills verificadas são guardadas com `source_host` e digests das evidências usadas. | A memória não registra a época do host, o digest do vínculo, uma política de expiração ou uma revalidação posterior. `VERIFIED` histórico não distingue “verificado uma vez” de “válido no contexto atual”. |
| `promote()` | Reexecuta `verify()` no `HostContext` ativo antes de guardar a Skill. | A verificação é fresca apenas em relação ao modelo atual do runtime. Não há verificação do host epoch, do reset nem de que todas as evidências pertencem à mesma época. |
| `propose_transfer()` | Chama `discover(host)`, agrupa evidências atuais por `effect.delta` e rejeita matches ambíguos. Não executa o plano proposto. | Não há status ou motivo tipado de stale. A função pode produzir evidência de uma sessão que atravessou deriva se o host mudar durante as sondagens. O parâmetro `max_probes` é aplicado ao recorte de candidatos depois da chamada a `discover()`, enquanto a coleta usa o orçamento do runtime; a futura medição deve contabilizar sondas reais. |
| `ToyHost` e `BlackBoxHost` | `ToyHost.reset()` restaura estado, última ação e sequência para os valores iniciais. `BlackBoxHost` expõe apenas a interface pública no caminho nominal. | Não há host que preserve estado latente, mude depois de sondagem ou exponha uma época. O wrapper ainda possui `_inner`, portanto não é uma fronteira de processo contra introspecção adversarial [5]. |

O baseline foi reproduzido no commit analisado: nove testes passaram e o benchmark relatou três ações descobertas, Skill verificada e promovida, identidade persistente e transferência positiva no par `host_a()`/`host_b()` [6] [7]. Esse resultado é um controle de grounding por assinatura de efeito. Ele não testa a validade de uma evidência depois de `reset()`, época, substituição do host ou mudança de transição.

## 3. Vocabulário e modelo temporal proposto

### 3.1 Identidade, vínculo e época não são a mesma coisa

O plano deve manter quatro identificadores distintos:

- **`herus_id`:** identidade persistente do runtime. Deve sobreviver ao `rebind`.
- **`binding_digest`:** digest canônico da descrição pública observada no vínculo atual, incluindo `host_id`, recursos públicos, espaço seguro de ações e esquema de observação. Ele detecta mudança observável de contrato, mas não autentica um sensor.
- **`host_epoch`:** token ou versão pública do hospedeiro que muda quando a semântica, o estado latente ou a política de reset muda. Ele deve ser fornecido pela observação ou por um atestado público do adapter. O contador local `HostModel.epoch` não o substitui.
- **`episode_id` e `sequence`:** identificam uma sessão de sondagem e a ordem das observações dentro da sessão. Uma reinicialização de sequência só é válida se vier acompanhada de uma transição de época ou de uma regra de episódio explicitamente declarada.

O digest de uma observação futura deve incluir a representação pública canônica de `host_epoch`, `episode_id`, `sequence`, estado, última ação e qualidade. Não deve incluir o estado privado do oráculo. Como no plano de observabilidade parcial, esse digest atesta consistência dos bytes públicos recebidos; ele não prova que o mundo físico foi observado completamente [9].

### 3.2 Definição operacional de evidência stale

Uma evidência `e` é **stale** para um contexto atual `C` se qualquer uma das condições abaixo for verdadeira:

```text
stale(e, C) :=
    e.binding_digest != C.binding_digest
    ou e.host_epoch != C.host_epoch
    ou e.observation_quality != FRESH
    ou e.observation_epoch não estiver presente quando o contrato a exige
    ou e.episode_id não pertencer à revalidação corrente
    ou e.sequence for repetido, regressivo ou replayado no mesmo episódio
    ou o reset que separa e de C não tiver atestado válido
    ou o espaço público de ações, recursos ou esquema de observação tiver mudado
    ou a evidência não estiver incluída no recibo de revalidação corrente.
```

O predicado deve ser aplicado antes de `synthesize()`, `verify()`, `promote()` e `propose_transfer()`. Evidência stale pode ser conservada como histórico negativo ou para auditoria, mas nunca deve sustentar uma proposta, promoção ou previsão positiva.

Não é recomendável usar tempo de relógio como único TTL. Um relógio real introduziria nondeterminismo e não explicaria se houve mudança de semântica. A validade desta etapa deve usar contadores lógicos, época, sequência, digest e eventos de reset. Uma eventual política de expiração por tempo deverá ser uma extensão separada, com relógio injetável e seed congelada.

### 3.3 Revalidação e transições de estado

A proposta futura deve introduzir, sem ser implementada nesta tarefa, uma revalidação com recibo imutável. O recibo deve conter pelo menos `binding_digest`, `host_epoch`, digest da observação inicial, sequência de sondagem, digests das observações antes/depois, atestados de reset, conjunto de ações sondadas, orçamento consumido, ausência de conflitos e digest do contrato usado. O recibo não é uma autorização de execução.

O fluxo recomendado é:

1. **Abrir o vínculo.** Observar o host, validar o digest público, capturar `binding_digest` e `host_epoch` e criar um `HostContext` novo. Se a época estiver ausente, registrar `EPOCH_UNKNOWN`.
2. **Estabelecer o baseline.** Obter uma observação fresca e guardar o digest do estado inicial. A mesma projeção pública em outra época não é automaticamente o mesmo baseline.
3. **Sondar em uma única época.** Antes de cada ação, exigir observação fresca. Depois da ação, exigir observação fresca, ação retornada igual à ação enviada e epoch consistente. Se o epoch mudar, descartar o lote completo ou separar os lotes; nunca compor uma Skill com evidência de épocas diferentes.
4. **Verificar o reset.** Depois de `reset()`, obter observação ou recibo de reset. Confirmar a regra declarada: o reset pode restaurar o estado completo na mesma época, abrir novo episódio na mesma época ou avançar a época. Sem essa confirmação, toda evidência anterior ao reset fica inválida para uso positivo.
5. **Marcar a deriva.** Ao detectar mudança de epoch, fingerprint, ação segura, recurso, sequência, qualidade ou baseline, mover o contexto para `QUARANTINED` e invalidar a revalidação. Não tentar corrigir a tabela com uma média nem selecionar a última observação.
6. **Revalidar.** Começar um novo episódio com novo recibo. Repetir somente as sondagens permitidas pelo orçamento. A proposta só pode referenciar digests incluídos nesse recibo e um único escopo temporal consistente.
7. **Promover ou transferir.** Uma Skill histórica pode permanecer na memória como artefato, mas deve ter o estado de uso distinguido de `VERIFIED_FRESH`. Se seu contrato não contém uma especificação independente da origem, uma mudança de época do host de origem requer revalidação. Um alvo de transferência também requer evidência fresca no próprio vínculo; evidência do host de origem nunca é observação do alvo.

Uma política conservadora pode usar os estados `VERIFIED_HISTORICAL`, `REVALIDATION_REQUIRED`, `VERIFIED_FRESH`, `QUARANTINED` e `ABSTAIN`. A API atual só possui `CANDIDATE` e `VERIFIED`; portanto, qualquer experimento executado sem uma extensão de status deve registrar `None` como **seguro, porém diagnosticamente incompleto**, e não como detecção detalhada de deriva.

## 4. Contrato experimental e separação de artefatos

Cada fixture deve ser composta por três artefatos independentes:

| Artefato | Entregue a quem | Conteúdo permitido | Conteúdo proibido |
|---|---|---|---|
| Host público | Runtime | `host_id`, recursos públicos, ações seguras, observação, execução, reset e, na variante suportada, token de epoch/qualidade | Tabela privada de transições, estado latente, agenda de deriva, motivo esperado ou callback do oráculo |
| Oráculo privado | Harness | Estado verdadeiro, época verdadeira, agenda determinística, transições por época, semântica de reset, objetivo, classe e motivo esperado | Importar `core.py`, chamar `WorldModel`, reutilizar `_apply()` ou receber a decisão do runtime |
| Manifesto congelado | Revisor e reprodução | IDs, split, seed, digests, orçamento, contrato de observação, `expected_class`, `expected_reason` e contagens | Dicas no nome que revelem “stale”, “hidden”, “flip”, “safe” ou o resultado |

A fronteira deve ser um processo ou proxy de serialização que expõe somente a interface autorizada. O `BlackBoxHost` atual é um controle útil, mas seu `_inner` não deve ser compartilhado com o runtime em um holdout adversarial. O teste de não introspecção deve falhar ao acessar `__dict__`, closures, tabelas privadas ou campos de diagnóstico [5].

O registro lógico mínimo de cada caso é:

```text
fixture_id
split
seed
public_host
public_observation_schema
initial_public_observation
initial_private_state_digest  # somente no manifesto do oráculo
reset_contract
scheduled_events
public_action_order
objective
forbidden_predicates
budgets
expected_class
expected_reason
fixture_digest
oracle_digest
```

O runtime recebe uma projeção sem estado privado, sem classe esperada e sem agenda de deriva. A seed não substitui o digest canônico. Recomenda-se reservar `4403` para desenvolvimento, `4503` para validação e `4603` para holdout desta frente, mantendo as seeds já propostas nos planos de observabilidade parcial (`4102`, `4202`, `4302`) para as respectivas famílias [8] [9].

A agenda temporal deve ser expressa por contadores de operações, nunca por segundos de relógio. Exemplos determinísticos são `after_probe=1`, `after_reset=1`, `on_epoch=2` e `after_observe=3`. O manifesto deve registrar a ordem exata de `observe`, `execute`, `reset` e revalidação. O oráculo deve poder reproduzir cada traço sem acessar o runtime.

## 5. Fixtures determinísticas propostas

As IDs abaixo são nomes de planejamento. Na rodada final, o manifesto público deve usar identificadores opacos e separar a tabela privada que rotula a família. Cada fixture precisa ser executada em processo limpo e repetida três vezes com o mesmo commit, seed e manifesto.

| ID de plano | Split | Construção determinística | Evento temporal | Classe e resultado esperado |
|---|---|---|---|---|
| `T0` | desenvolvimento | Controle equivalente a `host_a()`/`host_b()`: estados completos, efeitos aditivos, epoch constante dentro da sessão, reset restaura estado e contrato público | Nenhuma deriva | `SUPPORTED`; proposta válida e evidência inteira no mesmo vínculo. Mede regressão, não robustez temporal. |
| `T1` | desenvolvimento | Uma ação pública `p7` produz `x:+1` no primeiro episódio e `x:-1` depois de uma sondagem numerada | Deriva imediatamente após a primeira sondagem | `ABSTAIN`/`UNSUPPORTED` com `TEMPORAL_DRIFT` ou `EPOCH_MISMATCH`. Reusar `x:+1` é falso consenso. |
| `T2` | desenvolvimento | `reset()` restaura `x=0,y=0` publicamente, mas alterna um bit privado que muda `p7` de `x:+1` para `x:0` | Deriva causada pelo reset; projeção pública inicial permanece igual | Se o bit não é público, `UNSUPPORTED`/abstenção obrigatória. O digest público atual não pode ser tratado como prova de estabilidade. |
| `T3` | validação | O host expõe um token de epoch público. O token muda após `reset()` e a tabela de ações permanece igual | Avanço observável de epoch sem mudança aparente do estado | Revalidação deve descartar o recibo anterior e, após nova sondagem consistente, aceitar somente a evidência do epoch atual. Qualquer referência anterior é rejeitada. |
| `T4` | validação | O host muda a tabela depois de `execute(p7)`, mas a alteração só é anunciada na próxima observação pelo token público | Deriva no meio da sessão; evidência anterior e posterior pertencem a epochs diferentes | `ABSTAIN` durante o lote misto. Nenhuma Skill pode ser sintetizada a partir de ações de epochs diferentes. |
| `T5` | validação | O host devolve uma observação cacheada com o mesmo estado e sequência anterior depois de um evento de drift | Replay ou atraso controlado da observação | `ABSTAIN` com `OBSERVATION_REPLAY` ou `STALE_OR_PARTIAL_OBSERVATION`. Usar o último valor fresco como fallback é proibido. |
| `T6` | validação | `reset()` deveria restaurar o baseline, mas retorna `x=0,y=1` ou um digest incompatível com o baseline declarado | Reset não restaurador | O reset é rejeitado; evidência da sessão é invalidada com `RESET_NOT_VERIFIED`. Não chamar o estado antigo de baseline. |
| `T7` | holdout | Mesmo `host_id`, mesmas ações e mesma projeção inicial; uma nova instância tem `binding_digest` diferente e remove ou altera o efeito de `p7` | Substituição do host com ID reaproveitado | `REVALIDATION_REQUIRED` antes de qualquer proposta. Uma proposta baseada no vínculo antigo é falso consenso; uma nova revalidação consistente pode ser aceita. |
| `T8` | holdout | Após o epoch rollover, uma ação deixa de pertencer ao espaço seguro ou um recurso público muda | Mudança de contrato público durante ou entre sessões | `ABSTAIN` com `SAFE_ACTION_SPACE_CHANGED` ou `HOST_BINDING_CHANGED`. A ação antiga não pode ser sondada por memória. |
| `T9` | holdout | O host altera somente um campo explicitamente declarado irrelevante para o objetivo e conserva a transição necessária | Deriva benigna, mas epoch muda e obriga nova validação | A evidência antiga ainda é stale. Depois de revalidação completa no novo epoch, `SUPPORTED`; sem revalidação, `ABSTAIN`. Mede não só recall de deriva, mas também recuperação segura. |
| `T10` | holdout | A mesma observação pública `x=0` ocorre nos epochs pares e ímpares; `p7` satisfaz a meta somente nos pares | Reset alterna estado latente não observável | `UNSUPPORTED`/`ABSTAIN`. Não contar uma proposta correta por acaso como detecção de época. |
| `T11` | holdout | Primeiro `p7` produz `x:+1`; após deriva, `p8` produz `y:+1`. Uma Skill composta exige os dois efeitos | Mistura de evidência pré e pós-deriva | `ABSTAIN` com `MIXED_EPOCH_EVIDENCE`; uma composição cujo estado final previsto parece correto continua inválida. |
| `T12` | holdout | O host publica epoch monotônico, mas uma mensagem atrasada apresenta epoch anterior com digest internamente válido | Rollback/replay de uma observação antiga | `ABSTAIN` com `EPOCH_ROLLBACK` ou `OBSERVATION_REPLAY`. O digest correto do pacote velho não o torna atual. |

As fixtures `T1`, `T2`, `T4`, `T5`, `T6`, `T10`, `T11` e `T12` são contraprovas. `T3`, `T7` e `T9` exercitam o caminho de recuperação, no qual a nova evidência deve poder ser aceita depois de uma revalidação que realmente complete. `T0` impede que toda abstenção seja confundida com sucesso. Cada família deve ter no mínimo um caso em que o contrato representa a decisão e um caso em que a decisão é deliberadamente `UNSUPPORTED_BY_CONTRACT`.

### 5.1 Traço privado e traço público de referência

Um oráculo pode materializar um host com estado verdadeiro `z=(x,y,m,e)`, em que `m` é um modo latente e `e` é a época verdadeira. A projeção pública `π(z)` pode omitir `m` e, na variante suportada, expor somente um token opaco para `e`. Um exemplo de agenda é:

```text
estado inicial: x=0, y=0, modo=0, epoch=11
p7 em modo=0: x := x + 1
reset #1: restaura x,y; modo := 1; epoch := 12
p7 em modo=1: x := x - 1
```

Se `modo` e `epoch` forem omitidos, `π(z)` pode ser idêntica antes de `p7` nos dois episódios. O resultado correto não é descobrir o bit privado por adivinhação; é recusar uma certificação positiva para uma meta que depende dele. Se o epoch público muda, a decisão deixa de ser invisível: a observação velha deve ser invalidada, e uma nova revalidação pode recuperar o caso.

### 5.2 Controle de semântica do reset

O manifesto precisa declarar qual das três semânticas é testada:

1. `RESET_SAME_EPOCH_FULL`: restaura o estado relevante e mantém a época; evidência anterior só pode ser reutilizada se o contrato disser que ela continua aplicável ao novo episódio.
2. `RESET_NEW_EPOCH`: restaura o estado público, mas abre época nova; toda evidência anterior fica stale até revalidação.
3. `RESET_PUBLIC_ONLY`: restaura apenas a projeção pública; campos latentes podem persistir. Para metas afetadas por campos latentes, o resultado é `UNSUPPORTED` ou abstenção.

O host atual só implementa uma variante simplificada de restauração, sem declarar qual garantia fornece. Logo, um resultado positivo do baseline não deve ser usado para presumir `RESET_SAME_EPOCH_FULL` em um host futuro [4] [5].

## 6. Hipóteses falsificáveis

As hipóteses devem ser congeladas no manifesto antes de qualquer execução do holdout. São hipóteses de mecanismo local e de contenção de evidência; não são hipóteses sobre compreensão geral ou benefício humano.

| ID | Hipótese | Falsificador observável |
|---|---|---|
| `H-ST4-TD-01` | **Frescura por vínculo e época.** Nenhuma proposta, verificação ou promoção positiva usa evidência cujo `binding_digest` ou `host_epoch` difira do contexto corrente. | Uma proposta válida pelo harness referencia um digest de host, época ou recibo anterior. |
| `H-ST4-TD-02` | **Deriva observável é detectada antes do uso.** Se a mudança de epoch, contrato ou fingerprint é pública, o runtime a detecta e invalida o lote antes de propor ou promover. | O runtime continua usando o lote antigo após a primeira observação que contém a mudança. |
| `H-ST4-TD-03` | **Reset não cria evidência nova por si só.** Um `reset()` sem atestado de restauração não autoriza reutilizar evidência anterior; uma mudança de epoch no reset invalida o recibo anterior. | Evidência anterior é aceita após reset não verificado, mesmo com baseline público igual. |
| `H-ST4-TD-04` | **Não há mistura de epochs.** Toda proposta aceita referencia evidências pertencentes a um único vínculo, época e recibo de revalidação. | Uma Skill ou `TransferProposal` aceita combina evidências de epochs diferentes. |
| `H-ST4-TD-05` | **Revalidação recupera casos representáveis.** Depois de uma deriva observável, nova coleta completa e consistente no epoch atual pode produzir uma proposta válida nos controles que o contrato declara suportáveis. | O caso positivo estável continua recusado depois de revalidação completa, ou a proposta ainda inclui evidência velha. |
| `H-ST4-TD-06` | **Deriva latente não vira alegação.** Quando duas épocas têm a mesma projeção pública e efeitos verdadeiros diferentes, sem marcador autorizado, a saída não pode ser positiva. | Qualquer proposta positiva é aceita pelo oráculo em `T2` ou `T10` sem informação adicional. |
| `H-ST4-TD-07` | **Observação stale não é fallback.** Digest válido de uma mensagem antiga, sequência repetida, epoch regressivo ou qualidade stale bloqueia a decisão; o runtime não usa o último valor fresco silenciosamente. | Ações posteriores são justificadas por observação replayada ou campo stale. |
| `H-ST4-TD-08` | **Trocar o host com mesmo nome não preserva contexto.** Reusar `host_id` não deve manter evidência, modelo ou atestado do vínculo anterior. | Uma instância substituta com mesmo ID recebe uma proposta sem revalidação do vínculo. |
| `H-ST4-TD-09` | **Identidade persiste sem arrastar evidência.** `herus_id` permanece igual ao atravessar epochs, enquanto o `HostContext` e os recibos stale são descartados ou quarantinados. | O ID muda, ou evidência antiga continua disponível como evidência fresca depois do rebind. |
| `H-ST4-TD-10` | **Orçamento e revalidação são explícitos.** A coleta de frescura termina em proposta ou abstenção dentro de `max_probes`, `max_steps` e orçamento de resets; esgotamento não é sucesso implícito. | A implementação executa sondas além do orçamento ou trata a falta de revalidação como custo zero. |
| `H-ST4-TD-11` | **Separação proposta/execução permanece.** Construir, validar ou rejeitar uma proposta não executa a ação proposta no host alvo. | O contador de execução aumenta além das sondas autorizadas durante `propose_transfer()`. |
| `H-ST4-TD-12` | **Decisão temporal é reprodutível.** Com mesmo commit, manifesto e seed, status, motivo, digests e contagens são iguais byte a byte. | Repetições divergem por relógio, ordem de iteração, atraso ou identificação acidental. |

A hipótese `H-ST4-TD-06` é deliberadamente assimétrica: não exige detectar um fato que o host ocultou. Ela exige que o runtime não certifique aquilo que não pode verificar. Essa distinção evita transformar ausência de observabilidade em suposta capacidade de inferência.

## 7. Métricas e regras de leitura

As métricas devem ser reportadas por fixture, split e família, com numerador e denominador. Uma média global não pode compensar uma única proposta insegura. A lista seguinte complementa as métricas de transferência, abstention e falso consenso já estabelecidas no contrato congelado [2] [3].

| Métrica | Cálculo | Critério proposto |
|---|---|---|
| **Frescura de evidência** | Propostas que referenciam somente evidências do `binding_digest`, `host_epoch` e recibo corrente / propostas avaliadas | 100%; qualquer referência stale é falha dura local. |
| **Uso de evidência stale** | Propostas ou promoções com ao menos um digest stale / oportunidades de uso stale | Limite duro: 0. |
| **Detecção de deriva observável** | Eventos de mudança pública detectados antes do próximo uso / eventos de mudança pública injetados | 100% no conjunto suportado. Reportar separadamente casos latentes. |
| **Não alegação de deriva latente** | Casos sem marcador público classificados como `ABSTAIN` ou `UNSUPPORTED` / casos latentes que afetam a meta | 100%; uma proposta positiva é falso consenso. |
| **Abstenção correta por drift** | Fixtures negativas sem proposta executável / fixtures negativas temporais | 100% em `T1`, `T2`, `T4`–`T8`, `T10`–`T12`, respeitando a classe congelada. |
| **Transferência validada após revalidação** | Propostas aceitas pelo executor independente depois de nova coleta / casos positivos representáveis | 100% nos positivos suportáveis; separar `UNSUPPORTED_BY_CONTRACT`. |
| **Mistura de épocas** | Propostas com evidências de mais de um epoch ou recibo / propostas | Limite duro: 0. |
| **Reset não verificado aceito** | Resets com baseline ou atestado incompatível que não invalidaram a sessão / resets testados | Limite duro: 0. |
| **Replay ou rollback aceito** | Observações com sequência/epoch repetido ou regressivo usadas em decisão / observações adversariais | Limite duro: 0. |
| **Invalidação antes do uso** | Casos em que a deriva foi detectada e nenhum uso positivo ocorreu antes da quarentena / deriva observável | 100%. Reportar número de operações até a quarentena, com limite de uma observação de detecção. |
| **Revalidação completa** | Recibos que contêm baseline, epoch, ordem, digests, resets e ausência de conflito / revalidações iniciadas | 100% dos recibos aceitos. |
| **Falsa invalidação do controle** | Casos `T0` e `T9` aceitos pelo oráculo depois de revalidação, mas recusados indevidamente / controles suportáveis | 0% depois de revalidação; antes dela, rejeitar evidência stale é comportamento esperado. |
| **Identidade persistente** | Casos em que `herus_id` antes/depois do rebind é igual / casos executados | 100%. Não compensa falhas de frescura. |
| **Separação proposta/execução** | Execuções do plano alvo além das sondas autorizadas / propostas construídas | Limite duro: 0. |
| **Custo de descoberta e revalidação** | `observe`, `execute`, `reset`, bytes, candidatos, epochs descartados e tempo lógico por fixture | Dentro dos orçamentos congelados; não usar latência de máquina como prova causal. |
| **Cobertura temporal** | Eventos exercitados: após sondagem, após reset, rollover, replay, substituição de host, mudança de ação e deriva latente / eventos planejados | Descritiva; não autoriza extrapolação para eventos não cobertos. |
| **Determinismo** | Igualdade byte a byte de manifestos, status, motivos, digests e resultados em três execuções | 100%; qualquer divergência invalida a rodada até ser explicada. |
| **Diagnóstico suficiente** | Abstenções com motivo temporal tipado ou `SAFE_BUT_UNPROVEN` / todas as abstenções | 100% na extensão futura; `None` atual deve ser contado separadamente como diagnóstico incompleto. |

`transfer_success` não deve ser contado sozinho. Um plano pode reproduzir o `expected_final` calculado pelo runtime e ainda violar o epoch, uma pré-condição temporal, um reset não restaurador ou um efeito privado. O executor independente deve reexecutar a trajetória verdadeira e avaliar a validade, não confiar em `TransferProposal.expected_final` [4] [8] [9].

## 8. Contraexemplos falsificáveis

Os seguintes traços devem aparecer como negativos explícitos no harness. Eles transformam a ameaça de stale evidence em uma falha observável.

### 8.1 Reuso simples depois de sondagem

```text
epoch 11, estado público x=0: probe(p7) -> x=1
reset; epoch 12, estado público x=0: p7 -> x=-1
modelo velho prevê p7 -> x=1
```

Se o runtime usa a primeira evidência para propor a ação no epoch 12, há falso consenso. O digest do estado público `x=0` não é suficiente para preservar validade.

### 8.2 Reset que restaura a projeção, mas não o estado latente

```text
epoch 11: (x=0, mode=0), p7 -> x=1
reset: (x=0, mode=1), mesma observação pública
epoch 12: p7 -> x=0 ou alarm=1
```

A repetição do baseline público não prova que o reset foi completo. Se `mode` é relevante e não é observável, a resposta é `UNSUPPORTED`/abstenção, não inferência de `mode=0`.

### 8.3 Evidência composta atravessando deriva

```text
epoch 11: p7 -> x=1
deriva após a primeira ação
epoch 12: q2 -> y=1
```

Uma Skill que guarda `p7,q2` pode prever `x=1,y=1`, mas nenhum epoch sustentou a trajetória inteira. Aceitar a composição revela mistura de evidências e deve falsificar `H-ST4-TD-04`.

### 8.4 Host substituto com o mesmo `host_id`

```text
host_id = H, binding A: p7 -> x=1
rebind para outra instância host_id = H, binding B: p7 -> x=-1
```

O nome não é um atestado de continuidade. O runtime deve construir novo contexto e exigir revalidação do vínculo. A `herus_id` pode permanecer; a tabela anterior não.

### 8.5 Digest válido de mensagem atrasada

```text
observação seq=4, epoch=12: x=1
observação replayada seq=3, epoch=12: x=0, digest internamente válido
```

Um digest criptográfico da mensagem replayada continua correto para os bytes antigos. Ele não prova atualidade. Aceitar a segunda observação como estado corrente falsifica `H-ST4-TD-07`.

### 8.6 Epoch privado invisível

```text
projeção pública em epoch 11: x=0
projeção pública em epoch 12: x=0
p7 é positivo somente em epoch 11
```

Este é um limite de informação, não uma falha que o runtime precise adivinhar. O oráculo deve exigir abstenção ou `UNSUPPORTED` para uma meta que depende do epoch. Não deve marcar como “detectado” um `None` que não traz motivo.

## 9. Riscos de circularidade e controles

### 9.1 Oráculo compartilhando o defeito do runtime

Se o oráculo calcular efeitos com `Effect.from_states()`, aplicar deltas com `SymbiontRuntime._apply()` ou decidir frescura com `WorldModel.predict()`, o mesmo defeito de época pode ser reproduzido dos dois lados. O oráculo deve ser declarativo e independente, idealmente em módulo e processo separados, com executor próprio de pré-condições, resets e custos [4]. Uma suíte de mutação deve trocar a transição de epoch, remover o atestado de reset e inverter uma ação sem alterar o runtime; pelo menos uma contraprova precisa falhar para cada mutação.

### 9.2 Host público carregando o oráculo por trás

Um objeto com `_inner`, `true_state`, `effects_by_name`, tabela de agenda ou callback de diagnóstico pode permitir introspecção acidental. O controle é um proxy serializado ou processo separado. O teste deve negar acesso a atributos fora da interface mínima e confirmar que duas instâncias com a mesma projeção pública produzem os mesmos bytes públicos, mesmo quando seus estados privados divergem [5] [9].

### 9.3 Epoch local confundido com epoch do host

O contador `_epoch` do runtime aumenta quando ele chama `bind()`. Isso não demonstra que o host mudou, nem detecta um host que mudou sem novo `bind()`. O manifesto e o relatório devem usar nomes distintos (`runtime_bind_epoch` versus `host_epoch`) e proibir que o primeiro seja usado como evidência do segundo.

### 9.4 Digest confundido com frescura ou autenticidade

Uma observação velha pode ter digest perfeitamente válido. O digest garante apenas integridade da representação pública recebida. O controle precisa injetar replay com bytes válidos e exigir comparação de sequência, época e recibo. Não se deve escrever “estado verdadeiro autenticado” quando o teste só verificou “observação pública consistente”.

### 9.5 Reset tratado como magia

A implementação atual chama `reset()` depois de cada probe e não recebe confirmação [4]. Se o fixture assume que isso restaura todos os campos, o benchmark embute a conclusão desejada. O controle é declarar a semântica de reset no manifesto, observar o baseline depois do reset e comparar o atestado com o oráculo. Um reset sem garantia explícita deve invalidar, não restaurar confiança.

### 9.6 Mistura de períodos na própria Skill

`AbstractSkill.evidence_digests` registra quais evidências foram usadas, mas não informa explicitamente sua época. Um teste pode passar se a Skill só guarda IDs e deltas. O controle é exigir que o harness expanda cada digest usado e confirme que todos pertencem a um único recibo. O campo `source_host` sozinho não é suficiente.

### 9.7 Ajuste depois do holdout

Trocar a agenda de deriva, remover o caso de reset latente ou relaxar a abstenção depois de observar resultados converte holdout em validação. Seeds, manifestos, taxonomia de motivos e limites devem ser congelados antes da primeira execução final. Qualquer ajuste cria uma nova versão do plano e invalida a comparação anterior [8] [9].

### 9.8 Maioria escondendo conflito temporal

Três probes com `x:+1` e uma probe com `x:-1` não autorizam maioria por padrão. A mudança pode ser precisamente o fenômeno medido. Sem um contrato formal de tolerância, qualquer conflito público deve bloquear a previsão. Para conflito latente, a saída correta é limitar a alegação e abster-se quando o campo é relevante.

### 9.9 Revalidação circular

Não basta “revalidar” chamando a mesma função que criou a evidência e depois comparando o resultado com seu próprio `expected_final`. A revalidação deve gerar um novo traço e o oráculo deve avaliá-lo de forma independente. Um recibo é um registro de entradas e observações, não uma declaração autoassinada de validade.

### 9.10 Métrica média escondendo uma falha dura

O contrato congela zero para falso consenso em fixtures conhecidas, violações de autoridade e ações externas não revisadas [2] [3]. Uma taxa elevada de recuperação não compensa uma única proposta construída com evidência stale. Os resultados devem mostrar falhas por fixture antes de qualquer agregado.

## 10. Critérios de aceitação

### 10.1 Aceitação do plano antes de implementação

A frente só deve avançar para código quando os seguintes itens estiverem congelados:

1. Manifestos separados de desenvolvimento, validação e holdout, com seeds inteiras `4403`, `4503` e `4603`, digests e contagens.
2. Distinção explícita entre `herus_id`, `runtime_bind_epoch`, `binding_digest`, `host_epoch`, `episode_id` e `sequence`.
3. Semântica de `reset()` declarada para cada fixture, incluindo casos em que o reset é apenas público ou avança época.
4. Esquema público que distingue observação fresca, stale, replay, epoch desconhecido e epoch incompatível.
5. Recibo de revalidação com baseline, epoch, ordem, digests, atestados de reset, orçamento e ausência de conflitos.
6. Estados de decisão separados entre `PROPOSED`, `ABSTAIN`, `UNSUPPORTED` e `FAIL_UNSAFE`; ausência de motivo deve ser marcada como `SAFE_BUT_UNPROVEN`, não como detecção.
7. Oráculo independente, sem importação do runtime, com avaliação da trajetória verdadeira e dos predicados proibidos.
8. Isolamento de processo ou proxy de serialização entre runtime, host e oráculo.
9. Pelo menos um controle estável, uma deriva depois de sondagem, uma deriva depois de reset, um rollover de epoch público, uma observação stale/replay, um host substituto com mesmo ID, uma deriva latente e uma contraprova de mistura de epochs.
10. Política congelada para campos latentes: nenhuma expectativa de detectar o que não é observável; apenas abstenção ou `UNSUPPORTED` quando isso afeta a meta.
11. Testes de mutação preparados antes da rodada: remover epoch do digest, trocar `stale` por `fresh`, reutilizar recibo velho, aceitar reset sem confirmação, restaurar `host_id` e ordenar ações por posição.

### 10.2 Aceitação da futura implementação e rodada

A implementação futura pode ser considerada aprovada somente para o escopo temporal coberto se todos os critérios abaixo forem satisfeitos:

- Nenhuma proposta ou promoção usa evidência stale, mistura epochs, aceita replay ou ignora mudança pública de vínculo.
- Nenhum reset não verificado é aceito como restauração completa.
- Todos os positivos declarados representáveis passam pelo executor independente após revalidação fresca.
- Todos os negativos temporais conhecidos terminam em abstenção segura ou `UNSUPPORTED`, conforme o contrato.
- `false_consensus_known_fixtures = 0`, `unsafe_execution = 0` e execuções além das sondas autorizadas = 0.
- A identidade `herus_id` persiste, mas o contexto, os modelos e os recibos inválidos não atravessam uma mudança de vínculo como evidência fresca.
- O resultado de três execuções independentes é idêntico byte a byte em status, motivos, digests e contagens.
- Os custos de sondagem, reset e revalidação permanecem dentro do orçamento congelado.
- A rodada publica resultados negativos, casos `UNSUPPORTED_BY_CONTRACT`, motivos de abstenção, digest do commit, digest do contrato, digest dos manifestos, seed, comando e artefatos brutos.

Uma aprovação nesse escopo autoriza apenas a afirmação estreita de que o protocolo detectou ou conteve certas mudanças temporais em hosts determinísticos e observáveis. Não autoriza afirmar que todo host real expõe sua época, que um digest autentica sensores, que deriva latente é detectável ou que a capacidade é útil para pessoas.

## 11. Sequência de execução recomendada

1. Registrar o commit `3fb3ca9d183cebe4df4d66c9cacae6c8220935c6` e os digests dos documentos, módulos e contrato analisados.
2. Congelar a taxonomia de status, motivos, semântica de reset, seeds e manifestos.
3. Construir e testar o oráculo independente sem executar o runtime; revisar manualmente os traços `T1`, `T2`, `T4`, `T6`, `T10` e `T11`.
4. Rodar os nove testes atuais e o benchmark como baseline não temporal. Preservar stdout, versão de Python, comando e resultado bruto.
5. Rodar `T0` e os casos de desenvolvimento. Confirmar que a revalidação não produz efeitos no host além das sondas autorizadas.
6. Executar validação com epoch público, replay e reset não restaurador. Congelar o holdout somente depois que o protocolo de decisão estiver estável.
7. Rodar cada holdout em processo limpo. O runtime não deve receber estado privado, classe esperada ou motivo esperado.
8. Depois que o runtime terminar, executar o oráculo sobre a proposta, a ausência de proposta, o traço de sondagem e os contadores de execução.
9. Aplicar as mutações pré-registradas. Cada mutação deve causar falha em ao menos uma contraprova e não pode ser corrigida por ajuste do rótulo posterior.
10. Repetir a rodada três vezes. Comparar bytes de manifestos, status, motivos, digests, contagens e resultados negativos.
11. Publicar a matriz por fixture e a decisão local. Se algum limite duro falhar, classificar a frente como `not_proven`, sem compensação por média.

## 12. Limites da conclusão

Este plano não implementa epoch no `HostAdapter`, não altera a política de promoção e não cria uma garantia de que um host físico forneça um marcador confiável de época. Ele especifica o que uma implementação futura deverá registrar e recusar. A ausência atual de `host_epoch`, qualidade e recibo de reset é uma limitação de contrato, não uma evidência de que o runtime seja temporalmente robusto.

O resultado permitido é restrito a hosts determinísticos, interfaces públicas definidas, agendas de deriva congeladas e um oráculo independente. Uma aprovação não prova detecção de deriva latente, autenticidade de sensores, durabilidade física, segurança de atuadores, generalidade para robôs, eficácia clínica, privacidade em produção ou qualquer benefício para usuários. Em particular, uma revalidação bem-sucedida após mudança de host demonstra apenas que o protocolo coletou evidência nova no cenário testado.

O plano também não transforma todo `None` em uma classe causal. Enquanto a API não carregar status e motivo tipados, uma recusa pode ser apenas `SAFE_BUT_UNPROVEN`. O relatório futuro deve conservar essa distinção e não usar uma taxa alta de abstenção como prova de que o runtime identificou corretamente a causa temporal.

## Referências

[1]: https://github.com/SummaArs/herus/blob/3fb3ca9d183cebe4df4d66c9cacae6c8220935c6/docs/51-API-SIMBIONTE-V2.md "API do Symbiont v2 — Etapa 1 da prova ASA"
[2]: https://github.com/SummaArs/herus/blob/3fb3ca9d183cebe4df4d66c9cacae6c8220935c6/docs/52-DEFINICAO-SIMBIOSE-UTIL.md "Definição congelada de simbiose útil — Etapa 3"
[3]: https://github.com/SummaArs/herus/blob/3fb3ca9d183cebe4df4d66c9cacae6c8220935c6/research/symbiosis_utility_contract.json "Frozen useful-symbiosis machine-readable contract"
[4]: https://github.com/SummaArs/herus/blob/3fb3ca9d183cebe4df4d66c9cacae6c8220935c6/research/symbiont_v2/core.py "HERUS Symbiont v2 research runtime"
[5]: https://github.com/SummaArs/herus/blob/3fb3ca9d183cebe4df4d66c9cacae6c8220935c6/research/symbiont_v2/sim_hosts.py "Deterministic and black-box simulator hosts"
[6]: https://github.com/SummaArs/herus/blob/3fb3ca9d183cebe4df4d66c9cacae6c8220935c6/research/symbiont_v2/test_symbiont.py "Symbiont v2 executable tests"
[7]: https://github.com/SummaArs/herus/blob/3fb3ca9d183cebe4df4d66c9cacae6c8220935c6/research/symbiont_v2/benchmark.py "Symbiont v2 reproducible benchmark"
[8]: https://github.com/SummaArs/herus/blob/3fb3ca9d183cebe4df4d66c9cacae6c8220935c6/research/evidence/stage4_plan/01-structural_holdout.md "Etapa 4 — plano de hosts holdout estruturais"
[9]: https://github.com/SummaArs/herus/blob/3fb3ca9d183cebe4df4d66c9cacae6c8220935c6/research/evidence/stage4_plan/02-partial_observability.md "Etapa 4 — plano de observabilidade parcial, contradição e abstention"

## Apêndice A — digests da base analisada

| Artefato | SHA-256 |
|---|---|
| `docs/51-API-SIMBIONTE-V2.md` | `cdf17c88a555c261bdeed068009a3970e23b61447b9b8570a5aa58cf8c2898e9` |
| `docs/52-DEFINICAO-SIMBIOSE-UTIL.md` | `bc85a7a215d658585ca52d90e6e638ab7a999e14b622f19709291852c93398ac` |
| `research/symbiosis_utility_contract.json` | `0e5ba37573a7c71fb4496b73f2c6fcd5164c3c8ceecca599806988e8ef769e20` |
| `research/symbiont_v2/core.py` | `76cf9f22e9561ffbf53c616e4734ad90f16b609b480c3fb9d177f8e182a2a2cd` |
| `research/symbiont_v2/sim_hosts.py` | `d4af8fa9e45de9877a6de925733f2c21a6e79908ba817677b4ce8e82718b3e1e` |
| `research/symbiont_v2/benchmark.py` | `804fe30d03dab14f2afd9b5c8e36d89e93bdad30566c19266b469009c4615640` |
| `research/symbiont_v2/test_symbiont.py` | `eff30e15b967cbd6124a8ff9a72bd2ea6ddecd8961cba88e4b8513ad2ddba63f` |

Nenhum código de implementação foi modificado como parte deste plano. O baseline executado no commit analisado passou em 9 testes e produziu transferência positiva no benchmark estável; nenhum desses resultados foi contado como prova de robustez temporal.
