# HERUS — Fontes de pesquisa para inteligência e benchmarks

**Regra de interpretação:** as fontes abaixo orientam desenho de benchmark e arquitetura. Nenhum resultado numérico da literatura é resultado do HERUS.

## ARC-AGI-2

Chollet et al. descrevem o ARC-AGI-2 como uma atualização do ARC voltada a avaliar raciocínio abstrato e resolução de problemas com tarefas novas e uma linha de base humana. O valor para o HERUS não é copiar o domínio visual, mas adotar a separação entre exemplos de treino e tarefas novas, exigindo generalização composicional e não apenas repetição do corpus [1].

## LongMemEval

Wu et al. definem cinco capacidades de memória de longo prazo: extração de informação, raciocínio entre sessões, raciocínio temporal, atualização de conhecimento e abstenção. O benchmark contém 500 perguntas em históricos escaláveis e propõe decompor memória em indexação, retrieval e leitura [2]. Para o HERUS, isso sugere medir separadamente captura seletiva, atualização/confito temporal, retrieval e resposta abstente — sem guardar transcript ou depender de embeddings hospedados.

## Abstenção

Wen et al. organizam abstenção em três perspectivas: answerability da consulta, confiança do modelo na resposta e alinhamento com valores humanos. A decisão de responder pode ser composta por limiares ou regras, e abstenção parcial é diferente de simplesmente ignorar ou reformular a pergunta [3]. O HERUS já possui ausência, contradição, ambiguidade e limites como estados internos; o próximo benchmark deve medir cobertura versus erro e distinguir “sem evidência” de “fora de política”.

## Raciocínio neuro-simbólico

O trabalho DiffLogic descreve um compromisso recorrente: regras simbólicas oferecem precisão, embeddings oferecem eficiência e generalização, e o sistema precisa filtrar o espaço de inferência para equilibrar os dois [4]. Para o HERUS, a implicação é que o VSA não deve ser tratado como prova: ele deve gerar candidatos, enquanto o reasoner verifica fatos, regras, contradições e autoridade.

## Verificação formal de saídas

Proof of Thought usa uma representação intermediária tipada em DSL, um interpretador e um verificador formal; a garantia depende da correção da base de conhecimento e das regras. O ponto útil para o HERUS é a separação explícita entre representação gerada, interpretação tipada, verificação e resposta final [5]. O HERUS pode realizar isso localmente com um IR pequeno e uma prova compacta, sem exigir que um modelo externo seja autoridade.

## Memória modular

O estudo “Memory in the LLM Era” decompõe memória de agentes em extração, gerenciamento, armazenamento e retrieval, incluindo conexão, integração, transformação, atualização e filtragem. Ele também destaca que memória e RAG têm objetivos diferentes: memória preserva informação dependente da interação, enquanto RAG ancora conhecimento externo [6]. O HERUS deve permanecer no caminho de memória pessoal seletiva, com atualização temporal e conflito explícito, e não transformar qualquer armazenamento em “inteligência”.

## Memória escalável como comparador externo

Mem0 apresenta uma arquitetura que extrai, consolida e recupera informação em conversas multi-sessão e compara categorias de memória em LOCOMO. Seus resultados são de um sistema LLM-based e não podem ser transplantados como prova do HERUS; servem apenas para lembrar que custo, latência, atualização temporal e multi-hop devem ser medidos junto com acurácia [7].

## References

[1]: https://arxiv.org/abs/2505.11831 "ARC-AGI-2: A New Challenge for Frontier AI Reasoning Systems"

[2]: https://arxiv.org/abs/2410.10813 "LongMemEval: Benchmarking Chat Assistants on Long-Term Interactive Memory"

[3]: https://doi.org/10.1162/tacl_a_00754 "Know Your Limits: A Survey of Abstention in Large Language Models"

[4]: https://neurips.cc/virtual/2023/poster/71161 "Differentiable Neuro-Symbolic Reasoning on Large-Scale Knowledge Graphs"

[5]: https://arxiv.org/html/2409.17270v2 "Proof of Thought: Neurosymbolic Program Synthesis allows Robust and Interpretable Reasoning"

[6]: https://arxiv.org/html/2604.01707v1 "Memory in the LLM Era: Modular Architectures and Strategies in a Unified Framework"

[7]: https://arxiv.org/abs/2504.19413 "Mem0: Building Production-Ready AI Agents with Scalable Long-Term Memory"

## ProofWriter

ProofWriter separa três tarefas que são úteis para o HERUS: responder e gerar uma prova, enumerar implicações de uma teoria e fazer abdução restrita de um fato ausente. O trabalho usa semântica de mundo aberto, distingue `Unknown` de falso e avalia profundidades de prova e generalização fora da distribuição [8]. Para o HERUS, isso sugere que o benchmark deve exigir não apenas uma resposta, mas também prova verificável, enumeração limitada e uma hipótese explicitamente marcada como hipótese — nunca como fato confirmado.

## SCAN

SCAN é um conjunto de comandos de navegação compostos por primitivas e modificadores como `twice`, `thrice`, `and`, `after`, `around` e `opposite`. Seus splits isolam generalização de comprimento, nova primitiva, novo template e poucos exemplos [9]. O valor para o HERUS é direto: criar splits que escondam uma composição sem esconder seus componentes, medindo generalização sistemática e não memorização de frases.

## StrategyQA

StrategyQA foi desenhado para que os passos necessários não estejam explicitamente escritos na pergunta. O benchmark fornece decomposição e evidência, contém 2.780 exemplos e mede raciocínio implícito multi-hop; o artigo relata 87% humano e aproximadamente 66% para o melhor baseline da época [10]. Para o HERUS, a versão segura deve substituir conhecimento externo não verificado por um domínio pessoal/factory fechado, mas manter a exigência de escolher uma estratégia, decompor o objetivo e indicar quando a evidência não existe.

## References adicionais

[8]: https://arxiv.org/html/2012.13048v2 "ProofWriter: Generating Implications, Proofs, and Abductive Statements over Natural Language"

[9]: https://github.com/brendenlake/SCAN "SCAN tasks for compositional learning"

[10]: https://aclanthology.org/2021.tacl-1.21/ "Did Aristotle Use a Laptop? A Question Answering Benchmark with Implicit Reasoning Strategies"
