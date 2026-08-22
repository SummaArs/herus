# Notas de pesquisa — protocolo de comparação HERUS/LLM v1

**Data da consulta:** 2026-08-22  
**Escopo:** avaliação pré-hardware, local-first, sem usar LLM hospedada como componente do HERUS

## Fontes consultadas

### HELM

A página primária do HELM descreve uma avaliação holística que separa cenários de uso e métricas, em vez de reduzir a análise a uma única acurácia. O resumo relata sete métricas — accuracy, calibration, robustness, fairness, bias, toxicity e efficiency — aplicadas aos cenários centrais quando possível, além de avaliações direcionadas. O trabalho também enfatiza condições padronizadas e disponibilização pública de prompts e completions para transparência e reprodutibilidade.

Fonte: https://arxiv.org/abs/2211.09110  
Título: *Holistic Evaluation of Language Models*, Liang et al., publicado em TMLR, 2023.

**Decisão para o HERUS:** a matriz deve manter eixos independentes para qualidade semântica, calibração/abstenção, robustez, groundedness, privacidade/autoridade, eficiência e continuidade. Um score agregado não será gate de equivalência.

### LM Evaluation Harness

O repositório oficial do LM Evaluation Harness declara suporte a benchmarks acadêmicos, modelos locais, adaptadores e prompts públicos. A documentação atual também descreve backends separados e avaliação de modelos carregados de caminho local, além de tarefas de resposta direta e múltiplas configurações de few-shot.

Fonte: https://github.com/EleutherAI/lm-evaluation-harness  

**Decisão para o HERUS:** reutilizar a disciplina de tarefas versionadas, prompts congelados, configuração explícita, seed e artefatos de resultado. Não importar automaticamente o harness para o firmware: o HERUS precisa de um adaptador host-only que compare saídas tipadas e texto gerado sem inserir dependência neural no vestível.

### COGS

A fonte primária descreve um desafio de parsing semântico em um fragmento de inglês com gaps sistemáticos. O resumo reporta desempenho in-distribution de 96–99% para modelos avaliados e generalização de 16–35%, com sensibilidade a seed. O valor metodológico é separar combinações novas de estruturas familiares de frases simplesmente novas dentro da distribuição.

Fonte: https://aclanthology.org/2020.emnlp-main.731/  

**Decisão para o HERUS:** criar splits separados para in-distribution, recomposição composicional, paráfrase não observada, vocabulário novo e rejeição de segurança. Os casos controlados atuais não podem ser chamados de linguagem aberta.

### IberoBench e português

O IberoBench é descrito como benchmark multitarefa para línguas ibéricas, construído sobre o LM Evaluation Harness. A página da ACL informa 62 tarefas divididas em 179 subtarefas e avaliação em configurações zero-shot e five-shot para modelos existentes. O trabalho reforça a necessidade de cobrir várias capacidades em português europeu e outras línguas ibéricas, sem depender de uma única tarefa.

Fonte: https://aclanthology.org/2025.coling-main.699/  

**Decisão para o HERUS:** usar benchmarks portugueses públicos como comparadores externos, mas separar tarefas compatíveis com o HERUS — inferência textual, paráfrase e classificação — de tarefas que não medem o objetivo principal, como conhecimento escolar ou domínio jurídico. Os resultados externos serão citados como comparadores publicados, não como reimplementação própria.

### ASSIN2

A ficha pública do dataset informa 9.450 pares no total, divididos em treino, validação e teste, com julgamento de entailment e relatedness score em português. Os exemplos exibidos são pares de frases anotados para inferência textual e similaridade semântica.

Fonte: https://huggingface.co/datasets/nilc-nlp/assin2  

**Decisão para o HERUS:** ASSIN2 é um candidato a avaliação externa de compreensão semântica, mas o compilador atual não pode receber diretamente frases livres e fingir que as resolve. Deve ser usado em uma camada adaptadora explicitamente marcada como “fora da cobertura atual” até existir um parser de paráfrases e um oráculo de resposta compatível.

### Natural Portuguese Language Benchmark (Napolab)

O repositório do Napolab descreve uma coleção de datasets em português para avaliação de LLMs e explicita critérios de naturalidade, confiabilidade, disponibilidade pública, anotação humana e generalidade. A coleção inclui ASSIN/ASSIN2, FaQuAD-NLI, HateBR, PorSimplesSent e outros conjuntos, com métricas como accuracy, F1 e correlação de Pearson.

Fonte: https://github.com/ruanchaves/napolab  

**Decisão para o HERUS:** priorizar subconjuntos com anotação humana e baixo risco de vazamento para medir entendimento e abstenção. Não misturar métricas de classificação com uma métrica de “inteligência geral”; cada tarefa permanece separada.

### Portuguese-bench

O repositório adaptado para português descreve tarefas como ASSIN2 RTE/STS, FaQuAD-NLI, compreensão de leitura, ENEM, detecção ofensiva e sentimento. Ele informa suporte a modelos locais, prompts few-shot, resposta direta, controle de temperatura e codificação UTF-8.

Fonte: https://github.com/surus-lat/portuguese-bench  

**Decisão para o HERUS:** o projeto pode servir como referência de inventário e formato para baselines locais. A inclusão de APIs fechadas ou backends hospedados é incompatível com a restrição do produto e não deve entrar no caminho de execução do HERUS.

## Protocolo proposto

A comparação deve ser feita por tarefa e regime, não por uma alegação única de paridade. Para cada tarefa, registrar versão do corpus, prompt, modelo ou commit do HERUS, seed, orçamento, temperatura quando aplicável, cobertura, abstentions, erros, confiança e artefato bruto sanitizado. Para modelos neurais locais, registrar checkpoint, quantização, backend e hardware de execução. Para o HERUS, registrar commit, toolchain C11, fonte do conhecimento, orçamento e status de proveniência.

| Eixo | Métrica primária | Métrica de segurança | Estado inicial |
|---|---|---|---|
| Entendimento aberto em português | exact match/F1 por tarefa | risco-cobertura e abstenção | não implementado |
| Paráfrase e entailment | accuracy/F1 ou Pearson conforme o dataset | erro em negação e contradição | adaptador a definir |
| Composição | exact match por split e profundidade | preservação de proof roots | parcialmente medido |
| Geração | adequação humana e preservação semântica | taxa de alegação sem suporte | não medido |
| Groundedness | cobertura de evidência autorizada | unsupported-claim rate | domínio tipado |
| Continuidade | exactness de transições | replay, scrub e autoridade | medido em protocolo determinístico |
| Eficiência | tokens/bytes, RAM, latência | ausência de rede e retenção | host parcial |

A unidade de comparação precisa ser explicitamente assimétrica: uma LLM pode gerar texto aberto, enquanto o HERUS pode oferecer provas, abstenção e autoridade limitada. Não é válido declarar vitória global somando tarefas incompatíveis, nem declarar equivalência porque ambos retornaram uma frase. O objetivo é medir quais capacidades úteis do HERUS se aproximam das de uma LLM e onde a arquitetura continua inferior.

## Limites e decisões negativas

Não usar uma LLM hospedada para gerar rótulos, julgar saídas ou alimentar o HERUS. Para avaliação aberta, preferir oráculos determinísticos, rótulos humanos existentes e avaliação humana cega previamente especificada. Não baixar e executar checkpoints ou código sem verificar licença, hash, dependências e compatibilidade com a política local. Nenhuma métrica de benchmark externo será apresentada como medição do hardware do pulso.

A hipótese de trabalho é que o HERUS pode superar uma LLM em consistência de estado, proveniência, abstenção e autoridade no seu domínio autorizado, mas provavelmente ficará atrás em cobertura lexical, conhecimento aberto e geração livre até que uma camada neural local seja adicionada e medida. Essa hipótese deve ser tratada como previsão a ser falsificada, não como conclusão.

## Métricas adicionais verificadas

### Abstenção seletiva

O artigo *The Art of Abstention* formula predição seletiva como a possibilidade de rejeitar exemplos de baixa confiança e avalia conjuntamente qualidade da predição e eficácia do estimador de confiança, incluindo curvas risco–cobertura. A fonte também discute o compromisso entre precisão e cobertura e a utilidade da abstenção para cascatas de modelos.

Fonte: https://aclanthology.org/2021.acl-long.84/  

O trabalho *Overcoming Common Flaws in the Evaluation of Selective Classification Systems* diferencia avaliação em um ponto operacional de avaliação multi-limiar. Ele define risco como falha entre previsões aceitas e cobertura como a fração aceita, e propõe AUGRC para agregar o risco generalizado de falhas silenciosas ao longo dos limiares. A implicação para o HERUS é registrar pelo menos curvas de risco–cobertura e um ponto operacional explícito; não esconder baixa cobertura atrás de acurácia perfeita em um subconjunto fácil.

Fonte: https://proceedings.neurips.cc/paper_files/paper/2024/file/047c84ec50bd8ea29349b996fc64af4b-Paper-Conference.pdf  

**Decisão para o HERUS:** a métrica principal de segurança será `risk_at_coverage` em coberturas pré-registradas, acompanhada por uma curva completa e por `unsupported_claim_rate`. Não usar uma AUC sem declarar a definição de risco e sem separar falha detectada de falha silenciosa.

### Avaliação semântica da geração

O artigo *Semantic-Eval* propõe uma avaliação training-free de texto gerado baseada em similaridade semântica entre textos, ponderação por estrutura de sentenças e um modelo NLI para reduzir vieses de relação semântica. A fonte relata avaliação em oito datasets e comparação com métricas n-gram, métricas baseadas em BERT e modelos avaliadores. O princípio aproveitável para o HERUS é que preservação semântica deve ser avaliada separadamente de sobreposição lexical e fluência.

Fonte: https://aclanthology.org/2025.acl-long.477/  

**Decisão para o HERUS:** qualquer futuro gerador local será avaliado em duas camadas: primeiro por um oráculo tipado/determinístico quando o significado esperado existir; depois por avaliação humana cega para adequação, clareza e utilidade. Métricas automáticas de similaridade não serão usadas como prova de verdade nem como substituto da anotação humana.
