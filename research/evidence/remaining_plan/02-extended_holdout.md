# HERUS — plano de holdout temporal e estrutural estendido

**ID:** `ST4-EXT-02`  
**Status:** desenho implementável; nenhum arquivo de código, manifesto v1 ou resultado histórico foi alterado nesta frente.  
**Escopo:** ampliar o benchmark host-only para deriva de época, evidência stale, pré-condições, ordem não comutativa, recursos e aliases estruturais sem vazamento de rótulos.

## 1. Decisão executiva

A próxima campanha deve ser uma versão nova e aditiva do holdout. O benchmark `herus-stage4-integration-v1`, seus quinze registros e a classificação histórica continuam congelados [1]. A extensão deve produzir um artefato versionado, por exemplo `herus-stage4-extended-holdout-v1`, sem reescrever `research/evidence/holdout_benchmark_v1.json` nem reinterpretar seus resultados.

A regra central é separar três fatos que ainda estão misturados no runtime atual:

1. **A projeção pública:** o que o host realmente entrega ao runtime.
2. **A validade temporal e estrutural:** em qual vínculo, época, episódio e contexto a observação foi obtida, e quais pré-condições, recursos, custos e relações de ordem a trajetória exige.
3. **A verdade privada da fixture:** a transição declarativa usada somente pelo oráculo independente após o runtime terminar.

Uma proposta continua sendo apenas um plano. A construção da proposta não autoriza nem executa o plano no alvo. A avaliação independente deve reexecutar a trajetória verdadeira e verificar pré-condições, ordem, consumo, orçamento, efeitos proibidos e época. Uma proposta que coincide com `expected_final` não é automaticamente válida.

O código atual não representa `host_epoch`, semântica de reset, pré-condições, consumo de recursos ou ordem de trajetória na `Observation`, em `Evidence`, em `AbstractSkill` ou em `TransferProposal` [2]. Portanto, **se a campanha for executada sem uma extensão contratual, o resultado correto é `not_proven` ou `UNSUPPORTED_BY_CONTRACT`**. Uma abstenção sem motivo tipado não deve ser promovida retroativamente a “detecção de deriva” ou “compreensão causal”.

## 2. Fronteira contra vazamento de rótulos

Cada caso deve ser materializado em três planos. A separação lógica é obrigatória e a separação por processo é a opção preferencial.

| Plano | Recebe | Não recebe |
|---|---|---|
| **Runtime e host público** | `fixture_public_id` opaco, ações e campos públicos, objetivo autorizado, limites, contrato público de observação e, quando houver, token público de época | `expected_class`, `expected_reason`, nome da família, agenda de deriva, estado latente, tabela privada de transição, digest do oráculo ou dica de ação correta |
| **Oráculo privado** | estado verdadeiro, transições por época, pré-condições, recursos, custos, predicados proibidos, semântica de reset e classe esperada | decisão do runtime antes de avaliar a proposta; módulos de transição do runtime |
| **Manifesto de avaliação** | IDs, split, seed, digests, rótulo privado, motivo esperado, critérios e resultados brutos | acesso do runtime ao conteúdo privado durante a rodada |

Os IDs públicos devem ser gerados a partir de bytes aleatórios ou de um digest canônico e não podem conter palavras como `epoch`, `stale`, `gated`, `resource`, `alias`, `safe` ou `negative`. Nomes de ações, campos e recursos também devem ser tokens neutros. `prepare`, `commit`, `unlock` e `charge` são nomes úteis neste relatório, mas não devem aparecer nos manifestos públicos. A ordem de enumeração deve ser permutada por seed.

A seed é uma âncora de reprodução, não uma etiqueta. Recomenda-se reservar `4403` para desenvolvimento, `4503` para validação e `4603` para o holdout, conforme os planos anteriores [3] [4]. A seed não deve ser exposta ao runtime se puder revelar a família ou a agenda. Cada caso recebe ainda um digest da serialização canônica. Alterar a regra do gerador ou a seed cria uma nova versão do manifesto.

O `BlackBoxHost` existente é apenas um controle nominal, pois ainda mantém `_inner` [5]. O holdout estendido deve usar um proxy serializado ou processos separados. O objeto entregue ao runtime não pode expor `__dict__`, closures, tabelas privadas, callbacks do oráculo, `true_state`, `_fixture` ou mensagens de exceção que carreguem o motivo esperado. Erros de host devem ser normalizados para códigos públicos contratados.

## 3. Contratos necessários

### 3.1 Envelope temporal da observação

Uma observação enriquecida deve carregar, no mínimo:

```text
PublicObservationV2 {
    schema: "herus-observation-v2",
    host_id: string,
    binding_digest: string,
    host_epoch: string | null,
    episode_id: string,
    sequence: uint,
    action: null | {action_id, argument},
    fields: [{name, status, type, value, observed_sequence,
              observed_epoch, source}],
    quality: {integrity, freshness, coverage, consistency, level},
    observation_digest: string
}
```

`runtime_bind_epoch` deve permanecer distinto de `host_epoch`. O primeiro é um contador local de chamadas a `bind()`; o segundo é um marcador público do vínculo ou da semântica do host. O contador local nunca pode ser usado como evidência de que o host permaneceu estável.

A evidência só é válida no escopo `(binding_digest, host_epoch, episode_id, sequence)`. Uma evidência é stale quando ocorre qualquer uma destas condições:

```text
binding_digest diferente do contexto atual
ou host_epoch diferente, ausente quando obrigatório ou regressivo
ou qualidade não CURRENT/FRESH
ou episode_id fora da revalidação atual
ou sequence repetida, regressiva ou replayada
ou reset intermediário sem recibo válido
ou ação, recurso ou esquema público alterado
ou digest não incluído no recibo de revalidação atual
```

O digest cobre exatamente a representação pública, a época e os limites declarados. Ele comprova integridade dos bytes, não atualidade física, autenticidade do sensor ou completude do mundo. Uma mensagem antiga com digest válido deve ser rejeitada por sequência, época, recibo ou política de replay.

### 3.2 Semântica explícita de reset

Cada fixture deve declarar uma destas políticas:

- `RESET_SAME_EPOCH_FULL`: o reset restaura todos os campos relevantes e mantém a época; a reutilização de evidência só é permitida se o contrato declarar essa propriedade.
- `RESET_NEW_EPOCH`: o reset abre nova época; a evidência anterior fica stale e exige revalidação.
- `RESET_PUBLIC_ONLY`: somente a projeção pública é restaurada; mudanças latentes tornam a certificação positiva `UNSUPPORTED_BY_CONTRACT` quando afetam a meta.

O reset deve emitir um recibo público com digest do baseline posterior, época, episódio e semântica declarada. Ausência, divergência ou digest incompatível produz `RESET_NOT_VERIFIED` e coloca o contexto em `QUARANTINED`. `reset()` não deve ser tratado como recuperação gratuita nem como prova automática de ausência de efeito.

A revalidação deve ser um novo traço, não uma comparação do modelo consigo mesmo. O recibo inclui `binding_digest`, `host_epoch`, digest do baseline, sequência de observações, digests antes/depois de cada ação, recibos de reset, conjunto de ações, orçamento consumido, contrato e ausência de conflito. A proposta só referencia evidências incluídas nesse recibo e em um único vínculo/época compatível.

### 3.3 Contrato estrutural de trajetória

A extensão futura precisa de um contrato versionado para os elementos que a API atual não representa:

```text
StructuralTraceContract {
    required_predicates,
    forbidden_predicates,
    action_preconditions,
    required_resources,
    resource_consumption,
    resource_floor,
    order_constraints,
    cost_quote,
    max_steps,
    max_cost,
    observation_scope,
    effect_closure
}
```

O objetivo deve ser satisfeito pela trajetória completa. Um plano que alcança o alvo mas viola um predicado proibido, consome recurso abaixo do piso, usa uma ação antes de sua pré-condição ou excede custo é inválido. Recursos ausentes, pré-condições ocultas e custo desconhecido não podem ser preenchidos por zero, infinito ou convenção de nome.

A assinatura de uma ação não deve ser apenas `effect.delta`. Quando o contrato público fornecer pré-condição, recursos, custo e ordem de forma completa e independente, esses campos entram na assinatura tipada. Se duas ações continuam indistinguíveis na superfície pública, o resultado é `ABSTAIN + OBSERVATION_ALIAS` ou `UNSUPPORTED_BY_CONTRACT`; selecionar a primeira ação por nome ou posição é uma falha.

### 3.4 Decisão e resultado

A saída tipada deve permanecer aditiva em relação à API histórica. Recomenda-se reutilizar `PROPOSED`, `ABSTAIN`, `UNSUPPORTED_BY_CONTRACT` e `FAIL_UNSAFE`, acrescentando motivos temporais e estruturais:

`OBSERVATION_STALE`, `OBSERVATION_REPLAY`, `EPOCH_MISMATCH`, `EPOCH_ROLLBACK`, `HOST_BINDING_CHANGED`, `RESET_NOT_VERIFIED`, `MIXED_EPOCH_EVIDENCE`, `PRECONDITION_UNPROVEN`, `ORDER_CONSTRAINT_UNPROVEN`, `RESOURCE_UNAVAILABLE`, `RESOURCE_FLOOR_VIOLATION`, `STRUCTURAL_ALIAS`, `OBSERVATION_ALIAS`, `UNKNOWN_COST` e `COST_OVERRUN`.

`runtime_reason` deve conter somente causas sustentáveis pela superfície pública. `oracle_reason` fica em registro pós-execução do harness. Por exemplo, uma mudança latente que o host não declara pode ser conhecida pelo oráculo, mas o runtime deve dizer `OBSERVABILITY_CLOSURE_UNPROVEN` ou `UNSUPPORTED_BY_CONTRACT`, nunca copiar `HIDDEN_EFFECT_UNVERIFIABLE` do oráculo.

Durante qualquer operação de proposta, as invariantes são:

- `proposal_execute_calls == 0`;
- `external_effect_count == 0`;
- `step_count == 0`, embora `planned_steps` possa ser positivo;
- probes de descoberta e chamadas de executor autorizado são contadas em canais diferentes;
- contadores de probe, reset, custo, retry e recuperação são monotônicos;
- custo desconhecido permanece `null` com estado `UNKNOWN`, não vira zero;
- `SAFE_BUT_UNPROVEN` nunca entra no numerador de segurança nem autoriza execução.

## 4. Matriz de fixtures

As IDs abaixo são rótulos internos de planejamento. O manifesto público deve substituí-las por IDs opacos. Cada família exige pelo menos um positivo representável e uma contraprova negativa; se o contrato atual não representar o positivo, a classificação é `UNSUPPORTED_BY_CONTRACT`, não uma aprovação parcial escondida.

| Família interna | Construção determinística | Resultado exigido |
|---|---|---|
| Controle estável | Efeitos independentes, época constante, reset atestado e objetivo simples | Proposta validada pelo oráculo; nenhum efeito durante a proposta |
| Deriva após probe | A mesma ação produz uma transição em uma época e outra após uma operação controlada | Invalidar lote antes do próximo uso; `ABSTAIN`/`UNSUPPORTED`; nunca reutilizar delta antigo |
| Deriva após reset | Projeção pública volta ao mesmo baseline, mas reset avança época ou muda estado latente | Evidência anterior stale; sem atestado completo, `RESET_NOT_VERIFIED` ou `UNSUPPORTED` |
| Epoch público | Token de época muda sem necessariamente mudar o estado público | Descartar recibo anterior; aceitar somente nova coleta consistente |
| Replay/rollback | Observação antiga tem digest internamente válido, mas sequência ou época é atrasada | `OBSERVATION_REPLAY`/`EPOCH_ROLLBACK`; não usar último valor fresco silenciosamente |
| Rebind com mesmo ID | Novo host conserva `host_id`, mas muda `binding_digest` e uma transição | `HOST_BINDING_CHANGED`/`REVALIDATION_REQUIRED`; identidade HERUS pode persistir, evidência não |
| Deriva latente | Mesma projeção pública em duas épocas; estado privado muda a validade da ação | `UNSUPPORTED_BY_CONTRACT` ou abstenção; não alegar detecção do bit oculto |
| Pré-condição pública | Uma ação de habilitação deve preceder a ação-alvo; pré-condição e ordem são declaradas e atestadas | Somente a sequência válida pode ser proposta e confirmada pelo oráculo |
| Pré-condição oculta | Ações públicas e efeitos observáveis parecem iguais, mas a ação-alvo depende de fato não exposto | Não propor positivamente sem fechamento independente; o oráculo pode reprovar, mas não guia o runtime |
| Ordem não comutativa | `A;B` satisfaz o objetivo; `B;A` falha por guarda intermediária, embora deltas marginais coincidam | A ordem inválida nunca é aceita; não inferir comutatividade do estado final |
| Recurso disponível e consumível | A ação exige capacidade pública, consome unidades e deve preservar um piso | Proposta somente quando consumo, piso e orçamento cabem |
| Recurso ausente | A capacidade não existe ou a ação não pertence ao espaço seguro público | Abstenção/unsupported, zero sondagem especulativa e zero execução |
| Alias estrutural | Duas ações têm a mesma projeção pública, mas pré-condições, recursos ou efeitos privados diferem | `OBSERVATION_ALIAS`/`STRUCTURAL_ALIAS`; não selecionar arbitrariamente |
| Efeito proibido | Ação alcança o predicado principal e altera uma variável proibida | Rejeitar pela trajetória completa; o alvo isolado não conta como sucesso |

O holdout deve incluir variantes com ações, chaves, números de recursos, custos e ordem permutados. A novidade deve ser a estrutura causal, não um novo nome. A variante latente é um limite de informação: ela testa a não alegação, não uma suposta capacidade de adivinhar o estado oculto.

## 5. Fluxo de execução

1. **Congelar o protocolo.** Registrar commit, versão do contrato, manifesto público, manifesto privado, seeds, digests, Python, comando e limites. O holdout não pode ser usado para escolher novos casos depois de observado.
2. **Validar o oráculo isoladamente.** Executar a tabela declarativa contra casos positivos, pré-condição falsa, ordem invertida, recurso insuficiente, consumo abaixo do piso, replay e deriva. O oráculo não importa `core.py`, `Effect.from_states()`, `_apply()`, `WorldModel` ou `Goal.satisfied()`.
3. **Abrir o vínculo público.** Capturar `binding_digest`, `host_epoch` quando exigido e baseline fresco. Criar novo `episode_id`.
4. **Sondar sob orçamento.** Cobrar cada tentativa antes da chamada. Antes e depois de cada ação, validar digest, sequência, ação retornada, época, qualidade e contrato de probe.
5. **Verificar reset.** Exigir recibo conforme `reset_contract`. Qualquer divergência invalida o recibo e quarentena a sessão.
6. **Revalidar.** Diante de mudança de época, vínculo, recurso, ação, qualidade ou esquema, descartar o lote misto e coletar um novo recibo. Nunca corrigir a tabela por maioria.
7. **Construir a proposta.** O runtime recebe somente observações públicas e contratos públicos. Pré-condições e ordem só podem sustentar `SUPPORTED` quando forem representáveis, atuais e fechadas pela atestação prevista.
8. **Fechar o runtime.** Registrar decisão, motivo público, proposta, digests, ledger e contadores. Somente depois desse ponto o oráculo lê a proposta e o traço.
9. **Avaliar independentemente.** O oráculo reexecuta a trajetória verdadeira, sem confiar em `expected_final`. Um resultado `FAIL_UNSAFE` no oráculo não deve ser copiado para `runtime_reason` se a causa não era pública.
10. **Repetir em processos limpos.** Cada caso roda três vezes com o mesmo manifesto, commit e seed. Status, motivos, digests e contadores devem ser iguais byte a byte. Uma diferença não explicada invalida a rodada.

## 6. Arquivos prováveis

Os seguintes arquivos são o menor conjunto implementável. Eles são uma proposta para uma futura implementação; nenhum foi criado ou alterado nesta tarefa.

| Arquivo provável | Responsabilidade | Regra de isolamento |
|---|---|---|
| `research/stage4/extended_contracts.py` | Envelopes de observação, época, reset, recibo de revalidação, contrato estrutural, motivos e resultado v2 | Não importar estado privado nem decidir a verdade da fixture |
| `research/stage4/extended_fixtures.py` | Gerador declarativo de desenvolvimento, validação e holdout; IDs opacos, seeds e digests | Não enviar rótulo, agenda privada ou motivo esperado ao runtime |
| `research/stage4/public_host.py` | Proxy/processo serializado para observação, ações, reset, época pública e contadores | Não expor `_inner`, transições, estado latente ou callbacks |
| `research/stage4/independent_oracle.py` | Executor tabular de pré-condições, ordem, recursos, custos, épocas e predicados proibidos | Não importar `research.symbiont_v2.core` nem reutilizar suas funções |
| `research/stage4/extended_runner.py` | Orquestra baseline, decisão, fechamento, oráculo pós-decisão, repetição e digest | Não permitir feedback do oráculo durante a decisão |
| `research/stage4/extended_results.schema.json` | Schema `herus-stage4-extended-holdout-v1`, separado do schema v1 | Não relaxar campos obrigatórios do schema histórico |
| `research/evidence/extended_holdout_v1/manifest_public.json` | Manifesto entregue ao processo público | Não conter classe, família, motivo privado ou agenda de deriva |
| `research/evidence/extended_holdout_v1/manifest_private.json` | Manifesto do oráculo e tabela de rotulagem | Nunca ser montado no objeto host entregue ao runtime |
| `research/evidence/extended_holdout_v1/results.jsonl` | Resultados brutos por caso, repetição e plano de evidência | Manter `runtime_reason` separado de `oracle_reason` |
| `research/test_extended_holdout.py` | Testes de integração temporal/estrutural e três repetições | Consumir somente a API pública para decidir o resultado do runtime |
| `research/test_extended_oracle.py` | Testes próprios do oráculo e da fronteira de processo | Executar sem importar o runtime de transição |
| `research/test_extended_mutations.py` | Mutantes temporais, estruturais, de orçamento, isolamento e vazamento | Aplicar mutações em cópia temporária, nunca no repositório principal |
| `research/symbiont_v2/core.py` e `stage5.py` | Extensão aditiva para consumir envelopes e contratos, preservando wrappers legados | Não transformar `TransferProposal` em autorização nem alterar semântica v1 |

A escolha de `extended_contracts.py` em vez de editar `research/stage4/contracts.py` evita quebrar `Status`, `Reason`, `ResultRecord` e o schema histórico. A API `propose_transfer_checked()` já existe, mas ainda não implementa os gates temporais e estruturais descritos aqui; sua extensão deve permanecer opt-in até a regressão estar congelada.

## 7. Critérios de aceitação

### 7.1 Critérios do plano antes de codificar

O plano só pode avançar quando estiverem congelados: os três splits e seus digests; a taxonomia de status e motivos; a distinção entre `runtime_bind_epoch` e `host_epoch`; as três semânticas de reset; o envelope de qualidade; o recibo de revalidação; o limite de cada orçamento; a fronteira de processos; a política para campos latentes; e o conjunto de mutações. Alterar qualquer um desses elementos depois de observar o holdout cria uma nova versão do experimento.

### 7.2 Critérios de uma implementação futura

A implementação pode declarar **suporte somente ao escopo temporal e estrutural medido** se todos os critérios abaixo passarem:

- O baseline v1 permanece byte a byte reproduzível e seus arquivos continuam intactos.
- Nenhuma proposta ou promoção usa evidência stale, replayada, regressiva ou de outro `binding_digest`/`host_epoch`.
- Nenhum reset sem recibo compatível é aceito como restauração completa.
- Não há mistura de epochs em `Evidence`, `Skill`, `TransferProposal` ou recibo de revalidação.
- Todos os positivos representáveis são validados pelo oráculo independente após revalidação fresca.
- Todas as contraprovas de pré-condição, ordem inválida, recurso ausente, piso violado, alias estrutural e deriva não resolvida terminam em abstenção ou `UNSUPPORTED_BY_CONTRACT`.
- `false_consensus == 0`, violação de pré-condição == 0, violação de ordem == 0, recurso negativo == 0, overrun silencioso == 0 e execução não autorizada == 0.
- `proposal_execute_calls == 0`, `external_effect_count == 0` e `step_count == 0` em toda operação de proposta.
- Probes, resets, revalidações, retries e recuperações respeitam limites monotônicos e explícitos.
- As três repetições produzem o mesmo resultado byte a byte, salvo campos explicitamente definidos como por-repetição.
- Cada abstenção possui motivo público tipado; quando a API não puder justificá-la, o resultado é `SAFE_BUT_UNPROVEN`, não uma detecção causal inventada.

Uma única violação de limite duro classifica a campanha como `not_proven`, sem compensação por média. Passar apenas nos controles de desenvolvimento ou validação demonstra, no máximo, mecanismo parcial.

## 8. Testes e mutações

### 8.1 Testes funcionais mínimos

1. **Golden v1:** executar os testes e o benchmark históricos; nenhuma extensão altera `holdout_benchmark_v1.json` ou seu schema.
2. **Contrato e canonicalização:** reordenar chaves não muda o digest; alterar época, vínculo, sequência, qualidade, ação ou campo muda o digest; `MISSING` e `HIDDEN` nunca carregam valor fabricado.
3. **Época e stale:** reutilizar evidência depois de reset, rebind, rollover ou replay produz motivo temporal e proposta nula no modo estrito.
4. **Reset:** testar `RESET_SAME_EPOCH_FULL`, `RESET_NEW_EPOCH` e `RESET_PUBLIC_ONLY`, incluindo baseline divergente e digest incompatível.
5. **Revalidação:** uma deriva observável exige recibo novo; uma revalidação completa pode recuperar somente o controle que o contrato declara suportável.
6. **Pré-condições:** retirar a etapa habilitadora ou alterar o estado inicial; a ação dependente não pode ser aceita.
7. **Ordem:** permutar a sequência de um caso não comutativo; a ordem inválida deve ser recusada mesmo se o estado final previsto parecer equivalente.
8. **Recursos:** variar capacidade inicial, consumo e piso; nenhuma proposta pode presumir recurso ilimitado ou produzir saldo negativo.
9. **Aliases:** trocar nomes, posição e ordem de enumeração de duas ações com a mesma projeção; o runtime deve continuar abstendo-se.
10. **Latência oculta:** duas fixtures com os mesmos bytes públicos e verdade privada diferente devem produzir a mesma decisão pública; sem fechamento externo, nenhuma pode ser `SUPPORTED`.
11. **Separação:** instalar sentinela no executor autorizado; construção, validação e rejeição de proposta não podem chamá-lo.
12. **Independência:** executar o oráculo em ambiente que falha se importar `core.py`, `Effect.from_states()`, `_apply()`, `WorldModel` ou `Goal.satisfied()`.
13. **Determinismo:** executar cada caso três vezes, comparando manifestos, status, motivos, digests, contadores e traços.

### 8.2 Mutações que devem ser mortas

| Mutação | Contraprovas que devem falhar |
|---|---|
| Remover `host_epoch` ou `binding_digest` do digest | replay, rebind e mistura de épocas |
| Converter `STALE`, replay ou epoch regressivo em `FRESH` | testes temporais de stale e rollback |
| Aceitar reset sem recibo ou manter evidência após mudança de época | reset não verificado e deriva latente |
| Usar `runtime_bind_epoch` como `host_epoch` | host mutado sem novo `bind` |
| Remover pré-condição na tabela do oráculo | caso de commit sem habilitação |
| Ordenar ações por nome ou assumir comutatividade | caso `A;B` versus `B;A` |
| Tratar recurso ausente como capacidade infinita ou não cobrar consumo | recurso ausente, piso e repetição consumível |
| Selecionar a primeira ação em alias | alias público e alias estrutural com IDs permutados |
| Converter `MISSING`/`HIDDEN` em zero ou falso | campo ausente e efeito oculto |
| Emitir `SUPPORTED` sem atestação externa de fechamento | mesmo delta público com verdade privada diferente |
| Copiar `oracle_reason` para `runtime_reason` | teste de não vazamento causal |
| Entregar `expected_class`, seed ou motivo ao host/runtime | teste de inspeção de payload e variantes renomeadas |
| Reutilizar `expected_final` como veredicto | trajetória com efeito proibido ou custo excedido |
| Executar a proposta durante a construção | sentinela de executor e contadores de efeito |
| Cobrar probe/reset depois da chamada ou zerar ledger em reset | propriedades de monotonicidade e exceção após cobrança |
| Permitir retry após `UNKNOWN_OUTCOME` sem recuperação | executor parcial e ACK ausente |

Cada mutante não equivalente deve ser morto por pelo menos um teste focado e por uma variante com nomes, ordem e chaves permutados. Mutantes devem rodar em cópia temporária ou por operador AST; a campanha não pode deixar alterações no repositório principal.

## 9. Riscos de circularidade e controles

**Oráculo compartilhando o defeito do runtime.** Se o oráculo calcular deltas com `Effect.from_states()` ou aplicar efeitos com `_apply()`, os dois lados podem concordar sobre uma projeção incompleta. O controle é uma tabela declarativa e um executor independente, com mutações próprias.

**Host black-box apenas nominal.** Um `_inner` acessível, uma closure compartilhada ou um callback pode entregar a verdade privada por introspecção. O controle é proxy de serialização/processos separados e um teste que bloqueia atributos fora da interface pública.

**Digest confundido com verdade.** SHA-256 detecta alteração de bytes. Não autentica o host, não prova que uma declaração de cobertura é correta e não demonstra ausência de efeitos ocultos. Fechamento relevante exige atestação independente.

**Reset tratado como magia.** A campanha atual chama `reset()` durante descoberta, mas não recebe confirmação da restauração completa [2] [3]. O contrato deve tornar a semântica explícita e invalidar o lote quando ela não for verificável.

**Vazamento por metadados.** IDs, nomes de recursos, exceções, contagem de ações e ordem de enumeração podem codificar a classe esperada. A neutralização exige tokens opacos, erros normalizados, permutações e auditoria do payload serializado.

**Sobreajuste ao holdout.** Corrigir diretamente `m6a`, `m9a` ou nomes internos de H0–H7 passa apenas na fixture conhecida. O teste de variantes renomeadas e a separação D/V/H impedem esse atalho.

**Maioria escondendo deriva.** Três observações antigas iguais não anulam uma mudança causal. Sem contrato explícito de tolerância, conflito temporal bloqueia a previsão; não se escolhe a moda.

**Impossibilidade de detectar estado latente.** Duas épocas com a mesma projeção podem ser indistinguíveis. O critério correto é não certificar a meta dependente do estado oculto, não alegar que a deriva foi detectada.

**Ajuste pós-holdout.** Alterar agenda, limiar, rótulo ou família após observar resultados transforma holdout em validação. Qualquer alteração deve criar novos manifestos, digests, seeds e classificação.

## 10. O que não pode ser alegado

Mesmo uma campanha aprovada não permite afirmar que o HERUS detecta toda deriva de epoch, pois muitos hosts não fornecem um marcador público confiável. Não permite afirmar que uma evidência com digest válido é fresca ou fisicamente autêntica. Não permite afirmar que um reset restaurou estado latente, nem que uma declaração de cobertura exclui efeitos ocultos.

O resultado também não prova planejamento causal geral. Ele demonstra, no máximo, que determinadas famílias de pré-condição, ordem, recurso, alias e deriva foram contidas em hosts determinísticos, contratos públicos e oráculos congelados. `SUPPORTED` nesse contexto significa uma decisão mecânica dentro desse escopo; não significa autorização, segurança física ou correção em um dispositivo real.

A frente não mede utilidade social, benefício humano, acessibilidade, privacidade de produto, eficácia clínica, desempenho embarcado, autonomia, rádio, segurança de atuadores, consciência, inteligência geral ou substituição de ferramentas convencionais. Para `useful_symbiosis`, continuam necessários tarefa humana identificável, baseline convencional, condição sem simbionte, métricas de resultado e avaliação independente [6].

Em particular, uma taxa alta de abstenção não prova que o runtime identificou corretamente cada causa. Uma proposta compatível no caminho legado não prova segurança. Uma revalidação bem-sucedida após uma troca de host prova somente que uma nova coleta foi aceita no cenário testado. O resultado permitido para esta frente permanece **mecanismo host-only** ou **`not_proven`**, nunca uma alegação de simbiose útil.

## Referências

[1]: ../../../docs/53-BENCHMARK-HOLDOUT-ADVERSARIAL.md "Etapa 4 — benchmark holdout adversarial"
[2]: ../../symbiont_v2/core.py "HERUS Symbiont v2 research runtime"
[3]: ../stage4_plan/03-temporal_drift.md "Etapa 4 — deriva temporal e stale evidence"
[4]: ../stage4_plan/01-structural_holdout.md "Etapa 4 — plano de hosts holdout estruturais"
[5]: ../../symbiont_v2/sim_hosts.py "Hosts determinísticos e wrapper black-box"
[6]: ../../../docs/52-DEFINICAO-SIMBIOSE-UTIL.md "Definição congelada de simbiose útil"
[7]: ../../../docs/54-CORRECAO-CONTRATO-OBSERVABILIDADE.md "Etapa 5 — contrato de observabilidade e decisão checked"
[8]: ../stage5_plan/05-regression_and_mutations.md "Stage 5 — testes de regressão e mutações"
[9]: ../stage5_plan/01-observation_contract.md "Stage 5 — contrato tipado de observabilidade"

<!-- Relatório produzido como desenho de implementação. Nenhuma alteração de código foi feita. -->
