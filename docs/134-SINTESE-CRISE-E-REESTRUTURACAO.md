# HERUS — síntese de crise científica e reestruturação prática

**Data:** 2026-10-07  
**Escopo:** síntese das cinco auditorias fornecidas.  
**Decisão executiva:** congelar a expansão de claims e de arquiteturas. O caminho de baixo custo é transformar o HERUS, por ora, em um **gate de recuperação auditável com abstenção**, testado em um único domínio e sob holdout temporal/por entidade. Qualquer linguagem de simbiose geral, causalidade, transferência ampla, segurança operacional, AGI ou superioridade deve ser retirada até existir demonstração específica.

## 1. O que morreu

As seguintes teses não estão apenas “incompletas”; os artefatos auditados não as identificam e, portanto, elas deixam de ser claims científicos ativos:

- **Aprendizagem simbiótica geral, AGI, RL geral e interação com um parceiro.** Não há segundo agente, modelo explícito do parceiro, intervenção, recompensa, ambiente sequencial, negociação de objetivos ou feedback interpretado.
- **Predição end-to-end do efeito a partir da entrada.** `propose` recebe `target_effect`; quando esse alvo vem do rótulo verdadeiro, há vazamento estrutural. O núcleo atual é um lookup/retriever de `(contexto, efeito) -> ação`, não um preditor do efeito.
- **Causalidade, contrafactuais e transferência semântica.** `Episode` registra associação `before/after`; não há intervenção, outcome independente, contrafactual nem generalização para efeitos, estados ou contextos novos. `before` sequer participa adequadamente da decisão atual.
- **Generalização ampla e superioridade universal.** MIntRec/MInDS-14 são, no máximo, smoke tests ou controles textuais; seus resultados não validam o núcleo HERUS, multimodalidade, áudio, transferência entre hosts ou utilidade humana.
- **SOTA, vencedor universal, eficiência universal e metas de 94/95/100%.** Não há denominador, definição, regra de atualização, comparador único e ledger executado que sustentem esses números. O scorecard é inconsistente entre variantes, splits, seeds e objetos.
- **`confidence` como probabilidade calibrada ou garantia futura.** `confidence_milli` é um limite de Wilson sobre sucessos positivos selecionados. Falhas são ignoradas; para 1, 2, 5 e 32 sucessos sem falha, resulta aproximadamente em 0,206, 0,342, 0,565 e 0,893, sem medir precisão futura.
- **Conformal assurance e segurança de produção.** O módulo conformal trata rótulos de MIntRec, não ações/efeitos HERUS; a separação temporal não garante exchangeability nem segurança operacional. Testes internos, fail-closed, host-only e proposal-only são contenção de engenharia, não eficácia ou segurança de campo.
- **Acurácia seletiva como acurácia geral.** 1,0 em 24/386 casos e 0,931818 em cobertura 0,227979 não são vitórias gerais. Sem piso de cobertura, custo de abstenção, intervalos e comparação pareada, são compatíveis com seletividade trivial.
- **Causalidade inferida de bootstrap/simulações.** 10.000 reamostragens não criam dados; seeds não são amostras independentes. O ledger causal está vazio (0/3.474 linhas), e o protocolo é apenas de desenho.

**Regra de linguagem a partir de agora:** “HERUS” não deve ser usado como sinônimo de uma competência única. Em cada resultado, declarar explicitamente se se trata de `HERUS-core`, um adaptador textual, um controle estatístico, uma ablação ou infraestrutura de contenção.

## 2. O que permanece valioso

O projeto não é vazio. O que permanece defensável e útil é mais estreito:

- **Memória episódica auditável:** há uma estrutura simples para registrar contexto, efeito, ação, timestamp/step e provenance. Ela pode ser útil como memória de casos, desde que exemplos positivos e negativos sejam preservados e a unidade independente seja explícita.
- **Abstenção como comportamento primário:** o núcleo parece mais apto a evitar propostas sem evidência do que a garantir acerto. A abstention deve ser tratada como saída de primeira classe, com cobertura, risco, custo e motivo registrados.
- **Proposta-only e host-only:** manter zero execução externa no experimento. Isso limita danos e permite falsificação barata antes de qualquer integração com dispositivo, agente ou ambiente real.
- **Identidade reprodutível do conjunto:** os 386 IDs e pares `season/episode/clip` e o hash SHA-256 canônico são bons elementos de auditoria. Devem ser preservados, mas não confundidos com autenticidade ou proveniência externa.
- **Harness de falhas e disciplina de ledger:** os testes internos, manifestação imutável, configuração versionada e ledger por `example_id` são fundações de engenharia. O ledger precisa registrar execução real, não apenas plano.
- **Controle de deriva como problema:** a preocupação com tempo, conflito e mudança de contexto é válida; a implementação atual mede distância de `step`, não mudança de distribuição. A oportunidade é testar deriva com holdout temporal real.
- **Comparação pareada por exemplo/episódio:** é útil para reduzir variação, desde que a unidade do estimando, cluster de dependência e replicação por seed sejam separados.
- **Hipótese de transferência entre interfaces, em versão futura:** hosts permutados são uma boa forma de testar mecanismo sem alegar multimodalidade ou AGI. Não entra na primeira campanha; fica como extensão após o gate básico.

## 3. Nova tese central: estreita, executável e falsificável

### Tese H1 — gate seletivo de memória episódica sob deriva

> **Em um único domínio fechado, com `example_id` estável, contexto e ação definidos, resultado positivo/negativo independente, timestamp e fonte de `target_effect` independente do rótulo de teste, um gate HERUS que recupera casos exatos, incorpora contraevidência negativa, calibra um limiar em período temporal separado e abstém em conflito/deriva atingirá precisão entre decisões aceitas de pelo menos 95%, com cobertura mínima de 30%, no holdout temporal; além disso, superará em pelo menos 3 pontos percentuais, no mesmo regime de cobertura, um classificador simples com limiar de confiança e o baseline de ação mais frequente condicionado ao contexto.**

Esta tese **não** afirma causalidade, transferência universal, melhoria humana, multimodalidade, RL ou superioridade sobre modelos de linguagem. Ela é sobre uma propriedade operacional mensurável: **decisões aceitas, auditáveis e seletivas em um domínio fixo**.

### Objeto mínimo congelado antes do teste

1. **Entrada:** um registro textual/estruturado de contexto e estado atual; o estado atual e precondições devem participar da decisão.
2. **Saída:** uma ação proposta ou `ABSTAIN`; nenhuma ação é executada externamente.
3. **Evidência:** casos com `entity_id`, timestamp, `context`, `before`, `action`, `after`, `outcome` positivo/negativo, custo, risco, provenance, verifier e delay.
4. **Target:** jamais derivado de `y_true` do holdout; sua origem e versão entram no `Object Lock`.
5. **Memória:** positivos e negativos entram na contagem; conflito, contexto vazio e evidência velha devem reduzir confiança ou provocar abstention.
6. **Partições:** ajuste em S04, calibração em S05, holdout intocado em S06; adicionalmente, holdout temporal e, se possível, por entidade/episódio.
7. **Métricas:** precisão aceita, cobertura, risco de abstention, cobertura por classe, custo, conflitos, latência, taxa de proposta inválida e intervalo de confiança por episódio. Reportar também precisão global; não mascarar abstention.
8. **Baselines:** ação mais frequente condicionada ao contexto, baseline simples com limiar de confiança e memória/lookup trivial. Um único controle primário deve ser nomeado e versionado.

### Pré-registro de falsificação

H1 falha se houver qualquer vazamento de target, se a cobertura ficar abaixo de 30%, se a precisão aceita ficar abaixo de 95%, se o limite inferior do intervalo não sustentar o piso, se não houver vantagem de pelo menos 3 p.p. contra o controle na cobertura pareada, ou se o ganho depender de um episódio/classe e desaparecer no leave-one-episode-out. A conclusão positiva, caso ocorra, será apenas “evidência para um gate seletivo no domínio X”, nunca SOTA.

## 4. Arquitetura e repositório a reorganizar

### 4.1 Separar objetos científicos

Substituir a fachada única por quatro camadas com contratos próprios:

```text
herus-crisis-research/
├── 00-sintese-crise-e-reestruturacao.md
├── object-lock/
│   ├── v1.yaml                 # entrada, target, unidade, split, métricas, claims permitidas
│   ├── schema_episode.json     # contrato e tipos
│   └── decision_policy.md      # proposta, empate, abstention, custo e risco
├── herus_core/
│   ├── memory.py               # recuperação episódica; sem treinamento escondido
│   ├── evidence.py             # positivos, negativos, conflito e provenance
│   ├── gate.py                 # limiar, deriva, abstention e explicação
│   └── version.py              # versão do contrato e da implementação
├── adapters/
│   └── domain_x.py             # texto/estado -> objeto congelado; sem ler y_true
├── baselines/
│   ├── majority_context.py
│   ├── simple_threshold.py
│   └── finite_memory.py
├── experiments/
│   ├── manifests/              # dados, código, pesos, tokenizer, config, seed, hashes
│   ├── run_gate.py
│   ├── adversarial_harness.py
│   └── temporal_holdout.py
├── ledgers/
│   ├── episodes.jsonl          # uma linha por exemplo, episódio e seed conforme contrato
│   └── README.md
├── reports/
│   ├── risk_coverage.csv
│   ├── episode_deltas.csv
│   └── claims.md
└── archive/
    ├── exploratory/            # MIntRec/MInDS-14 e variantes, sem claims HERUS
    └── deprecated_claims.md
```

### 4.2 Regras de implementação

- Colocar um **Object Lock antes dos Gates 0–6**. Ele fixa entrada, fonte do target, unidade independente, negativos, split, atribuição do pipeline, métricas, versão, seed e claim permitida.
- Fazer `propose` receber estado atual/precondições e separar `candidate_effect` de `observed_success`. O objeto atual não deve fingir prever o efeito se o efeito já foi fornecido.
- Renomear `confidence` para **stability/support of evidence** até existir calibração futura. Wilson não é precisão futura.
- Corrigir `step=0` e remover truthiness ambígua; substituir distância de step por uma regra de holdout temporal/deriva definida e testável.
- Fazer negativos funcionarem como contraevidência real; registrar atraso, custo, risco, verificador e motivo da abstention.
- Tornar contexto vazio explícito: não pode casar com tudo por padrão.
- Manter `data`, `program` e `learning` como módulos versionados e independentes. A fachada, se mantida, deve declarar que é conveniência de API, não uma competência científica única.
- Reconstruir o scorecard por `run_id`, commit, pipeline, split, seed, hash de dados/pesos/configuração, `example_id` e definição de utility. Não misturar MIntRec/MInDS-14 com validação do core.
- Parar a adição de novas famílias de modelos até haver ledger completo e comparação com controles triviais. Arquivar, não apagar, resultados exploratórios e claims revogadas.

## 5. Campanha experimental de três marcos

### Marco 1 — congelamento e teste adversarial barato (dias 1–2)

**Objetivo:** verificar se há um objeto testável, antes de gastar em modelos.

- Publicar o `Object Lock v1` e manifesto imutável.
- Especificar adaptador texto/estado, ações, target independente, semântica de `positive/negative/success`, abstention, utility e custo.
- Importar o módulo sem alterá-lo e executar harness sintético com: target derivado de rótulo (deve bloquear), aliases, contexto vazio, estado incompatível, positivo seguido de negativo, evidência antiga, conflito, `step=0` e ordem embaralhada.
- Criar o ledger por `example_id`, com `run_id`, seed, versão e hashes.

**Gate:** só avançar se o sistema falhar fechado quando houver vazamento/ambiguidade e se a decisão for reproduzível após embaralhamento. Se a interface não suportar isso, rebatizar o artefato como retriever de casos, não como preditor.

### Marco 2 — microbenchmark temporal de baixo recurso (dias 3–7)

**Objetivo:** testar H1 em um único domínio, sem host físico, multimodalidade ou novos encoders.

- Usar ajuste S04, calibração S05 e holdout S06; manter também corte temporal/por entidade quando possível.
- Comparar somente: `HERUS-gate`, baseline de maioria por contexto, baseline simples com limiar de confiança e finite memory.
- Rodar três seeds; tratá-las como replicações, não como novas amostras. A unidade primária é exemplo, a dependência é episódio e macro-F1 é secundária.
- Produzir curva risco–cobertura, ponto pré-registrado de 30%, precisão aceita, cobertura, abstention, custos, conflitos, taxa de proposta inválida, resultados por classe e por episódio, intervalo pareado e leave-one-episode-out.
- Completar o ledger real antes de qualquer bootstrap; usar bootstrap de episódio somente como análise de sensibilidade.

**Gate:** avançar apenas se a auditoria de identidade, ausência de leakage e métricas estiver completa; o desempenho não precisa ser “SOTA”, mas deve ser comparável e reproduzível.

### Marco 3 — falsificação de mecanismo ou pivot controlado (semana 2)

**Objetivo:** distinguir memória seletiva de artefato de composição/lexicalidade.

- Repetir o teste em pelo menos três cortes temporais e com ablação de contexto, negativos, calibração e memória.
- Testar a interação pré-registrada: o ganho deve aparecer mais em itens de baixa margem/discordância entre controles do que em itens de alta confiança/concordância, e permanecer estável entre episódios.
- Se o Marco 2 passar, executar a extensão barata de interfaces permutadas em três hosts simulados, proposal-only, três seeds, holdout de host não visto; comparar sem memória, memória atual, HERUS-gate e baseline genérico. Não chamar isso de transferência geral.
- Se a interação não for estável, se o ganho concentrar classes frequentes ou se desaparecer com ablação de contexto, pivotar para diagnóstico lexical/monitoramento.

**Entregável:** um relatório com três possíveis conclusões — (a) evidência limitada para gate seletivo; (b) efeito de composição/lexicalidade; (c) ausência de efeito — e claims permitidas para cada caso.

## 6. Critério objetivo para abandonar novamente a tese

Abandonar H1 sem novo tuning, sem aumentar a arquitetura e sem mover o alvo se ocorrer qualquer uma das condições abaixo no protocolo congelado:

1. **Vazamento ou objeto inválido:** qualquer `target_effect` dependente do rótulo de teste, partição contaminada, `example_id` inconsistente, provenance ausente ou decisão não reproduzível.
2. **Falha do piso operacional:** cobertura <30% no ponto pré-registrado, precisão aceita <95% ou limite inferior do intervalo abaixo de 95%.
3. **Ausência de ganho discriminante:** vantagem <3 p.p. contra o controle primário na mesma cobertura, ou controle trivial igual/melhor.
4. **Instabilidade:** resultado não replicado em três seeds, cortes temporais ou leave-one-episode-out; ou ganho explicado por um único episódio/classe.
5. **Abstenção enganosa:** custo/risk ajustado pior que o baseline, taxa de proposta inválida não menor, ou melhora obtida apenas recusando casos sem utilidade definida.
6. **Mecanismo não identificado:** remover contexto, negativos ou calibração não altera o resultado, ou a interação de dificuldade/concordância não aparece; nesse caso, não chamar o efeito de contextual ou simbiótico.

Ao falhar, o projeto deve ser **rebaixado definitivamente nesta fase** para “memória episódica auditável/monitoramento com abstenção” e encerrar investimento em SOTA, transferência ampla, RL, AGI e segurança de campo. Só uma nova tese, com novo Object Lock e novo comparador, pode ser proposta; não se deve reinterpretar a mesma falha como sucesso parcial.

## Conclusão

A crise é de **objeto e prova**, não de falta de arquiteturas. O caminho executável com poucos recursos é reduzir o HERUS a uma propriedade observável, bloquear vazamento, medir risco–cobertura contra baselines triviais e registrar cada decisão. Se esse gate estreito não vencer de forma estável e auditável, a contribuição honesta será infraestrutura de contenção e auditoria — útil, mas não aprendizagem simbiótica geral.
