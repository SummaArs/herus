# HERUS — Investigação acadêmica: pergunta, matriz e critério de novidade

**Rodada:** 1.  
**Data de consulta:** 21 de agosto de 2026.  
**Estado:** síntese provisória; nenhuma prioridade científica é reivindicada.

## 1. Pergunta científica

A pergunta central desta investigação é:

> **Um dispositivo local, com recursos limitados, pode manter um modelo semântico pessoal contínuo por meio de uma máquina explícita de transições de autoridade — separando percepção, candidato, memória, recuperação, intervenção e ação — e preservar utilidade, privacidade, conflito, validade e recuperação segura sem depender de uma LLM hospedada ou de uma memória textual indiscriminada?**

A pergunta é mais estreita que “criar uma AGI no pulso”. Ela pode ser falsificada. O HERUS falha se não conseguir preservar fatos autorizados, abster-se em conflitos, rejeitar dados stale, não reviver autoridade após reboot, manter custo dentro do envelope do alvo ou oferecer utilidade superior a uma linha de base de recuperação sem governança.

## 2. Matriz da literatura consultada

| Trabalho | Contribuição primária | O que já está estabelecido | Lacuna relevante para o HERUS |
|---|---|---|---|
| Vaswani et al., 2017 [1] | Transformer baseado exclusivamente em attention | É possível trocar a unidade central de computação de recorrência/convolução por attention | HERUS precisa definir uma unidade nova de continuidade pessoal, não apenas adicionar attention |
| Lewis et al., 2020 [2] | RAG com memória paramétrica e índice não paramétrico | Recuperação externa melhora acesso, atualização e especificidade do conhecimento | Recuperar não prova autoridade, validade atual, conflito ou direito de agir |
| Kirkpatrick et al., 2017 [3] | EWC para reduzir esquecimento catastrófico | Plasticidade pode ser separada da estabilidade de conhecimento | HERUS precisa governar atualização pessoal sem depender de treinamento paramétrico amplo |
| Zhu et al., 2024 [4] | Revisão de treinamento on-device | Recursos, memória, heterogeneidade e backpropagation limitam aprendizado no dispositivo | O caminho principal pode ser estado semântico compacto e governado, não treino de LLM |
| Jiang et al., 2026 [5] | Memória neuro-simbólica episódica, semântica e lógica | Essa decomposição já possui precedente recente | Novidade não pode ser alegada apenas pela separação de camadas |
| Luo et al., 2026 [6] | Agentic abstention como POMDP com ANSWER, ABSTAIN e ACT | Abster-se é uma competência agentiva e sequencial mensurável | HERUS pode conectar abstention a promoção de memória e autoridade física |
| Xuan et al., 2026 [7] | Preferências multidimensionais para proatividade e adaptação reversível | Timing, iniciativa, comunicação e contexto afetam aceitação | HERUS pode representar preferências de silêncio/autoridade como estado semântico local |
| Jiang et al., 2025 [8] | PersonaMem-v2 para personalização implícita e memória agentiva | Personalização implícita continua difícil mesmo com modelos grandes | HERUS precisa avaliar compreensão de preferências e não só recuperação |
| Zhong et al., 2023 [9] | MemoryBank com atualização e esquecimento inspirados em Ebbinghaus | Memória de longo prazo pode reforçar/esquecer experiências | Decaimento temporal não substitui autoridade, revisão, conflito ou proveniência |
| Kasper et al., 2025 [10] | Inferência contínua on-device em wearable clínico | Wearables podem executar inferência local prolongada em tarefa delimitada | Não demonstra memória pessoal, conversa, autoridade ou soberania |
| Zulfikar et al., 2024 / Memoro [11] | Assistente vestível de memória com captura contínua e estudo de usuários | Memória vestível pode reduzir carga e aumentar confiança de lembrança | HERUS precisa oferecer soberania local sem áudio bruto, LLM hospedada ou Core executor |

## 3. Hipótese de paradigma, ainda provisória

A hipótese de trabalho será chamada **Authority-Governed Semantic Continuity (AGSC)**, ou **Continuidade Semântica Governada por Autoridade**. O nome não é uma reivindicação de prioridade; é um rótulo para tornar a hipótese testável.

A AGSC não define a inteligência pessoal como geração contínua de texto nem como recuperação de trechos. Ela define a unidade central como uma transição entre estados semânticos com autoridade explícita:

```text
observação → candidato → evidência autorizada → memória atual
                         ↓                 ↓
                    descarte/conflito   expiração/supersession
                                           ↓
                            recuperação → intervenção limitada → ação autorizada
```

Cada seta deve carregar condições diferentes. Uma observação pode ser útil sem ser memória. Uma memória pode ser recuperável sem autorizar intervenção. Uma intervenção pode ser oferecida sem autorizar ação. Conflito não é um erro de consulta: é um estado de conhecimento que deve bloquear seleção arbitrária.

## 4. Critérios de novidade

A proposta não será considerada nova apenas porque usa palavras como “semântico”, “neuro-simbólico”, “memória” ou “soberano”. Para sustentar uma contribuição, será necessário demonstrar simultaneamente:

| Critério | Teste de honestidade |
|---|---|
| Unidade arquitetural | A transição de autoridade é o mecanismo central, não um rótulo sobre RAG |
| Composição | Memória, abstention, proatividade, conflito e ação são avaliados em uma cadeia única |
| Soberania | A política continua executável localmente e o Core não possui autoridade de ação |
| Recursos | O desenho tem envelope de memória, energia e latência compatível com o alvo declarado |
| Falsificabilidade | Existem baselines e ataques que podem mostrar que a hipótese falha |
| Reprodutibilidade | Estados, entradas, mutações e decisões podem ser reproduzidos sem dados privados |
| Distinção | A contribuição é comparada diretamente com memória agentiva, NS-Mem, RAG e abstention |
| Evidência humana | Utilidade, intrusão e compreensão são avaliadas separadamente do host-side |

## 5. Métricas que não confundem contagem com inteligência

O próximo protótipo acadêmico deve medir propriedades comportamentais, não apenas o número de testes:

| Métrica | Pergunta |
|---|---|
| Precisão de promoção | Quantos candidatos viram memória somente quando autorizados? |
| Taxa de memória falsa | Quantas memórias persistem sem evidência suficiente? |
| Preservação de conflito | Quantos conflitos permanecem contraditórios sem seleção falsa? |
| Resistência a stale facts | Quantos fatos vencidos ou abaixo do floor são rejeitados? |
| Precisão de autoridade | Quantas ações ocorreram dentro do escopo concedido? |
| Abstention apropriada | O sistema abstém quando deveria e continua quando há autorização suficiente? |
| Custo de intervenção | Quantas ofertas, vibrações e interrupções são geradas por unidade de utilidade? |
| Vazamento | Quanto áudio, texto, identidade ou conteúdo deixa o vestível? Meta: zero no log de produto |
| Recuperação | O reboot retorna a quarentena correta sem reviver autoridade? |
| Custo físico | Memória, latência e energia medidos no alvo, nunca estimados como resultado físico |

## 6. Baselines obrigatórios

Uma comparação séria precisa incluir pelo menos quatro baselines host-side: sem memória; recuperação do último fato; recuperação por similaridade sem conflito; e memória governada sem camada de autoridade separada. O HERUS só terá evidência de contribuição se a AGSC reduzir memória falsa, ações indevidas e intrusão sem destruir a recuperação de fatos autorizados.

A comparação não pode usar uma LLM hospedada de terceiro como parte do produto. Modelos acadêmicos podem aparecer como referências experimentais em uma avaliação, desde que não sejam apresentados como tecnologia do HERUS nem introduzam dependência arquitetural incompatível com a soberania local.

## 7. Veredito provisório

A investigação já eliminou três reivindicações fáceis, mas falsas: o HERUS não inventou attention; não inventou memória episódico-semântica; e não inventou abstention. O espaço promissor está na **governança da continuidade pessoal**, especialmente na cadeia entre evidência, memória, intervenção e ação sob soberania on-wrist.

Ainda não há base para chamar AGSC de novo paradigma. Há uma pergunta clara, uma matriz de comparação e uma hipótese falsificável. O próximo passo é ampliar a revisão sistemática, extrair definições e métricas dos trabalhos, e então implementar baselines e o primeiro protótipo comparável no simulador do HERUS.

## Referências

[1]: https://arxiv.org/abs/1706.03762 "Attention Is All You Need"

[2]: https://arxiv.org/abs/2005.11401 "Retrieval-Augmented Generation for Knowledge-Intensive NLP Tasks"

[3]: https://www.pnas.org/doi/10.1073/pnas.1611835114 "Overcoming catastrophic forgetting in neural networks"

[4]: https://arxiv.org/html/2212.00824v3 "On-device Training: A First Overview on Existing Systems"

[5]: https://arxiv.org/html/2603.15280v1 "Advancing Multimodal Agent Reasoning with Long-Term Neuro-Symbolic Memory"

[6]: https://arxiv.org/html/2606.28733v1 "Agentic Abstention: Do Agents Know When to Stop Instead of Act?"

[7]: https://arxiv.org/html/2602.04000v1 "After Talking with 1,000 Personas: Learning Preference-Aligned Proactive Assistants"

[8]: https://arxiv.org/abs/2512.06688 "PersonaMem-v2"

[9]: https://arxiv.org/abs/2305.10250 "MemoryBank"

[10]: https://www.nature.com/articles/s41467-025-67728-y "Wearable AI for on-device frailty assessment"

[11]: https://www.media.mit.edu/projects/memoro/overview/ "Memoro: Wearable Personal Memory Assistant"
