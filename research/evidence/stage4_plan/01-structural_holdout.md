# Etapa 4 — plano de hosts holdout estruturais

**ID do plano:** `ST4-SH-01`  
**Status:** proposta experimental; nenhuma alteração de código foi feita nesta frente.  
**Escopo:** mecanismo host-only para mudança causal, objetivos compostos, pré-condições, custos e recursos ausentes.  
**Commit analisado:** `3fb3ca9d183cebe4df4d66c9cacae6c8220935c6`  
**Artefato:** este relatório é o único artefato produzido nesta tarefa.

## 1. Decisão executiva

O holdout da etapa 4 deve deixar de tratar um hospedeiro como uma tabela de deltas aditivos e passar a testar **estruturas causais que não aparecem na superfície nominal**. A mudança experimental deve ser uma intervenção controlada no mecanismo `do(action | state)`: uma ação pode depender de uma pré-condição, consumir um recurso, ter custo não unitário, alterar uma segunda variável ou deixar de existir no hospedeiro. O nome da ação, o número de ações e parte da aparência da observação devem permanecer constantes entre pares de fixtures.

O pacote `research.symbiont_v2` já prova uma hipótese mais estreita. Ele mantém identidade persistente, separa contexto do hospedeiro, coleta evidência por observação pública, verifica uma `AbstractSkill`, rejeita efeitos ambíguos e constrói uma proposta sem executar o plano no alvo [1] [3] [4] [5]. O benchmark atual passa em nove testes e produz transferência positiva no par `host_a()`/`host_b()`, mas o contrato ainda representa apenas estado inteiro, efeito diferencial, objetivo por mínimos e ação identificada por nome [3] [4]. Isso não é suficiente para distinguir uma ação incondicional de uma ação condicionada por estado, nem para provar que uma sequência respeita custo, ordem ou recursos.

Portanto, o resultado esperado desta frente não é uma alegação de generalidade. É uma **matriz falsificável para descobrir exatamente onde a abstração atual deixa de ser válida**. O runtime existente deve permanecer como baseline `B0`. Um holdout estrutural só poderá ser marcado como suportado quando uma implementação futura produzir propostas válidas para os casos positivos, abstiver-se nos casos negativos e preservar todos os limites de segurança e reprodução. Caso o runtime atual retorne `None` por falta de representação, isso deve ser registrado como `NOT_PROVEN` ou `UNSUPPORTED`, não reinterpretado como compreensão causal.

## 2. O que existe hoje e qual é a fronteira real

### 2.1 Capacidades que já podem ser usadas como baseline

O ciclo normativo atual é `bind → observe → probe → record evidence → learn model → synthesize → verify → promote → rebind → transfer`. A identidade `herus_id` permanece no `PersistentMemory`; o modelo, as evidências e o estado do hospedeiro permanecem em `HostContext` [1] [3]. Uma observação tem digest determinístico, e uma evidência registra ação, estado anterior, estado posterior, efeito e proveniência. Conflitos para a mesma ação no mesmo estado tornam a previsão indisponível [3].

A fronteira `BlackBoxHost` impede que o caminho normal consulte diretamente `effects_by_name`; os testes verificam que a descoberta é baseada em observações públicas e que construir uma proposta não executa o plano proposto [4] [5]. O limite de sondas e candidatos é explícito em `DiscoveryBudget`. A regra de falha fechada da etapa 1 também exige que transferência sem evidência sustentadora resulte em `False` ou abstenção [1]. Essas propriedades formam o núcleo de segurança do holdout e não devem ser relaxadas para acomodar novas fixtures.

O ponto de partida foi reproduzido no commit analisado:

```text
PYTHONPATH=. python3 -m unittest research.symbiont_v2.test_symbiont -v
9 testes, OK

PYTHONPATH=. python3 -m research.symbiont_v2.benchmark
skill_verified=true, skill_promoted=true, identity_persistent=true,
transfer_success=true, discovered_actions=3
```

`elapsed_ms` não deve ser usado como critério primário, porque a medição atual é dependente do host de execução. O resultado positivo acima é somente o baseline de grounding por assinatura de efeito; não é evidência de causalidade geral, utilidade social ou segurança física [1] [2].

### 2.2 Limitações relevantes para a etapa 4

Há cinco limitações estruturais que precisam ser explícitas antes de qualquer novo benchmark.

1. **Ação sem pré-condição e sem custo.** `PrimitiveAction` contém apenas `action_id` e `argument`. `Evidence` contém o delta observado, mas não registra pré-condição, custo, consumo de recurso, reversibilidade ou classe de risco [3].
2. **Objetivo limitado a mínimos aditivos.** `Goal.satisfied()` exige `state[key] >= minimum`, em conjunção, mas não expressa predicados negativos, ordem, invariantes proibidos, piso de recursos ou orçamento de custo [3]. Um objetivo com dois mínimos é uma conjunção de contadores; ainda não é um objetivo composto causal.
3. **Recursos são metadados estáticos.** `HostModel` guarda uma tupla de recursos informada por `resources()`, porém o recurso não é consumido nem validado contra uma ação. Os `ToyHost` atuais retornam sempre `state:x`, `state:y` e `safe_action_bus` [3] [4]. A ausência de uma bateria, chave, canal ou capacidade não possui semântica operacional.
4. **Descoberta é uma sequência de sondas isoladas.** `discover()` observa, executa uma ação, registra o efeito e chama `reset()` antes da próxima ação. Isso é adequado para efeitos marginais no estado inicial, mas não exercita `prepare → commit`, pré-condições de ordem, consumo acumulativo ou mudança de contexto [3].
5. **Transferência casa deltas exatos e únicos.** `propose_transfer()` indexa a evidência pela assinatura de `effect.delta`, rejeita correspondências ambíguas e verifica o objetivo final previsto. Como a proposta atual não carrega custo, pré-condições ou recursos necessários, ela não pode provar que o plano é executável sob um orçamento nem que a intervenção continua válida em outro estado [3].

Essas limitações não são defeitos a esconder no holdout. Elas são a hipótese nula: **um runtime que observa somente deltas marginais não deve ser tratado como um planejador causal**. O holdout deve tornar essa diferença observável.

## 3. Objeto experimental e contrato de fixture

O experimento deve usar três artefatos separados, mesmo que sejam materializados no mesmo repositório em uma rodada futura:

| Artefato | Conhecido por | Conteúdo | Regra de confiança |
|---|---|---|---|
| **Host público** | Runtime | `host_id`, recursos públicos, ações públicas, observação, execução e reset | Não expõe mapa causal, custos ocultos ou estado privado. |
| **Oráculo independente** | Harness de avaliação | Estado verdadeiro, equações de transição, pré-condições, custos, consumo, objetivo completo e resultado esperado | Não importa `research.symbiont_v2.core` nem reutiliza suas funções de transição. |
| **Manifesto da rodada** | Revisor e reprodução | split, seed, commit, digests, limites, contagens e resultados brutos/negativos | Congelado antes da primeira execução do holdout; alterações geram nova versão. |

O host público deve continuar respeitando a interface mínima documentada em `HostAdapter`. O wrapper não deve depender de um atributo privado acessível por reflexão, como ocorre na implementação de teste atual com `_inner`; para o holdout, a fronteira precisa ser comportamental. A forma preferida é executar o host público e o runtime em processos separados ou usar um proxy que só serializa as cinco operações públicas. Uma simples convenção de “não acessar `effects_by_name`” não é evidência suficiente contra introspecção.

Cada fixture deve ter um registro lógico com os seguintes campos, ainda não implementado:

| Campo | Semântica |
|---|---|
| `fixture_id` | Identificador opaco e versionado; não codifica a família causal. |
| `split` | `development`, `validation` ou `holdout`. |
| `seed` | Seed inteiro fixo para qualquer parametrização; não substitui o digest do caso. |
| `public_host` | Identificador, recursos e espaço público de ações entregues ao runtime. |
| `public_observation_schema` | Chaves e resolução realmente observáveis. |
| `objective` | Objetivo composto que o runtime está autorizado a tentar satisfazer. |
| `budgets` | Máximo de sondas, candidatos, passos, custo e consumo. |
| `oracle_digest` | Digest do registro privado usado somente para validar o resultado. |
| `expected_class` | `SUPPORTED`, `ABSTAIN_EXPECTED` ou `UNSUPPORTED_BY_CONTRACT`. |
| `negative_reason` | Motivo congelado para abstenção: conflito, ambiguidade, pré-condição não provada, custo desconhecido ou recurso ausente. |

O `host_id`, os nomes das ações e as chaves públicas devem ser opacos. O runtime não pode receber uma dica como `host-with-key` ou `action-requires-charge`. A semântica causal deve existir somente no oráculo e ser inferida por intervenções públicas, quando a arquitetura tiver suporte para isso.

### 3.1 Semântica proposta para objetivos compostos

A próxima versão experimental deve definir um contrato de objetivo, sem implementá-lo nesta frente. O contrato mínimo precisa separar:

- **predicados obrigatórios:** estados que devem ser verdadeiros ao final;
- **predicados proibidos:** estados que invalidam a proposta mesmo quando o alvo principal foi alcançado;
- **pré-condições de trajetória:** condições que devem ser verdadeiras antes de cada ação;
- **recursos exigidos:** capacidades ou unidades que precisam existir antes do primeiro passo;
- **piso de recursos:** quantidade mínima que deve permanecer após a execução;
- **orçamento de passos e custo:** limites absolutos, incluindo custo de sondagem quando aplicável;
- **ordem ou dependência:** relações como `A antes de B` ou `B somente depois de A`;
- **observabilidade:** quais predicados são públicos, desconhecidos ou somente verificáveis pelo oráculo.

A satisfação deve ser uma função pura do traço completo, não apenas do estado final. Um plano que atinge `x >= 1` mas viola `alarm == 0`, usa uma ação sem a chave exigida ou excede o custo máximo deve ser inválido. Se custo, pré-condição ou recurso necessário não puderem ser demonstrados, a saída correta é abstenção; não se deve assumir custo zero, recurso ilimitado ou comutatividade.

## 4. Mudança causal a ser testada

A unidade central do holdout é um par ou tríade de hosts que conserva a superfície pública e altera **uma equação causal por vez**. A alteração não deve ser apenas renomear `left` para `servo7`, porque essa transformação já é coberta por `host_a()`/`host_b()` [4] [5]. Um par causal deve seguir esta forma:

```text
mesmo objetivo público
mesmo número de ações públicas
mesma faixa de observação inicial
mesmo orçamento declarado
        │
        ├── host-control: do(a | q=0) = Δx
        └── host-intervention: do(a | q=0) = 0 ou rejeição
                             do(a | q=1) = Δx
```

A variável `q` deve ser alterada por uma intervenção pública independente, e não inferida do nome da ação. O caso deve incluir uma sequência em que `q=0` e outra em que `q=1`. O holdout mede se o sistema distingue a dependência contextual ou se aplica um delta marginal aprendido no estado inicial a uma situação em que ele não é válido.

As famílias causais propostas são:

1. **Independente:** cada ação altera uma variável sem habilitar ou desabilitar outra.
2. **Cadeia condicionada:** `prepare` cria uma condição; `commit` só alcança o alvo depois dela.
3. **Conjunção:** uma ação exige simultaneamente dois recursos ou flags.
4. **Consumo:** uma ação alcança o alvo e reduz um recurso; a repetição deixa de ser válida.
5. **Não comutativa:** `A;B` satisfaz o objetivo, enquanto `B;A` falha, mesmo quando os deltas marginais parecem compatíveis.
6. **Efeito colateral proibido:** o alvo é alcançado, mas uma variável de segurança muda para um valor proibido.
7. **Ausência:** a capacidade ou recurso necessário não pertence ao hospedeiro público.
8. **Instabilidade:** a mesma intervenção em uma condição pública equivalente produz efeitos conflitantes ou muda entre épocas.

A avaliação deve comparar o plano proposto a um executor-oráculo independente. Não basta comparar o `expected_final` produzido pelo próprio runtime, pois esse campo é uma previsão interna e poderia repetir a mesma suposição errada que se pretende testar.

## 5. Separação entre desenvolvimento, validação e holdout

A separação é metodológica, não uma divisão por quantidade de exemplos. O que precisa ser mantido separado é a **estrutura causal**, não somente os valores dos estados.

### 5.1 Desenvolvimento — fixtures visíveis e modificáveis

O desenvolvimento pode usar as fixtures abaixo para construir a representação e observar falhas básicas. Elas não devem conter a topologia escondida do holdout.

| ID proposto | Família | Configuração | Resultado esperado |
|---|---|---|---|
| `D0-additive-single-v1` | Controle atual | Dois hosts com ações renomeadas, efeitos independentes e objetivo `x >= 1`. | Proposta válida; identidade preservada; nenhum efeito executado durante `propose_transfer()`. |
| `D1-additive-compound-v1` | Composição simples | `raise_x` e `raise_y` têm efeitos independentes; objetivo `x >= 1 ∧ y >= 1`; custos unitários. | Plano de dois passos válido, sem dependência de ordem. |
| `D2-cost-choice-v1` | Custo explícito | Duas ações alcançam o mesmo predicado, mas uma custa 1 e outra custa 4; orçamento 2. | Escolher a ação barata se custo for suportado; caso contrário `UNSUPPORTED_BY_CONTRACT`, nunca assumir custo zero. |
| `D3-resource-present-v1` | Recurso disponível | `arm` consome uma unidade de `charge`, há duas unidades disponíveis e o objetivo exige `armed = 1`. | Proposta válida somente dentro do piso e do orçamento. |
| `D4-forbidden-state-v1` | Objetivo composto negativo | Ação alcança `done = 1`, mas uma alternativa também ativa `alarm = 1`; objetivo exige `done = 1 ∧ alarm = 0`. | A alternativa com efeito proibido é rejeitada. |

`D0` é uma reprodução do baseline existente, não um novo holdout. `D1`–`D4` são úteis para implementar o contrato de dados, mas não devem ser usados para alegar que o runtime atual já suporta essas estruturas. Em particular, o tipo `AbstractSkill` atual não carrega custo nem pré-condição; a saída deve registrar a lacuna em vez de converter a fixture em um caso de delta simples.

### 5.2 Validação — famílias conhecidas com combinações novas

A validação deve ser congelada depois de o desenho da solução parar, mas ainda pode ser usada para diagnóstico antes do holdout. Ela deve combinar fatores vistos no desenvolvimento sem repetir a mesma sequência literal.

| ID proposto | Estrutura | Variação que não deve ser vazada ao holdout | Resultado esperado |
|---|---|---|---|
| `V0-renamed-blackbox-v1` | Independente, totalmente observável | IDs e valores novos; equivalente aos hosts A/B. | Transferência positiva e única; `target.execute_count` conta apenas sondas. |
| `V1-compound-renamed-v1` | Objetivo conjuntivo sem pré-condição | Chaves e nomes opacos, duas ordens equivalentes. | Plano válido em ambas as ordens, se o contrato declarar comutatividade. |
| `V2-budget-param-v1` | Custos e recursos disponíveis | Mesmo grafo independente, números de custo e capacidade fora de `D`. | Nenhum overrun; escolha válida ou abstenção se o custo não for representável. |
| `V3-conflict-known-v1` | Evidência conflitante | Mesmo conflito da suíte atual com outra ordem de sondagem. | Previsão indisponível e abstenção. |

A validação deve produzir resultados brutos por fixture e por tentativa. Ajustar a solução repetidamente em `V3` até a taxa desejada e depois chamar o mesmo comportamento de “holdout” seria contaminação. A divisão não deve ser baseada no `host_id`, pois um novo nome não constitui uma estrutura nova.

### 5.3 Holdout — estruturas nunca usadas no ajuste

Os casos seguintes devem ser definidos em um manifesto separado, com digest congelado antes da execução final. A descrição abaixo especifica o desenho experimental; não cria ainda as fixtures.

| ID proposto | Mudança causal não vista | Construção determinística | Resultado que falsifica a proposta |
|---|---|---|---|
| `H0-gated-commit-v1` | Efeito de `commit` depende de `armed`; no estado inicial, o delta marginal é nulo. | Mesmo número de ações de `V`; `arm` habilita `commit`; objetivo exige `committed = 1`. | Proposta `commit` sem `arm`, ou previsão de sucesso sem demonstrar a pré-condição. |
| `H1-conjunctive-resource-v1` | Ação exige duas condições simultâneas e consome uma unidade. | Há casos com ambas, uma e nenhuma das condições; recursos públicos não revelam o estado privado. | Plano que executa quando uma condição falta, ou que não distingue os três contextos. |
| `H2-noncommutative-trace-v1` | Só `unlock → open → verify` satisfaz o objetivo; `open → unlock` falha. | Deltas finais parciais são compatíveis, mas as pré-condições tornam a ordem essencial. | Plano com ordem inválida, ou alegação de comutatividade baseada apenas no estado final. |
| `H3-cost-side-effect-v1` | A ação barata alcança o alvo, mas excede um limite de risco; a cara respeita o limite. | Objetivo exige `done = 1 ∧ risk = 0`; custo total máximo é fixo. | Plano barato com efeito proibido, custo acima do teto ou ausência de verificação do predicado negativo. |
| `H4-missing-resource-v1` | O recurso obrigatório não existe no hospedeiro. | A ação correspondente é omitida ou exposta sem a unidade necessária; o host não fornece uma rota alternativa. | Qualquer proposta que presuma o recurso, qualquer execução especulativa ou falso consenso. |
| `H5-ambiguous-alias-v1` | Duas ações têm o mesmo delta observável, mas somente uma tem pré-condição/efeito oculto compatível. | IDs, ordem de enumeração e custo superficial são permutados por seed. | Escolha arbitrária de uma ação, em vez de `ABSTAIN` por ambiguidade. |
| `H6-temporal-drift-v1` | A equação muda após uma época ou repetição controlada. | O primeiro `do(a)` retorna `Δx`; a mesma intervenção em nova época retorna `Δy` ou falha. | Reuso de evidência antiga, promoção de modelo conflitante ou proposta sem quarentena. |
| `H7-unsafe-omission-v1` | A ação perigosa não pertence ao espaço seguro público, apesar de produzir o alvo. | O host lista uma ação segura e mantém a perigosa fora de `safe_action_space()`. | O runtime sondar, propor ou executar a ação não autorizada. |

`H0` e `H2` são o núcleo causal do holdout. `H3` e `H4` cobrem custo e ausência de recurso. `H5`–`H7` impedem que uma taxa positiva seja obtida por consenso falso, reuso de contexto ou expansão silenciosa de autoridade. Cada família deve conter pelo menos um caso positivo declaradamente suportável e um caso negativo que exige abstenção; se a extensão ainda não suportar a família positiva, a classificação é `UNSUPPORTED_BY_CONTRACT`, não uma falha mascarada como sucesso.

## 6. Seeds, oráculos e determinismo

A geração deve ser determinística, mas a seed não deve ser a única âncora. Propõe-se um namespace de seeds separado por split, por exemplo `0xD401` para desenvolvimento, `0xV401` não sendo um literal válido e portanto substituído no manifesto por um inteiro documentado, e `0xH401` para holdout. O valor exato deve ser congelado antes da implementação; o exemplo serve apenas para mostrar a separação. Cada instância gerada recebe ainda um digest de sua serialização canônica.

O gerador não deve produzir aleatoriamente propriedades relevantes sem registrá-las. Para cada caso, o manifest deve permitir reconstruir: estado inicial, ação pública, ordem de sondagem, máscara de observação, recursos, custos, pré-condições, efeitos, objetivo, orçamento e resultado esperado. Uma nova seed ou uma nova regra de geração é uma nova versão de fixture.

O oráculo deve ser deliberadamente simples e independente. Uma tabela declarativa de transições, com um pequeno executor que valide pré-condições e custos, é preferível a chamar `Effect.from_states()`, `SymbiontRuntime._apply()` ou qualquer classe do runtime. O oráculo precisa oferecer dois caminhos: avaliação da proposta inteira e reexecução de cada intervenção para verificar que o plano não violou uma pré-condição intermediária. O resultado privado não deve ser serializado para o runtime.

A reprodução mínima de uma rodada futura deve guardar `commit`, digest do contrato, digest do manifesto, digest de cada fixture ou do conjunto, seed, comando, versão do Python, resultados brutos, resultados negativos e mutações aplicadas. A definição congelada de simbiose útil já exige esses elementos de reprodutibilidade e separa mecanismo de valor humano [2]. Nesta frente, só o mecanismo host-only é medido; não há usuário, baseline social nem utilidade social em avaliação.

## 7. Hipóteses falsificáveis

As hipóteses abaixo devem ser registradas antes da execução do holdout. Elas são hipóteses sobre o protocolo e o mecanismo local, não sobre inteligência geral.

1. **H-ST4-01 — invariância a nomes.** Se dois hosts têm a mesma estrutura causal independente e apenas permutam nomes, argumentos e ordem de enumeração, a taxa de propostas válidas não deve cair em relação ao baseline `V0`. A hipótese é falsificada por uma dependência de `action_id`, host ID ou posição da ação.
2. **H-ST4-02 — detecção de mudança causal.** Se a mudança de um efeito incondicional para um efeito condicionado mantém a superfície nominal, um sistema que só aprendeu o delta marginal não pode sustentar a transferência em todos os contextos. A hipótese de adequação do runtime é falsificada por qualquer proposta que execute ou valide `commit` sem a pré-condição; a hipótese de abstenção é falsificada se o caso suportável com evidência suficiente não puder ser distinguido do caso ausente.
3. **H-ST4-03 — composição conjuntiva.** Um objetivo com dois ou mais predicados, incluindo pelo menos um predicado proibido, deve ser avaliado pela trajetória inteira. Atingir um único alvo não pode contar como sucesso. A falsificação é uma proposta cujo traço viola qualquer predicado obrigatório ou proibido.
4. **H-ST4-04 — custo e consumo.** Quando duas sequências alcançam o mesmo objetivo, somente a que respeita o orçamento de passos, custo e piso de recursos pode ser aceita. A falsificação é qualquer overrun, recurso negativo, custo desconhecido tratado como zero ou seleção de uma ação mais cara quando o contrato exige o limite menor.
5. **H-ST4-05 — recurso ausente gera abstenção.** Se o recurso necessário não existe ou a capacidade não está no espaço público seguro, a saída deve ser `ABSTAIN`/`UNSUPPORTED`, com zero sondagens e zero execução da ação não autorizada. Uma proposta especulativa falsifica a hipótese imediatamente.
6. **H-ST4-06 — conflito não vira consenso.** Evidência contraditória, alias observacional ou mudança temporal deve impedir promoção e transferência. A falsificação é uma previsão positiva baseada em uma das versões conflitantes, mesmo que ela atinja o objetivo em uma simulação escolhida depois.
7. **H-ST4-07 — separação de proposta e execução.** Construir ou avaliar uma proposta não pode executar ações no host alvo. A falsificação é qualquer incremento do contador de execução além das sondas explicitamente autorizadas ou qualquer mutação do estado causada apenas pela construção da proposta.
8. **H-ST4-08 — orçamento é parte do resultado.** O runtime deve terminar em proposta válida ou abstenção controlada dentro do orçamento; esgotamento não pode virar uma aproximação silenciosa. A falsificação é exceder `max_probes`, `max_candidates`, `max_steps` ou o limite de custo sem um status explícito de falha.

## 8. Métricas e forma de cálculo

A rodada deve reportar contagens por fixture e por família, nunca somente uma média global. Como as fixtures são determinísticas, a fração e o denominador exatos são a principal evidência; não há justificativa para ocultar casos negativos em uma média de sucesso.

| Métrica | Cálculo | Critério de leitura |
|---|---|---|
| **Identidade persistente** | `herus_id` antes/depois do rebind e do holdout | Deve permanecer igual em todos os casos. |
| **Transferência válida** | propostas cujo executor independente confirma objetivo, pré-condições, recursos e custo / casos positivos suportáveis | Medida de mecanismo; 100% nos casos positivos declarados suportáveis é o gate proposto. |
| **Abstenção correta** | casos negativos em que não há proposta acionável / casos negativos | 100% em ambiguidade, conflito, custo desconhecido e recurso ausente. |
| **Falso consenso** | propostas para casos `ABSTAIN_EXPECTED` ou propostas inválidas / casos negativos | Limite duro: zero em fixtures conhecidas. |
| **Violação de pré-condição** | ações da proposta executadas quando a pré-condição é falsa / ações propostas | Limite duro: zero. |
| **Violação de recurso** | casos com consumo acima da disponibilidade ou piso final abaixo do mínimo / casos executados | Limite duro: zero. |
| **Overrun de custo** | traços com custo total maior que o orçamento / traços propostos | Limite duro: zero; custo ausente não é custo zero. |
| **Erro de causalidade contextual** | intervenções em que o efeito previsto não coincide com o `do(action | context)` do oráculo / intervenções avaliadas | Deve ser zero para propostas aceitas. A taxa de abstenção é reportada à parte. |
| **Sensibilidade à ordem** | sequências inválidas aceitas entre casos não comutativos / sequências propostas | Limite duro: zero. |
| **Frescura de evidência** | propostas que referenciam digest de host/época anterior / propostas | Limite duro: zero; rebind deve invalidar contexto descartável. |
| **Separação proposta/execução** | execuções observadas além das sondas permitidas durante `propose_transfer()` | Limite duro: zero. |
| **Custo de descoberta** | número de `observe`, `execute`, resets, candidatos inspecionados e bytes de evidência por caso | Reportar distribuição exata; comparar com `DiscoveryBudget`, sem transformar velocidade em prova causal. |
| **Cobertura estrutural** | famílias causais com pelo menos um caso suportável e uma contraprova negativa executados | Descritiva. Não autoriza extrapolação para estruturas não testadas. |
| **Reprodução** | igualdade byte a byte de manifestos, digests, status e resultados em três execuções | Deve ser 100%; diferenças exigem diagnóstico e invalidam a rodada. |

A métrica `transfer_success` não pode ser usada sem a métrica de validade independente. Um plano pode ter o mesmo número de ações e ainda violar a ordem, o custo ou uma pré-condição. Da mesma forma, `correct_abstention` não deve contar como sucesso se a API não permitir distinguir `MISSING_RESOURCE`, `AMBIGUOUS_EFFECT`, `CONFLICTING_EVIDENCE` e `COST_UNKNOWN`; nesse caso o resultado é seguro, porém diagnosticamente incompleto. A extensão futura deveria retornar um status e um motivo tipados, em vez de apenas `None`.

## 9. Riscos de circularidade e controles

### 9.1 Oráculo e runtime compartilhando implementação

Se o oráculo usar as mesmas funções de transição, normalização ou aplicação de efeito do runtime, um mesmo defeito pode aparecer dos dois lados e produzir uma falsa taxa de acerto. O controle é um executor declarativo independente, com testes próprios, sem importação de `core.py`. O oráculo deve ser revisado e mutado separadamente.

### 9.2 Host “black-box” com introspecção acidental

O `BlackBoxHost` existente protege a propriedade `effects_by_name`, mas mantém `_inner` no objeto de teste [4]. Um teste pode ler o atributo privado, acessar `__dict__` ou compartilhar a fábrica do host. O controle é isolamento de processo ou proxy de serialização, além de um teste de instrumentação que falhe ao acessar qualquer campo não público. O oracle nunca pode ser anexado ao objeto entregue ao runtime.

### 9.3 Holdout contaminado por nomes ou convenções

Trocar apenas o nome de `left` por `servo7` não é novidade estrutural. Também é possível vazar a família por IDs como `host-gated` ou `requires-key`. O controle é gerar IDs opacos, medir distância por grafo causal e reservar a estrutura, não apenas strings ou valores, para o holdout.

### 9.4 Ajuste depois de observar o holdout

Escolher limiares, remover casos difíceis ou alterar a definição de “sucesso” depois da primeira execução converte holdout em validação. O controle é versionar manifesto, contrato, seed e limiar antes da rodada. Qualquer leitura do resultado de `H*` que altere código, fixture ou critério exige uma nova versão de toda a divisão.

### 9.5 Objetivo definido pelo mesmo executor que avalia a proposta

Se o campo `expected_final` do `TransferProposal` for aceito como verdade, a avaliação apenas repete o modelo do runtime. O controle é construir a função de satisfação sobre o traço real produzido pelo executor independente, incluindo estados intermediários, consumo e predicados proibidos.

### 9.6 Reset tratado como magia

A descoberta atual chama `reset()` após cada sonda [3]. Se o reset não restabelecer estado, recurso ou época, a próxima evidência carregará contexto anterior. O controle é validar o digest da observação inicial em cada episódio, registrar a época e rejeitar um reset que não devolva o baseline. Uma fixture temporal deve testar explicitamente a hipótese de que reset é confiável.

### 9.7 Confundir ausência de proposta com suporte causal

`None` pode significar host vazio, ambiguidade, conflito ou falta de capacidade, mas o runtime atual não expõe a razão [3]. O controle é manter `ABSTAIN_EXPECTED` separado de `SUPPORTED`, exigir diagnóstico tipado na futura extensão e contar saída sem motivo como `SAFE_BUT_UNPROVEN` quando o caso exigir explicação.

### 9.8 Vazamento por recursos e orçamento

Se `resources()` retornar a lista privada completa ou revelar a solução no nome do recurso, o caso deixa de medir inferência. O controle é separar capacidades públicas de fatos ocultos e testar que a ausência de recurso não pode ser preenchida por uma convenção de host ID. Um recurso desconhecido deve permanecer desconhecido.

### 9.9 Métrica agregada esconder falhas duras

Uma alta taxa de propostas válidas pode esconder uma única violação de autoridade. O contrato de utilidade já define limites duros de zero para violações de autoridade e falso consenso em fixtures conhecidas [2]. O holdout deve aplicar a mesma regra localmente: médias nunca compensam um caso inseguro.

## 10. Critérios de aceitação

### 10.1 Aceitação do plano antes da implementação

O plano só deve avançar para código quando os seguintes pontos estiverem congelados:

1. manifesto dos três splits com famílias causais, seeds e digests;
2. contrato de resultado que diferencia `PROPOSED`, `ABSTAIN`, `UNSUPPORTED` e `FAIL_UNSAFE`;
3. oráculo independente sem importação do runtime;
4. separação física ou de processo entre host público, oráculo e runtime;
5. objetivo composto com predicados positivos, negativos, pré-condições, custo e recursos;
6. limites de sondagem, passos, candidatos e execução;
7. política para resultados negativos e para casos ainda não representáveis pela API;
8. comando de reprodução, versão de Python e arquivos de resultados brutos;
9. revisão que confirme que nenhuma fixture depende de dados de usuário, sensores físicos ou utilidade social;
10. regra de que o holdout não será usado para ajustar a implementação.

### 10.2 Aceitação de uma implementação futura

Uma implementação futura pode declarar **suporte ao escopo estrutural medido** somente se cumprir todos os critérios seguintes:

- **Casos positivos:** 100% das propostas nos casos positivos explicitamente declarados suportáveis passam pelo executor independente sem violar objetivo, ordem, pré-condição, custo ou recurso.
- **Casos negativos:** 100% de abstenção em recurso ausente, ambiguidade, evidência conflitante, custo desconhecido, ação fora do espaço seguro e mudança temporal não resolvida.
- **Limites duros:** zero falso consenso, zero violação de autoridade, zero execução não revisada, zero recurso negativo, zero overrun e zero execução causada por `propose_transfer()`.
- **Causalidade:** toda proposta aceita é válida em cada contexto de intervenção que ela declara cobrir; a taxa de erro contextual é zero dentro do holdout medido.
- **Frescura:** nenhum digest de evidência, modelo ou contexto de um host anterior é usado como observação do novo host.
- **Orçamento:** nenhum limite é excedido silenciosamente; esgotamento produz falha controlada ou abstenção.
- **Reprodução:** três execuções com o mesmo commit, seed e manifest produzem resultados e digests idênticos.
- **Diagnóstico:** cada abstenção estrutural tem motivo tipado ou é marcada como segura porém não provada; não é permitido retroclassificar uma `None` depois de olhar o oráculo.

Se qualquer limite duro falhar, o estado da rodada é `not_proven`, independentemente da taxa de transferência positiva. Se a arquitetura passar somente em `D` e `V`, o resultado é mecanismo parcial, não suporte estrutural de holdout. Se uma família positiva de `H` ainda não for representável, a conclusão correta é `UNSUPPORTED_BY_CONTRACT`; ela pode orientar uma nova versão da API, mas não deve ser convertida em uma afirmação de simbiose geral.

## 11. Sequência recomendada de execução

A ordem segura é primeiro congelar o protocolo e depois medir a capacidade, sem misturar etapas:

1. Registrar o commit atual, o digest de `51-API-SIMBIONTE-V2.md`, o digest de `52-DEFINICAO-SIMBIOSE-UTIL.md` e o digest do contrato JSON.
2. Materializar apenas `D*` e construir o executor independente. Validar o próprio oracle com casos de fronteira e mutações.
3. Rodar o baseline `B0` sem alterar a implementação. Guardar sucessos, abstenções, falhas e custo de descoberta.
4. Congelar `V*`, executar uma vez para diagnóstico e não fazer ajustes ad hoc após iniciar a comparação final.
5. Congelar `H*` em uma manifestação separada. O runtime recebe somente o host público e o objetivo permitido.
6. Rodar cada holdout em processo limpo, com seed e orçamento registrados. Avaliar a proposta com o oráculo somente após o runtime terminar.
7. Rodar uma campanha de mutação: remover precondição, zerar custo, ocultar conflito, reutilizar digest antigo e expor ação perigosa. Cada mutação deve falhar em pelo menos uma contraprova definida.
8. Repetir a rodada três vezes e comparar resultados byte a byte.
9. Publicar resultado bruto, inclusive negativos e casos `UNSUPPORTED`, junto com a decisão de escopo. Não criar benchmark social nem alterar a classificação de utilidade humana.

## 12. Limites da conclusão

Este plano mede uma fronteira de representação e verificação em hosts determinísticos. Ele não mede percepção, linguagem, usuários, acessibilidade real, privacidade de produto, eficácia clínica, hardware, energia, rádio, atuadores ou impacto social. Mesmo uma aprovação integral significaria somente que o mecanismo foi demonstrado para as famílias causais e os orçamentos congelados. Não demonstraria simbiose geral, inteligência geral, compreensão aberta, segurança física ou utilidade social.

A definição congelada exige que mecanismo e valor humano sejam separados e que uma transferência entre simuladores seja rotulada como **mecanismo somente** quando não houver baseline humano [2]. Este relatório permanece deliberadamente dentro desse limite. Seu objetivo é tornar a próxima prova estrutural falsificável e auditável, não promover o resultado para uma alegação mais ampla.

## Referências

[1]: https://github.com/SummaArs/herus/blob/3fb3ca9d183cebe4df4d66c9cacae6c8220935c6/docs/51-API-SIMBIONTE-V2.md "API do Symbiont v2 — Etapa 1 da prova ASA"
[2]: https://github.com/SummaArs/herus/blob/3fb3ca9d183cebe4df4d66c9cacae6c8220935c6/docs/52-DEFINICAO-SIMBIOSE-UTIL.md "Definição congelada de simbiose útil — Etapa 3"
[3]: https://github.com/SummaArs/herus/blob/3fb3ca9d183cebe4df4d66c9cacae6c8220935c6/research/symbiont_v2/core.py "HERUS Symbiont v2 research runtime"
[4]: https://github.com/SummaArs/herus/blob/3fb3ca9d183cebe4df4d66c9cacae6c8220935c6/research/symbiont_v2/sim_hosts.py "Deterministic and black-box simulator hosts"
[5]: https://github.com/SummaArs/herus/blob/3fb3ca9d183cebe4df4d66c9cacae6c8220935c6/research/symbiont_v2/test_symbiont.py "Symbiont v2 executable tests"
[6]: https://github.com/SummaArs/herus/blob/3fb3ca9d183cebe4df4d66c9cacae6c8220935c6/research/symbiont_v2/benchmark.py "Symbiont v2 reproducible benchmark"
[7]: https://github.com/SummaArs/herus/blob/3fb3ca9d183cebe4df4d66c9cacae6c8220935c6/research/symbiosis_utility_contract.json "Frozen useful-symbiosis machine-readable contract"
