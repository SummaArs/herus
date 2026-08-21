# Achados acadêmicos — rodada 1

## 1. Vaswani et al. (2017), *Attention Is All You Need*

Fonte primária: https://arxiv.org/abs/1706.03762

O trabalho propõe o Transformer como uma arquitetura baseada exclusivamente em mecanismos de atenção, eliminando recorrência e convoluções do modelo de transdução. A contribuição não foi apenas adicionar uma função de atenção: foi mudar a unidade central de computação e tornar o processamento mais paralelizável. O resumo relata ganhos em tradução WMT14, incluindo 28,4 BLEU em inglês-alemão e 41,8 BLEU em inglês-francês, sob a configuração experimental do artigo.

Implicação para o HERUS: uma contribuição de nível paradigmático teria de mudar a unidade central de uma inteligência pessoal, e não apenas adicionar memória ou um sensor. A unidade candidata do HERUS não deve ser “token gerado”, mas uma **transição autorizada de estado semântico pessoal**: observação tipada → candidato → evidência/proveniência → estado atual ou abstention → ação local limitada. Isso é uma hipótese própria, ainda não uma reivindicação de novidade.

## 2. Lewis et al. (2020), *Retrieval-Augmented Generation for Knowledge-Intensive NLP Tasks*

Fonte primária: https://arxiv.org/abs/2005.11401

O trabalho combina memória paramétrica de um modelo seq2seq com memória não paramétrica em um índice denso da Wikipédia, acessado por um retriever neural. A motivação explícita inclui dificuldade de acessar e manipular conhecimento com precisão, atualizar conhecimento de mundo e fornecer proveniência. O artigo compara uma formulação que usa os mesmos trechos recuperados durante toda a geração com outra que pode usar trechos diferentes por token, e relata resultados fortes em tarefas de perguntas e respostas abertas.

Implicação para o HERUS: RAG resolve principalmente **acesso a conhecimento externo durante geração**. O HERUS precisa resolver outra coisa: **governança de continuidade pessoal**, decidindo o que pode entrar, permanecer, expirar, ser superseded, conflitar e tornar-se autoridade. Um índice denso ou memória recuperável não é suficiente para isso; recuperar não prova que o fato é atual, autorizado ou não contradito.

## Hipóteses provisórias

| Observação da literatura | Lacuna relevante para o HERUS |
|---|---|
| Attention muda a unidade de computação da sequência | HERUS pode precisar de uma unidade de transição semântica autorizada, não de token ou passagem recuperada |
| RAG separa memória paramétrica e não paramétrica | HERUS precisa separar candidato, evidência revisada, estado atual, conflito e autoridade |
| Proveniência é reconhecida como problema em conhecimento recuperado | Proveniência no HERUS deve ser condição de promoção e não apenas metadado de resposta |
| Atualização de conhecimento é difícil | No HERUS, atualização precisa preservar floor, validade, supersession e abstention |

## Limites

Esta rodada não prova que a hipótese do HERUS é original. Ela apenas estabelece duas bases primárias e uma diferença conceitual inicial. A pesquisa precisa consultar trabalhos de aprendizado contínuo, memória episódica/semântica, agentes pessoais e raciocínio neuro-simbólico, além de verificar se arquiteturas semelhantes já foram propostas.

## 3. Kirkpatrick et al. (2017), *Overcoming catastrophic forgetting in neural networks*

Fonte primária: https://www.pnas.org/doi/10.1073/pnas.1611835114

O artigo formula o problema de aprendizado sequencial: tarefas posteriores podem alterar pesos importantes para tarefas anteriores. A proposta Elastic Weight Consolidation (EWC) reduz a plasticidade de pesos considerados importantes para conhecimentos anteriores, inspirada em consolidação sináptica, e demonstra retenção em tarefas sequenciais de classificação e jogos Atari. O texto também destaca que replay de dados pode exigir memória proporcional ao número de tarefas.

Implicação para o HERUS: aprender continuamente não deve significar alterar indiscriminadamente um modelo central. O HERUS pode separar uma camada estável de competência de uma camada pessoal plástica e governada. A unidade mínima de atualização deve ser uma evidência semântica com revisão, validade e conflito; adaptação paramétrica, se existir, deve ser subordinada a esse registro e poder ser congelada ou revertida.

## 4. Zhu et al. (2024), *On-device Training: A First Overview on Existing Systems*

Fonte primária: https://arxiv.org/html/2212.00824v3

A revisão identifica três desafios de treinamento on-device: descompasso entre recursos e demanda, heterogeneidade dos dispositivos e limitações do trabalho existente. Destaca que backpropagation armazena intermediários em grande volume, enquanto microcontroladores possuem memória em kilobytes ou megabytes. Também registra vantagens potenciais: privacidade por processamento local, personalização, adaptação a drift, menor dependência de conectividade e menor custo de comunicação.

Implicação para o HERUS: colocar um modelo grande no pulso não é a única forma de aprender. É provável que a contribuição deva usar **aprendizado sem atualização paramétrica contínua** como caminho principal: o vestível constrói e governa um estado semântico pessoal compacto, com plasticidade explícita, enquanto o Core apenas alimenta conhecimento autorizado. Isso respeita memória, energia e soberania melhor que tentar treinar um LLM inteiro no ESP32.

## Síntese provisória da rodada

A literatura sugere três separações necessárias: attention mudou a unidade de computação; RAG separou memória paramétrica de não paramétrica; EWC separou plasticidade de estabilidade. O HERUS pode buscar uma quarta separação: **separar a geração de uma interpretação, a promoção de uma experiência a memória e a autorização de uma ação**. Essa separação é uma hipótese de pesquisa, não ainda uma contribuição comprovadamente original.

## 5. Jiang et al. (2026), *Advancing Multimodal Agent Reasoning with Long-Term Neuro-Symbolic Memory*

Fonte primária: https://arxiv.org/html/2603.15280v1

O NS-Mem propõe três camadas: episódica, semântica e lógica. A episódica armazena observações temporais com descrição e embedding; a semântica consolida conhecimento associado a entidades; a lógica contém regras e DAGs procedurais, com links de volta para evidências episódicas. O trabalho descreve manutenção incremental, fusão de conhecimento e consultas simbólicas combinadas com descoberta neural. A arquitetura é muito próxima de algumas intenções do HERUS na separação episódico-semântico-lógica.

Implicação para a novidade: não é defensável afirmar que o HERUS inventou uma arquitetura de memória episódica-semântica-lógica. A diferença possível precisa estar em outro eixo: soberania on-wrist, ausência de texto/áudio/embedding no núcleo de memória, promoção baseada em autoridade física, floor anti-rollback, conflitos que geram abstention, ação local limitada e adaptação a recursos extremos. Mesmo esse conjunto precisa ser comparado com literatura adicional antes de ser chamado de contribuição original.

## 6. MemoryAgentBench / OpenReview

Fonte identificada: https://openreview.net/forum?id=DT7JyQC3MR

O acesso automatizado encontrou uma verificação de navegador do OpenReview e não permitiu ler o artigo completo. Portanto, nenhum detalhe do benchmark foi tratado como evidência confirmada. A existência do resultado reforça, porém, a direção metodológica: memória de agentes deve ser avaliada por competências distintas, como recuperação correta, aprendizado durante uso e retenção, e não por um único número agregado.

## Atualização da lacuna

A hipótese de uma “quarta separação” precisa ser refinada. A separação episódico-semântico-lógica já aparece em trabalho recente. O possível espaço do HERUS é uma **máquina de transições de autoridade semântica** que controla quando uma percepção pode se tornar memória, quando memória pode influenciar uma oferta e quando uma oferta pode produzir ação — com soberania local e abstention como estados de primeira classe. A novidade ainda é uma hipótese de engenharia/pesquisa, não prioridade científica estabelecida.

## 7. Luo, Wen e Wang (2026), *Agentic Abstention: Do Agents Know When to Stop Instead of Act?*

Fonte primária: https://arxiv.org/html/2606.28733v1

O trabalho define agentic abstention como reconhecer que uma tarefa é inexequível e abster-se em vez de responder incorretamente ou executar ações desnecessárias. Formaliza o problema como POMDP com ações ANSWER, ABSTAIN e ACT, em que ACT pode obter informação adicional, enquanto ANSWER e ABSTAIN encerram o episódio. O artigo trata pedido de esclarecimento como uma forma de abstention e avalia a competência em tarefas sequenciais, não apenas em uma resposta isolada.

Implicação para o HERUS: a abstention do HERUS deve ser tratada como uma ação positiva e tipada, não como falha residual. A máquina de autoridade pode usar estados equivalentes a OBSERVE, HOLD, OFFER, ASK/CONFIRM, ABSTAIN e ACT-LOCAL, mas o último deve ser restrito ao vestível e a uma autoridade previamente concedida. A contribuição possível não é inventar abstention, mas combinar abstention sequencial com promoção de memória e autoridade física em um dispositivo pessoal local.

## 8. Xuan et al. (2026), *After Talking with 1,000 Personas: Learning Preference-Aligned Proactive Assistants From Large-Scale Persona Interactions*

Fonte primária: https://arxiv.org/html/2602.04000v1

O artigo observa que aceitação de proatividade depende do que o assistente faz, quando intervém, quanto de iniciativa assume, como comunica e como se adapta ao contexto. Propõe sinais de preferência multidimensionais e adaptação reversível por categoria, em vez de uma única direção global. A abordagem usa interações sintéticas em larga escala e uma etapa de adaptação individual para alinhar o comportamento proativo.

Implicação para o HERUS: timing e iniciativa são parte da inteligência, não apenas UX. A presença ambiente do HERUS já modela hold, oferta única, cooldown e expiração, mas ainda não possui um modelo longitudinal de preferências de intervenção. O caminho original possível é aprender **preferências de silêncio e autoridade** como estado semântico governado e reversível, sem atualizar um modelo inteiro nem inferir consentimento permanente a partir de uma única resposta.

## Refinamento da hipótese

A literatura recente torna inadequada a frase “HERUS inventará abstention” ou “HERUS inventará memória episódico-semântica”. A hipótese mais defensável passa a ser:

> **Um dispositivo pessoal pode operar uma máquina local de transições de autoridade semântica, na qual percepção, memória, intervenção e ação possuem estados e provas distintos; a continuidade pessoal é permitida apenas quando a cadeia de evidência, preferência, validade e autoridade permanece íntegra.**

A originalidade dessa combinação ainda precisa ser estabelecida por uma revisão sistemática mais ampla e por comparação experimental. O próximo passo acadêmico é construir uma taxonomia e uma matriz de comparação, não ainda anunciar um novo paradigma.

## 9. Jiang et al. (2025), *PersonaMem-v2*

Fonte primária: https://arxiv.org/abs/2512.06688

PersonaMem-v2 apresenta um conjunto de dados para personalização implícita com 1.000 interações simuladas, mais de 300 cenários, mais de 20.000 preferências e janelas de contexto de até 128k tokens. O resumo reporta que modelos de fronteira atingem apenas 37–48% em personalização implícita; um Qwen3-4B ajustado chega a 53%, e um sistema de memória agentiva alcança 55% usando uma memória de 2k tokens em vez de todo o histórico de 32k tokens.

Implicação para o HERUS: personalização implícita continua difícil mesmo com modelos grandes. Uma memória compacta pode ser mais eficiente que contexto bruto, mas o trabalho não resolve a soberania física, a promoção autorizada de fatos, conflitos com abstention ou execução local em microcontrolador. O resultado reforça que o HERUS deve medir entendimento de preferências, não apenas recuperação de fatos.

## 10. Zhong et al. (2023), *MemoryBank*

Fonte primária: https://arxiv.org/abs/2305.10250

MemoryBank propõe memória de longo prazo para LLMs, com atualização contínua, recuperação de memórias relevantes e adaptação à personalidade do usuário. O mecanismo de atualização é inspirado na curva de esquecimento de Ebbinghaus, reforçando ou esquecendo memórias conforme tempo e importância. O trabalho demonstra um chatbot de companhia e relata análises qualitativas com diálogos reais e quantitativas com diálogos simulados.

Implicação para o HERUS: esquecimento seletivo é necessário para uma inteligência pessoal sustentável, mas o critério humano de importância não pode ser substituído por decaimento temporal sozinho. No HERUS, expiração pode retirar validade contextual; persistência de uma preferência ou objetivo exige evidência, confirmação, revisão e possibilidade de conflito. A curva de esquecimento é uma inspiração, não uma autoridade.

## Estado da comparação

A revisão já encontrou precedentes para: memória episódico-semântica-lógica, memória de longo prazo, personalização implícita, proatividade alinhada e abstention agentiva. Portanto, não é honesto dizer que o HERUS sozinho inventou esses componentes. O espaço potencialmente original está na composição restrita: uma máquina de transições de autoridade semântica, local, soberana, com memória governada, intervenção mínima, conflito como estado e nenhum poder de execução concedido ao Core externo.

## 11. Kasper et al. (2025), *Wearable AI for on-device frailty assessment*

Fonte primária: https://www.nature.com/articles/s41467-025-67728-y

O trabalho apresenta um wearable com inferência on-device para avaliação de fragilidade baseada em marcha. O resumo relata ensaios in vivo, operação contínua por dez dias e análise longitudinal sem intervenção do usuário, com redução da necessidade de enviar grandes volumes de biosinais para fora do dispositivo. O contexto é clínico e não equivale ao HERUS, mas demonstra que inferência local contínua em wearable pode ser validada em uma tarefa delimitada.

Implicação para o HERUS: a transição entre host e físico precisa medir estabilidade, energia e operação prolongada em workloads específicos. O estudo não prova memória pessoal, conversa, autoridade ou soberania; serve como precedente metodológico de que resultados de wearable devem ser ligados a uma tarefa, população, duração e instrumento declarados.

## 12. Zulfikar, Chan e Maes (2024), *Memoro: Using Large Language Models to Realize a Concise Interface for Real-Time Memory Augmentation*

Fonte primária e descrição do projeto: https://www.media.mit.edu/projects/memoro/overview/

Memoro é um assistente vestível que captura áudio continuamente por headset de condução óssea, transforma conversas em memória estruturada e oferece consultas por voz, lembretes proativos e sumários. A página do projeto relata uma fase CHI 2024 com 20 participantes, aumento de confiança de lembrança e redução de carga de interação, além de uma segunda fase com dez participantes idosos, mais de 140 horas de fala e mais de 460 interações de memória em dez dias. A arquitetura depende de LLM e captura contínua de áudio, portanto não é um substituto direto do HERUS.

Implicação para o HERUS: Memoro confirma o valor potencial de uma memória vestível invisível e a importância de ritmos naturais de uso. Também expõe a diferença estratégica: o HERUS não deve depender de captura remota ou de uma LLM hospedada; sua contribuição precisa ser soberania local, filtragem semântica pré-persistência, privacidade por construção e autoridade física. Os resultados de usuários de Memoro não podem ser transferidos automaticamente para o HERUS, mas justificam um futuro protocolo humano próprio.

## Conclusão da rodada sobre wearables

Há evidência de que wearables podem operar inferência contínua e que assistentes de memória podem gerar valor humano. Não há, nas fontes consultadas até aqui, prova de que um wearable local combine simultaneamente: modelo semântico pessoal contínuo, memória governada por autoridade, abstention, conflito explícito, adaptação reversível, soberania on-wrist e Core externo sem poder de execução. Essa afirmação ainda é uma lacuna provisória, não uma reivindicação de prioridade.

## 13. Zulfikar, Chan e Maes (CHI 2024), publicação primária do Memoro

Fonte primária: https://arxiv.org/html/2403.02135v1

A publicação descreve Memoro como um wearable de aumento de memória em tempo real, com captura de áudio, consultas de voz e um modo Queryless no qual o sistema tenta inferir a necessidade do usuário a partir do contexto. A página HTML organiza o trabalho em desenho do sistema, estudo de usuários, resultados e limitações. O método é importante para o HERUS porque trata concisão da interface, recuperação de lembranças e operação sem tela como problemas de interação, mas sua arquitetura depende de áudio e LLM, enquanto o HERUS precisa ser local e não registrar/transmitir áudio ou conteúdo de terceiros.

Implicação: a avaliação do HERUS precisa separar valor da interface, qualidade da memória e soberania do processamento. Um resultado humano positivo em Memoro não transfere automaticamente para o HERUS; ele sugere, porém, que a experiência deve ser avaliada por carga de interação, confiança de lembrança, timing e qualidade conversacional, além da correção lógica.

## Fontes externas consultadas nesta rodada

- Transformer: https://arxiv.org/abs/1706.03762
- RAG: https://arxiv.org/abs/2005.11401
- EWC: https://www.pnas.org/doi/10.1073/pnas.1611835114
- On-device Training: https://arxiv.org/html/2212.00824v3
- NS-Mem: https://arxiv.org/html/2603.15280v1
- Agentic Abstention: https://arxiv.org/html/2606.28733v1
- Preference-aligned proactive assistants: https://arxiv.org/html/2602.04000v1
- PersonaMem-v2: https://arxiv.org/abs/2512.06688
- MemoryBank: https://arxiv.org/abs/2305.10250
- Wearable AI: https://www.nature.com/articles/s41467-025-67728-y
- Memoro project: https://www.media.mit.edu/projects/memoro/overview/
- Memoro paper: https://arxiv.org/html/2403.02135v1

## 14. Dash et al. (2026), *From Untrusted Input to Trusted Memory: A Systematic Study of Memory Poisoning Attacks in LLM Agents*

Fonte primária: https://arxiv.org/html/2606.04329v1

O trabalho identifica quatro canais de escrita de memória — instrução explícita executada, escrita guiada por prompt de sistema, compactação e experiência convertida em procedimento — e nove vulnerabilidades estruturais. Introduz uma taxonomia de seis classes de ataques e o MPBench. O resumo relata que ataques persistem e que sistemas que escrevem/recuperam memória mais agressivamente são mais exploráveis; também afirma que defesas de prompt injection não cobrem completamente memory poisoning.

Implicação para o HERUS: o problema não é somente filtrar o conteúdo do Core. É governar cada canal de promoção para memória. O desenho atual do HERUS, com candidatos transitórios, autoridade física, proveniência, floor, conflito e quarentena, está alinhado com essa ameaça, mas ainda precisa de um benchmark comparável de poisoning no simulador de vida.

## 15. Xu et al. (2026), *Memory Provenance Laundering in LLM Agents*

Fonte primária: https://arxiv.org/abs/2607.29167

O trabalho define provenance laundering: durante consolidação, uma observação externa pode ser reescrita como se fosse história do usuário ou suporte de workflow, preservando o gatilho de ação e apagando a fonte de baixa confiança. Propõe um firewall de memória que mantém proveniência de plataforma e autoriza chamadas conforme risco da ação e autoridade da memória. O resumo relata que, em sua avaliação, memórias consolidadas vulneráveis alcançam ASR de até 1, enquanto proveniência intacta, confirmação e rótulos de risco bloqueiam ações de alto risco não autorizadas.

Implicação para o HERUS: esta fonte é especialmente próxima da proposta. Portanto, “preservar proveniência” isoladamente não é uma novidade segura. A contribuição possível precisa incluir o ciclo completo local: observação → evidência → memória pessoal → oferta háptica → confirmação → ação, com o Core impedido de executar e com reboot/floor como fronteira adicional. O HERUS também deve tratar a autoridade da memória como não amplificável: uma fonte externa não pode ganhar autoridade só por ser resumida.

## Consequência para o programa de pesquisa

A literatura recente reduziu o espaço de originalidade, mas tornou a hipótese mais forte. O HERUS deve ser comparado diretamente contra poisoning, laundering e memória sem governança. O próximo artefato científico não deve ser apenas um módulo de memória: deve ser um benchmark de **não amplificação de autoridade pessoal** com ataques que tentem converter observação externa, preferência implícita, conflito e contexto vencido em ação local.
