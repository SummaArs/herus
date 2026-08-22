# HERUS — Auditoria da inteligência atual e distância até o estado da arte

**Escopo:** somente capacidade local demonstrada no host.  
**Estado:** baseline host executado na branch `feat/herus-semantic-compiler`, após a extensão composicional e de abdução ainda em preparação para publicação.  
**Conclusão curta:** o HERUS já possui uma base de inteligência simbólica, memória seletiva, VSA/resonator, diálogo e planejamento com limites explícitos. Ainda não possui uma capacidade geral de linguagem, um modelo aprendido admitido para produção ou evidência comparável a uma LLM em tarefas abertas.

## 1. O que foi efetivamente demonstrado

| Componente | Evidência executada | Capacidade demonstrada | Limite da evidência |
|---|---:|---|---|
| Compilador semântico | **54/54** no contrato C11; benchmark separado **32/32 exact** | regras com até quatro premissas, composição, registry, rejeições e autoridade explícita | benchmark natural ainda fechado e pequeno; não mede compreensão aberta |
| Reasoner | **26/26** | fatos, regras, cadeias de derivação, prova compacta, ausência, contradição, ambiguidade, limites e abdução restrita read-only | lógica de primeira ordem muito restrita; não aprende regras novas |
| Planner | **9/9** | plano causal de três passos, custo, ordem, orçamento de nós, no-plan e confirmação | estados e ações são totalmente fornecidos pelo chamador |
| Diálogo simbólico integrado | **20/20** | compõe fatos, regras e VSA; propõe fato faltante, preserva ambiguidade e mantém confirmação/abstenção | não produz conversa livre; abdução só usa regras fornecidas |
| Resonator VSA | **9/9** | recupera fatores de produto rotacionado em codebook limitado, com margem e residual | fatoriza códigos conhecidos; não atribui significado sem codebook |
| Ponte VSA→símbolo | **6/6** | proposta tipada, negação, limiar de margem/residual, abstém ruído e não possui autoridade | não é um modelo semântico aprendido |
| Stress do resonator | **37/37**, 36 trials | comportamento limitado em codebooks 8/16/24 e corrupção controlada | sweep de codebook não equivale a benchmark de linguagem |
| Nucleus | **14/14** | aprendizagem opt-in de transições semânticas autorizadas, suporte, confiança, expiração e esquecimento | é memória de transições A→B, não geração aberta ou raciocínio geral |
| Diálogo runtime | **9/9** | sessão física, contexto tipado expirável, resposta local, zeroização e ausência de transcript persistente | o modelo local real ainda não está admitido no caminho de produção |
| Retrieval de memória | **10/10** | consulta tipada, ranking explicável, ambiguidade, política e cardinalidade limitada | não resume nem sintetiza conhecimento livre |
| Memory Grand Finale | **18/18** | captura, política, revisão humana, vault, retrieval e apresentação em uma cadeia composta | fixture host-only; não mede uso cotidiano |
| Model lab | **7/7** | gate para aceitar somente modelo local medido no alvo, dentro de orçamento e display-only | é uma barreira de admissão, não um modelo já disponível |

Esses resultados mostram um sistema coerente de **inferência simbólica limitada e memória sob controle humano**. Eles não mostram que o HERUS entende qualquer frase, conversa sobre qualquer assunto, aprende autonomamente ou iguala uma LLM generalista.

## 2. Forças técnicas atuais

A primeira força é a **disciplina epistemológica**. O reasoner retorna ausência, contradição, ambiguidade e limite como estados distintos. O diálogo não transforma uma conclusão barata em certeza. A ponte VSA exige confirmação antes de promover uma proposta a fato, e o planner devolve planos com fronteira de confirmação em vez de executá-los.

A segunda força é a **composição verificável**. Fatos pessoais confirmados podem ser combinados com regras de fábrica de múltiplas premissas e gerar conclusões acompanhadas de evidência. Quando a conclusão não é demonstrável, a abdução pode propor um fato ausente como hipótese, sem inseri-lo. Esse é um núcleo importante para um segundo cérebro: o sistema mostra de onde uma resposta veio e também distingue explicação possível de conhecimento confirmado.

A terceira força é a **contenção operacional**. O Nucleus aprende apenas quando habilitado, o retrieval não abre cartões por aproximação, a memória exige sessão e confirmação, e os módulos não possuem rádio, persistência ou autoridade de execução. O custo dessa segurança é deliberado: o HERUS prefere não responder a fabricar uma resposta.

## 3. Limitações que impedem a alegação de state of the art

O compilador atual reconhece um vocabulário e uma gramática fechados. O benchmark aceita 16 frases válidas cuidadosamente preparadas; isso prova estabilidade do contrato, mas não mede robustez a paráfrases, erros de digitação, elipse, referência temporal, negação escopal, perguntas compostas ou mudança de assunto. A taxa `32/32` deve ser lida como exact-match de uma microcorpus, não como acurácia de compreensão de linguagem natural.

O reasoner é generativo somente no sentido de aplicar regras fornecidas. Ele não aprende regras novas a partir de exemplos, não constrói autonomamente uma representação causal do mundo e não dispõe de um mecanismo de seleção de hipóteses comparável a busca guiada por linguagem. O resonator amplia a composição dentro de um codebook, mas um vetor desconhecido não ganha semântica por existir.

A memória atual é seletiva e segura, porém predominantemente **indexadora**. Ela armazena cartões tipados sob confirmação e recupera candidatos por consulta limitada. Ainda não há síntese multi-documento, atualização de crença temporal, resolução robusta de entidades, explicação de conflitos em linguagem natural ou aprendizagem de conceitos pessoais novos sem uma etapa explícita de engenharia.

O runtime de diálogo possui uma posição para um modelo local e um gate rigoroso, mas nenhum modelo medido e identificado foi promovido ao caminho de produção. Logo, não há resultado honesto de perplexidade, acurácia, latência, energia, WER, taxa de alucinação ou comparação contra uma LLM.

## 4. Diagnóstico de distância

| Capacidade | HERUS demonstrado | Nível necessário para uma comparação séria com LLMs |
|---|---|---|
| parsing | gramática fechada e exact-match | conjunto amplo de paráfrases, ruído e composição |
| raciocínio | regras fornecidas e proof trace | tarefas geradas com hipóteses, multi-hop e contraexemplos |
| memória | cartões tipados, retrieval ambíguo | retenção seletiva, atualização temporal, síntese e conflito |
| geração | templates/transições e proposições | respostas composicionais avaliadas por conteúdo e abstention |
| planejamento | busca simbólica curta | tarefas variadas, custos, recursos e replanejamento |
| conhecimento | fábrica/pessoal explicitamente carregados | cobertura de domínio mensurável e atualização controlada |
| aprendizado | transições autorizadas | generalização fora da sequência observada, sem esquecer segurança |
| linguagem háptica | contrato semântico e encoder | percepção humana e channel coding físico |
| modelo aprendido | gate de admissão, nenhum modelo promovido | modelo local medido, quantizado, identificado e avaliado |

A conclusão de engenharia é que “state of the art” não deve ser um rótulo global. O HERUS pode buscar estado da arte em **tarefas delimitadas de memória privada, raciocínio auditável, abstenção e controle humano**, mas não deve alegar equivalência geral com uma LLM enquanto o benchmark não cobrir os limites acima.

## 5. Próximo alvo de maior impacto

O próximo avanço de maior impacto não é adicionar mais regras isoladas. É executar o benchmark composicional v1 que já foi especificado, gerando instâncias novas a partir de um DSL de entidades, relações, tempo, negação, conflito, abdução e objetivos. Cada instância deve possuir resposta canônica, prova mínima, casos ambíguos e casos sem evidência. O mesmo conjunto deve avaliar compilador, reasoner, memória, retrieval e planner em cadeia.

Depois que esse benchmark existir, o próximo núcleo inteligente deve ser escolhido pela falha dominante observada: resolução de entidades, atualização temporal, síntese de múltiplos cartões ou busca de planos sob incerteza. Assim o HERUS poderá melhorar por evidência, e não por quantidade de módulos.

## 6. O que não foi alegado

Esta auditoria não afirma que o HERUS é uma LLM, não afirma compreensão aberta, não afirma inteligência geral, não afirma aprendizado autônomo, não afirma modelo local pronto, não afirma acurácia fora do microcorpus, não afirma desempenho físico, não afirma equivalência perceptiva háptica e não afirma state of the art global. O estado correto é **base simbólica local forte em segurança e rastreabilidade, porém ainda estreita em cobertura e generalização**.

## Referências internas

- [`firmware/core/test_semantic_compiler_benchmark.c`](../firmware/core/test_semantic_compiler_benchmark.c)
- [`firmware/core/test_symbolic_reasoner.c`](../firmware/core/test_symbolic_reasoner.c)
- [`firmware/core/test_symbolic_planner.c`](../firmware/core/test_symbolic_planner.c)
- [`firmware/core/test_symbolic_dialogue.c`](../firmware/core/test_symbolic_dialogue.c)
- [`firmware/core/test_resonator.c`](../firmware/core/test_resonator.c)
- [`firmware/core/test_resonator_bridge.c`](../firmware/core/test_resonator_bridge.c)
- [`firmware/core/test_resonator_stress.c`](../firmware/core/test_resonator_stress.c)
- [`firmware/core/test_nucleus.c`](../firmware/core/test_nucleus.c)
- [`firmware/core/test_dialogue.c`](../firmware/core/test_dialogue.c)
- [`firmware/core/test_memory_retrieval.c`](../firmware/core/test_memory_retrieval.c)
- [`firmware/core/test_memory_finale.c`](../firmware/core/test_memory_finale.c)
- [`firmware/core/test_model_lab.c`](../firmware/core/test_model_lab.c)
