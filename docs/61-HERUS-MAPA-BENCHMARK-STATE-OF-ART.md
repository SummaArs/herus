# HERUS — Mapa de benchmark para inteligência delimitada

**Objetivo:** medir avanço real do HERUS em tarefas de raciocínio, memória e abstenção sem confundir uma microcorpus simbólica com equivalência geral a uma LLM.

## 1. Princípio de comparação

O HERUS não será comparado a uma LLM por uma impressão subjetiva de conversa. A comparação deve ocorrer em tarefas com entrada, saída, evidência e regra de pontuação definidas antes da execução. A tarefa deve ser executável localmente, sem rede e sem modelo hospedado. Quando um benchmark externo depender de conhecimento mundial, ele será usado apenas como inspiração metodológica ou será reduzido a um domínio fechado com fatos fornecidos na entrada.

A métrica principal não será uma acurácia única. Cada sistema deverá reportar **resposta correta**, **prova correta**, **abstenção correta**, **erro confiante**, **contradição detectada**, **custo de busca**, **memória usada** e **autoridade violada**. Um sistema que responde mais, mas inventa, não supera um sistema que abstém corretamente.

## 2. Comparadores escolhidos

| Dimensão | Referência externa | Adaptação local para o HERUS | Métricas HERUS |
|---|---|---|---|
| dedução e provas | ProofWriter/RuleTaker | teorias sintéticas com fatos, regras, negação e mundo aberto | resposta, prova mínima, profundidade, `unknown`, abdução correta |
| generalização composicional | SCAN | DSL de intenções e ações simbólicas com splits de nova primitiva, template e comprimento | exact-match fora da composição observada, profundidade, limite |
| raciocínio implícito | StrategyQA | perguntas sobre domínio factory/pessoal fornecido, com decomposição obrigatória | resposta, decomposição, evidência usada, abstention |
| abstração | ARC-AGI-2 | tarefas simbólicas de transformação de estruturas, sem imagens no primeiro estágio | regra inferida, generalização para tarefa nova, número de hipóteses |
| memória longitudinal | LongMemEval | sessões sintéticas de cartões autorizados com atualização temporal, conflito e abstention | extração, multi-sessão, tempo, update, retrieval e abstention |
| arquitetura de memória | framework modular de memória | separar extração, gestão, armazenamento e retrieval no pipeline HERUS | custo por etapa, perda, conflito, expiração, privacidade |
| abstenção | survey de Wen et al. | eixos answerability, confiança da prova e política/autoridade | coverage-risk, erro confiante, abstention apropriada e falsa abstention |

As referências são comparadores de desenho, não metas numéricas transplantadas. Os resultados publicados por outros sistemas dependem de modelos, dados, prompts, hardware e avaliadores que não existem no HERUS.

## 3. Benchmark HERUS em quatro camadas

### Camada A — interpretação e compilação

A entrada é uma frase em um subconjunto documentado de português natural, com paráfrases e ruído controlado. A saída é uma unidade tipada com símbolos collision-aware, slots de entidade, polaridade, tempo, escopo e confirmação. Os casos incluem frase não suportada, conteúdo sensível, entidade desconhecida e duas interpretações igualmente plausíveis.

O split de generalização deve esconder combinações, não os primitivos. Por exemplo, o treino pode conter `possui caderno` e `pode estudar`, mas o teste combina entidades e relações em uma consulta temporal inédita. A pontuação deve separar falha de parsing, falha de desambiguação e falha de raciocínio.

### Camada B — raciocínio e prova

Cada instância fornece fatos, regras, contradições possíveis e um objetivo. A resposta permitida é `true`, `false`, `unknown`, `contradicted` ou `limit`. Quando houver resposta derivada, o sistema deve fornecer os pais e as regras usadas. Quando a resposta não for provada, nenhuma cadeia inventada pode pontuar.

A extensão de maior valor sobre o benchmark atual é incluir **enumeração de consequências** e **abdução restrita**. Na abdução, a saída é um fato ausente que tornaria o objetivo demonstrável, marcado como hipótese e nunca inserido automaticamente na memória.

### Camada C — memória seletiva

A campanha gera sessões autorizadas contendo fatos pessoais, preferências, eventos, datas, atualizações, duplicatas e conflitos. O sistema deve decidir o que é candidato a memória, registrar apenas a representação permitida, atualizar ou rejeitar cartões conflitantes e recuperar o mínimo necessário.

Cada pergunta é classificada em cinco eixos: extração, multi-sessão, temporal, atualização e abstenção. A avaliação não permite que o sistema obtenha pontuação por guardar transcript bruto, identidade, áudio, localização ou chave. A resposta deve apontar o card técnico e sua proveniência interna, ou abster-se.

### Camada D — planejamento e interface

A entrada fornece estado, objetivo, ações, custo e requisitos de confirmação. O sistema deve produzir um plano ou `no_plan`, contar nós explorados, expor custo e marcar cada passo que exige gesto humano. A campanha inclui objetivos impossíveis, ações contraditórias, ciclos, orçamento insuficiente e uma ação de alto custo com plano barato concorrente.

O planner não recebe autoridade de execução. A pontuação de planejamento mede qualidade do plano, não se o sistema efetuou uma ação.

## 4. Métricas obrigatórias

| Métrica | Definição | Falha crítica |
|---|---|---|
| exact-match | saída canônica completa correta | não distingue resposta de prova |
| proof-validity | prova aceita pelo verificador local | prova plausível mas inválida |
| unknown precision | proporção de `unknown` realmente sem prova | responder sem evidência |
| abstention recall | casos bloqueados corretamente recusados | falsa segurança |
| error confidence | resposta errada sem estado de incerteza | alucinação operacional |
| contradiction recall | conflitos detectados e não colapsados | perda de evidência |
| compositional generalization | desempenho em combinação fora do treino | memorização de templates |
| temporal update accuracy | estado mais recente e validade respeitados | memória obsoleta |
| proof cost | passos, nós, bytes e tempo do host | custo oculto/unbounded |
| authority violations | qualquer persistência, envio ou execução sem confirmação | falha de segurança |

## 5. Comparação com LLMs sem autoengano

Só será possível dizer que o HERUS “se equipara” a uma LLM em uma tarefa se ambos receberem a mesma entrada, o mesmo conjunto de fatos/evidências, a mesma definição de saída e a mesma regra de pontuação. Para tarefas abertas, o HERUS deve ser declarado fora de escopo até possuir um mecanismo local de cobertura suficiente e uma avaliação humana/automática adequada.

A comparação também deve distinguir **qualidade da resposta** de **qualidade da memória**. Um sistema externo pode usar um modelo hospedado para extrair, atualizar e julgar; o HERUS não pode importar essa vantagem sem declarar que deixou de ser soberano. Por isso, o benchmark inicial será próprio, determinístico e auditável, com baselines simples: parser atual, reasoner atual, planner atual, retrieval atual e composição completa.

## 6. Decisão para a próxima fase

O maior retorno agora é implementar o **benchmark composicional próprio** com geração determinística de teorias, splits de generalização, provas canônicas e casos de abstenção. Ele deve ser implementado antes de adicionar um novo mecanismo de aprendizagem. Caso o benchmark mostre que o gargalo é a linguagem, evolui-se o compilador; caso mostre que é composição, evolui-se a representação/prova; caso mostre que é memória, evolui-se atualização temporal e conflito. A arquitetura deve obedecer ao resultado, não a uma preferência prévia.

## Referências

[1]: https://arxiv.org/html/2012.13048v2 "ProofWriter: Generating Implications, Proofs, and Abductive Statements over Natural Language"

[2]: https://github.com/brendenlake/SCAN "SCAN tasks for compositional learning"

[3]: https://aclanthology.org/2021.tacl-1.21/ "Did Aristotle Use a Laptop? A Question Answering Benchmark with Implicit Reasoning Strategies"

[4]: https://arxiv.org/abs/2505.11831 "ARC-AGI-2: A New Challenge for Frontier AI Reasoning Systems"

[5]: https://arxiv.org/abs/2410.10813 "LongMemEval: Benchmarking Chat Assistants on Long-Term Interactive Memory"

[6]: https://arxiv.org/html/2604.01707v1 "Memory in the LLM Era: Modular Architectures and Strategies in a Unified Framework"

[7]: https://doi.org/10.1162/tacl_a_00754 "Know Your Limits: A Survey of Abstention in Large Language Models"
