# HERUS — Comparação com capacidades de LLM: protocolo e primeiro resultado

**Autor:** Gustavo
**Estado:** comparação offline host-only; baseline LLM local ainda não fornecido
**Commit de referência da etapa:** em preparação
**Escopo:** medir sobreposição de capacidades úteis, não declarar equivalência irrestrita

## 1. Pergunta científica

A pergunta desta etapa não é se o HERUS “parece” uma LLM em uma conversa. A pergunta testável é: **em quais tarefas úteis para uma inteligência pessoal local o HERUS pode igualar, superar ou ficar atrás de um baseline de modelo de linguagem local, sob entradas congeladas, contratos idênticos e métricas explícitas?**

Essa formulação segue a orientação de avaliações holísticas: desempenho não deve ser reduzido a uma única acurácia, e cenários diferentes exigem métricas diferentes para qualidade, calibração, robustez e eficiência [1]. Também segue a disciplina de harnesses reproduzíveis, com tarefas versionadas, prompts/configurações explícitos e suporte a modelos locais [2].

> **Regra de honestidade:** uma pontuação alta em um domínio tipado não autoriza afirmar paridade com linguagem aberta. Uma LLM pode ter maior cobertura lexical e geração livre; o HERUS pode oferecer provas, abstenção e autoridade delimitada. Essas propriedades devem ser reportadas separadamente.

## 2. Protocolo congelado

O contrato completo está em [`research/llm_comparison_protocol_v1.json`](../research/llm_comparison_protocol_v1.json). O protocolo separa tarefas in-distribution, recomposição composicional, OOD linguístico, casos adversariais, groundedness, continuidade, abstenção e eficiência.

| Família | Entrada | Saída comparada | Métrica principal | Estado |
|---|---|---|---|---|
| Semântica controlada em português | enunciado controlado | IR tipada ou rejeição | exact IR, slots, abstention | medido |
| Inferência natural em português | pares anotados | entailment/contradição/desconhecido | macro-F1, risco-cobertura | adaptador pendente |
| Raciocínio composicional | fatos e regras tipados | resposta e prova | exact match, validade da prova | medido em domínio tipado |
| Resposta grounded pessoal | evidência autorizada | texto, IDs e proveniência | precisão de claims suportadas | medido em domínio tipado |
| Geração aberta delimitada | significado e tarefa | texto preservando significado | adequação humana e preservação semântica | não medido |
| Continuidade multi-turno | traços de diálogo | transição e estado | exactness de sequência | medido deterministicamente |
| Abstenção seletiva | casos respondíveis e não respondíveis | resposta ou abstenção | risco@cobertura e falha silenciosa | parcialmente medido |
| Eficiência/localidade | mesma tarefa e ambiente | qualidade e custo | RAM, artefato, latência e rede | host parcial |

Os splits de generalização não serão misturados. COGS demonstra por que um modelo pode obter desempenho quase perfeito dentro da distribuição e cair substancialmente em combinações sistematicamente novas [3]. Por isso, o HERUS terá resultados separados para combinações familiares, componentes familiares em posições novas, profundidade nova, paráfrases controladas e linguagem natural fora da gramática.

## 3. Inventário real do ambiente

O inventário está em [`research/llm_baseline_inventory_v1.json`](../research/llm_baseline_inventory_v1.json). No ambiente atual há PyTorch, mas não há `transformers`, `llama_cpp`, `sentence_transformers`, `datasets` ou `evaluate`, e não há checkpoint de uma LLM instrucional local disponível. O artefato `char_transfer_v1` é um Transformer de caracteres pequeno, com base de 182.016 parâmetros e adaptador de 3.592 parâmetros. Ele demonstrou sinal de transferência em NLL, mas seu próprio README registra amostras imperfeitas e ausência de evidência de conversação, raciocínio geral ou paridade com LLM.

Esse artefato, portanto, **não foi promovido artificialmente a baseline conversacional**. A aquisição de um checkpoint local e de seu runtime, com licença, tokenizer, quantização, hardware e hash registrados, é uma etapa posterior. O produto HERUS continuará sem dependência desse baseline.

O português exige cuidado adicional. ASSIN2 oferece pares anotados para entailment e similaridade em português [4]. Napolab organiza conjuntos públicos com critérios de naturalidade, confiabilidade, anotação humana e generalidade [5]. IberoBench mostra a utilidade de uma cobertura multitarefa para línguas ibéricas, mas também confirma que um benchmark amplo não deve ser confundido com uma única medida de “inteligência” [6].

## 4. Harness offline implementado

O arquivo [`tools/llm_comparison_harness.py`](../tools/llm_comparison_harness.py) executa o mesmo corpus congelado para o roteador tipado do HERUS e uma ablação `similarity_only`. Ele grava apenas resultados tipados, IDs e métricas; não grava áudio, transcrição, embedding, identidade, localização, segredo, chave ou payload de rede.

Um baseline local pode ser fornecido somente por um JSONL offline com `id`, intenção prevista, decisão de abstenção, necessidade de confirmação e IDs de evidência. O harness rejeita IDs duplicados, campos proibidos, esquema incompleto, tipos incorretos e IDs de evidência acima do limite. Ele não executa o arquivo e não chama APIs externas. Sem esse artefato, o resultado permanece explicitamente `not_supplied`.

A avaliação de abstenção segue o princípio de predição seletiva: o sistema pode rejeitar exemplos de baixa confiança, e qualidade deve ser vista como compromisso entre risco e cobertura [7]. Não serão publicados números de “acurácia” que removam casos abstidos do denominador.

Para geração de texto, similaridade lexical não será tratada como verdade. Avaliações semânticas recentes distinguem preservação do significado de sobreposição de n-gramas e mostram a necessidade de validação semântica e humana [8]. Para o HERUS, a primeira camada será um oráculo tipado quando houver significado esperado; a segunda será avaliação humana cega para adequação, clareza e utilidade.

## 5. Resultado executado

A execução atual utilizou nove casos de teste do benchmark `intent_memory`, com treino separado e sem baseline LLM local:

| Sistema | Intent accuracy | Abstention accuracy | Confirmation safety | Evidence accuracy |
|---|---:|---:|---:|---:|
| HERUS typed router | 1,000000 | 1,000000 | 1,000000 | 1,000000 |
| Ablação similarity-only | 0,444444 | 0,888889 | 0,888889 | 0,888889 |
| LLM local | não medido | não medido | não medido | não medido |

A diferença observada entre HERUS e a ablação é evidência de que os overrides tipados e as regras de segurança melhoram esse corpus específico. **Não é evidência de superioridade sobre uma LLM**, porque nenhuma LLM foi executada com as mesmas entradas. Também não é evidência de linguagem aberta: o corpus é um conjunto controlado de intenções e memória.

O redteam [`tools/test_llm_comparison_harness_redteam.py`](../tools/test_llm_comparison_harness_redteam.py) matou semanticamente **5/5 mutantes críticos**: bypass de campo proibido, IDs duplicados, esquema incompleto, tipos booleanos e limite de IDs de evidência. A prova global terminou com `ALL INVARIANTS HOLD`; os gates históricos de proveniência também voltaram a passar depois da atualização dos digests.

## 6. Critérios para uma futura afirmação de paridade

Uma afirmação de paridade somente poderá ser feita por família de tarefa, com corpus congelado, contagem de casos, intervalo de incerteza quando houver amostragem, resultados de todos os casos, abstenções no denominador e comparação com baseline local declarado. Paridade em uma família não implica paridade no sistema inteiro.

A afirmação de paridade sistêmica permanece bloqueada até que existam evidências comparáveis para linguagem aberta, geração, inferência, groundedness, continuidade, segurança e eficiência. Qualquer resultado de hardware também permanecerá pendente até ensaios físicos; os números desta etapa são host-only.

> **Conclusão atual:** o HERUS possui uma comparação offline reproduzível e já demonstra vantagem da arquitetura tipada sobre sua ablação em um pequeno corpus de memória. O baseline de LLM local ainda não está disponível. A hipótese de que o HERUS possa ser competitivo em segurança, proveniência, abstenção e continuidade é plausível e testável; a hipótese de equivalência em cobertura e geração aberta continua sem evidência.

## Referências

[1]: https://arxiv.org/abs/2211.09110 “Holistic Evaluation of Language Models”
[2]: https://github.com/EleutherAI/lm-evaluation-harness “Language Model Evaluation Harness”
[3]: https://aclanthology.org/2020.emnlp-main.731/ “COGS: A Compositional Generalization Challenge Based on Semantic Interpretation”
[4]: https://huggingface.co/datasets/nilc-nlp/assin2 “ASSIN2 dataset card”
[5]: https://github.com/ruanchaves/napolab “Natural Portuguese Language Benchmark”
[6]: https://aclanthology.org/2025.coling-main.699/ “IberoBench: A Benchmark for LLM Evaluation in Iberian Languages”
[7]: https://aclanthology.org/2021.acl-long.84/ “The Art of Abstention: Selective Prediction and Error Regularization for Natural Language Processing”
[8]: https://aclanthology.org/2025.acl-long.477/ “Semantic-Eval: A Semantic Comprehension Evaluation Framework for Large Language Models Generation without Training”
[9]: https://proceedings.neurips.cc/paper_files/paper/2024/file/047c84ec50bd8ea29349b996fc64af4b-Paper-Conference.pdf “Overcoming Common Flaws in the Evaluation of Selective Classification Systems”
