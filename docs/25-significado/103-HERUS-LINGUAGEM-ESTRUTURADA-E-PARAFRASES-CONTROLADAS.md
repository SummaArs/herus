# HERUS — Linguagem Estruturada e Paráfrases Controladas

**Autor:** Gustavo
**Estado:** implementação host-only em C11; hardware, fala humana e interação acústica continuam pendentes
**Comando de prova:** `./prove.sh --quiet`
**Escopo:** expansão conservadora do compilador semântico; não é compreensão aberta nem uma LLM

## Pergunta técnica

A pergunta desta etapa é estreita e falsificável: **o compilador semântico pode aceitar algumas paráfrases portuguesas previamente delimitadas sem perder o significado intermediário tipado, a abstenção e as fronteiras de autoridade?** A resposta medida nesta revisão é positiva para o contrato congelado de 14 casos OOD de linguagem e negativa para qualquer extrapolação além desse contrato.

A motivação vem de uma distinção central na literatura de generalização. O COGS separa desempenho dentro da distribuição de gaps sistemáticos, como novas combinações de estruturas conhecidas; os resultados reportados para modelos avaliados mostram que alta acurácia in-distribution pode coexistir com queda substancial nos gaps [1]. O gSCAN leva a questão para um contexto grounded, no qual a mesma composição linguística precisa ser interpretada em relação ao estado situado [2]. O CFQ, por sua vez, maximiza a divergência entre compostos de treino e teste enquanto mantém pequena a divergência entre átomos, tornando explícito que frases novas não são suficientes para medir recomposição [3].

O HERUS não reproduz esses benchmarks nem reivindica seus resultados. Ele adota somente o princípio metodológico: separar formas familiares, combinações reservadas e rejeições de segurança. A matriz de paridade do projeto continua marcando a compreensão de linguagem aberta como **não medida**; o resultado desta etapa é evidência de linguagem controlada, não uma comparação com uma LLM [4] [5].

## O que foi alterado

O arquivo [`firmware/core/semantic_compiler.c`][7] continua sendo um compilador de gramática controlada. A expansão adiciona aliases lexicais e algumas estruturas de pergunta sem introduzir similaridade, geração probabilística ou correção automática de frases. A entrada é observada transitoriamente e o resultado contém somente estado tipado e IDs numéricos; o texto original não é armazenado no `sc_unit_t`.

A alteração também passou a reconhecer o verbo plural controlado `possuem` como a mesma relação canônica de `possui`. Isso é importante para que o caso adversarial `O que é que todos possuem?` seja rejeitado pelo **gate de quantificação**, e não por uma falha incidental de reconhecimento verbal. O redteam confirmou essa causalidade: quando o gate é removido, o caso quantificado chega a uma interpretação que a suíte derruba.

## Contrato de formas aceitas

A tabela descreve formas aceitas pelo parser atual. “Entidade” significa um único token lexemático resolvido por hash legado ou por registro versionado; não significa que o sistema tenha compreensão lexical aberta ou conhecimento de mundo.

| Unidade | Formas controladas aceitas | Saída e autoridade |
|---|---|---|
| Fato de posse | `entidade possui entidade`, `entidade tem entidade` e a variante verbal controlada `entidade possuem entidade` | `sr_fact_t`; requer confirmação antes de entrar no diálogo |
| Fato locativo | `entidade está em entidade`, `entidade esta em entidade`, `entidade fica em entidade`, com `em`, `no` ou `na` | predicado canônico `estar_em`; mutação sujeita a confirmação |
| Fato negado | relação controlada precedida por `não` ou `nao`, por exemplo `Gustavo não fica na casa.` | `negated = 1`; a negação não é descartada |
| Pergunta de posse | `O que entidade possui?`, `O que entidade tem?`, `O que entidade possuem?` | consulta somente leitura, com objeto variável |
| Pergunta interposta | `O que é que entidade possui?`, `O que é que entidade tem?`, `O que é que entidade pode?` | consulta somente leitura; `é` e `e` são aliases explícitos nessa posição |
| Pergunta locativa | `Onde está entidade?`, `Onde esta entidade?`, `Onde fica entidade?` | consulta somente leitura ao predicado `estar_em` |
| Pergunta sim/não | uma relação controlada exata seguida de `?`, por exemplo `Gustavo tem caderno?` | consulta tipada; a pontuação não a transforma em fato |
| Quantificação | `todos`, `todas`, `todo`, `toda` e `qualquer` | não é interpretada como entidade concreta; resulta em rejeição/abstenção |

As formas positivas e negativas foram escolhidas para testar equivalência semântica, não para afirmar cobertura do português. A presença de `fica`, `na`, `possuem` ou da expressão interposta `é que` não libera outras flexões, advérbios, elipses ou construções conversacionais. Uma cauda fora da gramática continua sendo erro; por exemplo, uma frase que acrescente contexto não declarado não é silenciosamente truncada para fabricar um fato.

## Representação e normalização

O parser reconhece pontuação ASCII limitada (`.`, `,`, `?`, `;` e `:`), espaços ASCII (`espaço`, tabulação, retorno de carro e nova linha) e tokens de tamanho limitado. A comparação de letras só aplica redução de caixa aos bytes ASCII `A`–`Z`. Os acentos portugueses são suportados apenas quando aparecem em aliases escritos explicitamente no código, como `esta`/`está`, `nao`/`não` e `e`/`é`; não existe normalização geral de Unicode, remoção de diacríticos ou segmentação linguística.

| Limite do contrato C11 | Valor |
|---|---:|
| Entrada máxima | 192 bytes |
| Número máximo de tokens | 24 |
| Lexema máximo | 32 bytes |
| Pontuação reconhecida | 5 sinais ASCII |
| Estado textual persistido no IR | 0 bytes de sentença |
| Unidade semântica de saída | enumeração tipada e campos numéricos |

Sem um resolver, os lexemas são convertidos localmente por FNV-1a de 32 bits dobrado para 16 bits. O hash é determinístico, mas não injetivo. Por isso o compilador detecta colisões entre lexemas diferentes dentro da mesma frase e abstém-se. Com um resolver, os símbolos passam por handles de registro versionado; falhas de autorização, colisão, versão e capacidade permanecem erros explícitos. Nenhum desses mecanismos transforma o hash em identidade criptográfica ou em compreensão lexical.

## Segurança semântica e autoridade

A expansão preserva a divisão entre **interpretar**, **propor** e **executar**. Uma consulta não recebe autorização de escrita; um fato ou regra exige confirmação explícita; um objetivo gera apenas uma proposta limitada; e a unidade de rejeição não produz efeito de estado. A ponte `sc_apply_dialogue()` repete a defesa verificando `status` e `exact_parse` antes de chamar o diálogo. Assim, ampliar uma paráfrase não amplia a autoridade do vestível.

Os termos associados a áudio, transcrição, embeddings, localização, senha e chave continuam bloqueados antes da produção do IR. Linguagem de incerteza não suportada, imperativos livres e texto de injeção não são “corrigidos” por aproximação. O comportamento desejado é fail-closed: o compilador prefere retornar erro a adivinhar uma intenção, descartar uma palavra relevante ou promover linguagem em autoridade.

A literatura de groundedness mostra por que plausibilidade verbal não é evidência suficiente para uma resposta fundamentada [8]. O HERUS mantém essa separação: a expansão linguística só produz uma estrutura canônica; prova, memória autorizada, proveniência e política de apresentação continuam sendo responsabilidades de camadas posteriores. A avaliação holística do HELM também motiva separar capacidade, robustez, calibração, toxicidade e eficiência em vez de reduzir o sistema a uma única métrica [4].

## Evidência OOD reproduzível

A suíte C11 [`firmware/core/test_semantic_compiler_language_ood.c`][9] contém os casos congelados desta etapa. Ela testa aliases locativos, perguntas locativas, pergunta interposta, consultas sim/não, rejeição de quantificadores, negação e rejeições de linguagem não suportada.

| Classe | Casos | Resultado observado |
|---|---:|---:|
| Formas OOD aceitas e preservadas no IR | 8 | 8/8 |
| Quantificação rejeitada | 2 | 2/2 |
| Incerteza, incompletude e imperativo fora do escopo | 3 | 3/3 |
| Negação com alias locativo | 1 | 1/1 |
| **Total** | **14** | **14/14; 0 falhas** |

A saída reproduzível da suíte é:

```text
SEMANTIC LANGUAGE OOD: 14 pass, 0 fail
```

O alvo `semantic-language-ood` do [`firmware/Makefile`][10] usa C11 estrito, `-Wall -Wextra -Werror` e o mesmo conjunto de fontes do alvo semântico existente. O `prove.sh` executa a suíte depois do compilador e do benchmark semântico, grava o log em `/tmp/herus_semantic_language_ood.log` e verifica a linha de resultado esperada.

## Frente adversarial GAN

A parte sabotadora está em [`tools/test_semantic_compiler_language_ood_redteam.py`][11]. Para cada mutação, o harness altera uma única defesa no arquivo C, recompila com C11 estrito e executa a mesma suíte. Portanto, um mutante não é contado como morto apenas por erro de compilação: ele precisa sobreviver à compilação e ser semanticamente detectado pela suíte.

| Mutante | Controle removido | Critério de morte |
|---|---|---|
| `locative-alias-bypass` | alias `fica` deixa de mapear para a relação locativa canônica | o IR locativo positivo falha |
| `interposed-question-bypass` | forma `O que é que ...` deixa de usar as posições interpostas | consultas interpostas falham |
| `quantifier-bypass` | tokens quantificados podem virar constantes hashed | os casos com `todos`/`qualquer` deixam de ser rejeitados |
| `negation-bypass` | negação explícita deixa de ser preservada | o caso negativo perde `negated = 1` |
| `question-mark-bypass` | `?` deixa de selecionar a rota de consulta | uma pergunta sim/não pode ser tratada como fato |

O resultado observado é:

```text
SEMANTIC LANGUAGE OOD REDTEAM: 5/5 critical mutants killed
```

Esse resultado tem uma interpretação limitada, mas útil: os cinco controles são necessários para os casos escolhidos e a suíte não passa silenciosamente quando cada controle é removido individualmente. Ele não prova resistência contra todas as paráfrases, todos os ataques ou todas as colisões de hash. O princípio GAN aqui é operacional: cada melhoria crítica vem acompanhada de uma parte sabotadora que tenta remover exatamente o controle que a melhoria poderia enfraquecer.

## Relação com a matriz de paridade

A matriz [`research/llm_parity_matrix_v1.json`][12] deve continuar distinguindo capacidade específica de equivalência geral. Esta etapa melhora a evidência da dimensão `language_understanding`, mas não muda seu status de linguagem aberta não medida.

| Dimensão | Estado após esta etapa | Interpretação correta |
|---|---|---|
| Linguagem controlada | medido no contrato OOD | 14/14 nas formas congeladas e 5/5 controles adversariais |
| Linguagem aberta em português | não medido | não há acurácia, cobertura ou risco-cobertura de domínio aberto |
| Geração aberta | não medido | paráfrase de entrada não é geração livre |
| Groundedness | medido somente no domínio tipado | não há benchmark de factualidade aberta |
| Continuidade multi-turno | medido em protocolo determinístico | não é conversa humana livre |
| Hardware, voz e percepção | pendentes | nenhum resultado de ESP32, microfone, WER, latência física ou percepção háptica |

O LM Evaluation Harness enfatiza tarefas, prompts e condições reproduzíveis para comparar modelos locais [5]. O HERUS adota a disciplina de versionar casos e separar oráculo estrutural de julgamento humano, mas não declara que seu compilador seja um modelo avaliado por esse harness. Da mesma forma, TruthfulQA demonstra a necessidade de testar veracidade contra perguntas adversariais, inclusive porque escala de modelo não garante verdade por si só [6]. Nenhum caso desta etapa mede factualidade aberta ou veracidade no sentido de TruthfulQA.

## Conclusão honesta

A expansão é um avanço real e mensurável: o HERUS agora preserva uma pequena família adicional de paráfrases portuguesas, perguntas locativas, perguntas interpostas, consulta sim/não e negação locativa, enquanto rejeita quantificação ambígua e linguagem fora do contrato. O resultado é reproduzível no host, compilado em C11 estrito e protegido por cinco mutações semanticamente mortas.

O avanço **não** torna o HERUS equivalente a uma LLM. Ele não mede compreensão aberta, vocabulário livre, fluência, diversidade de geração, factualidade de mundo, fala, WER, conversa humana, consumo, energia, latência ou execução no hardware. HELM recomenda justamente uma avaliação multidimensional para evitar conclusões de uma única métrica [4]. A alegação permitida permanece: **o HERUS possui capacidade local tipada e verificada de composição linguística controlada, enquanto a paridade com LLM em linguagem aberta continua não medida**.

O próximo passo lógico não é adicionar mais aliases indefinidamente. É congelar um corpus de paráfrases controladas com splits de composição, introduzir um oráculo de equivalência revisado por humano e medir risco-cobertura, abstenção e preservação semântica em uma expansão independente. Só depois disso será possível saber se a interface está se aproximando de uma capacidade generativa mais ampla, sem confundir cobertura artesanal com inteligência geral.

## Referências

[1]: https://aclanthology.org/2020.emnlp-main.731/ "Kim e Linzen, COGS, EMNLP 2020"

[2]: https://proceedings.neurips.cc/paper_files/paper/2020/hash/e5a90182cc81e12ab5e72d66e0b46fe3-Abstract.html "Ruis et al., gSCAN, NeurIPS 2020"

[3]: https://arxiv.org/abs/1912.09713 "Keysers et al., Measuring Compositional Generalization, ICLR 2020"

[4]: https://arxiv.org/abs/2211.09110 "Liang et al., Holistic Evaluation of Language Models, TMLR 2023"

[5]: https://github.com/EleutherAI/lm-evaluation-harness "EleutherAI, LM Evaluation Harness"

[6]: https://aclanthology.org/2022.acl-long.229/ "Lin et al., TruthfulQA, ACL 2022"

[7]: https://github.com/SummaArs/herus/blob/feat/herus-semantic-compiler/firmware/core/semantic_compiler.c "Compilador semântico C11 do HERUS"

[8]: https://aclanthology.org/2024.findings-naacl.100/ "Stolfo, Groundedness in Retrieval-augmented Long-form Generation, Findings of NAACL 2024"

[9]: https://github.com/SummaArs/herus/blob/feat/herus-semantic-compiler/firmware/core/test_semantic_compiler_language_ood.c "Suíte C11 de linguagem OOD do HERUS"

[10]: https://github.com/SummaArs/herus/blob/feat/herus-semantic-compiler/firmware/Makefile "Metas C11 do firmware"

[11]: https://github.com/SummaArs/herus/blob/feat/herus-semantic-compiler/tools/test_semantic_compiler_language_ood_redteam.py "Redteam da linguagem OOD do HERUS"

[12]: https://github.com/SummaArs/herus/blob/feat/herus-semantic-compiler/research/llm_parity_matrix_v1.json "Matriz de paridade do HERUS"
