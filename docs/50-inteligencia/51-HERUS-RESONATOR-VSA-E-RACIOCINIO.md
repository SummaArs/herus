# HERUS — Resonator, VSA e raciocínio relacional local

**Status:** protótipo host-only integrado ao núcleo generativo; não é uma LLM geral e ainda não foi medido no ESP32-S3.

## Decisão de arquitetura

O HERUS já possui uma álgebra VSA densa baseada em hypervectors, binding por XOR, rotação, geração determinística de codebook e similaridade por distância de Hamming. Esta PR adiciona um **Resonator bounded** que trata a fatoração de um produto vetorial como uma busca combinatória local. O produto pode ser representado como:

```text
P = R(o0)F0 XOR R(o1)F1 XOR ... XOR R(on)Fn
```

O solver começa por limpeza alternada: calcula o resíduo dos outros fatores, desfaz a rotação da posição atual e procura o vetor mais próximo no codebook. Se a limpeza cair em mínimo local, o solver pode usar uma busca exata residual limitada para até três fatores, desde que o chamador forneça orçamento explícito. O resultado informa se houve convergência, ambiguidade, limite ou não-convergência; o caminho exato não fica escondido.

A ideia é fundamentada por trabalhos que formulam a fatoração de estruturas VSA como um problema combinatório e usam Resonator Networks para recuperar componentes de árvores e cenas compostas [1]. Trabalhos posteriores aplicam a formulação à fatoração de semiprimos [2], a regras abstratas em um relational bottleneck hiperdimensional [3] e a modelos generativos hierárquicos de cenas visuais em hardware neuromórfico [4]. Essas fontes sustentam a utilidade do mecanismo para inferência estruturada; não sustentam a afirmação de raciocínio geral ou de substituição universal de uma LLM.

## Ponte com o reasoner e o diálogo

O módulo `resonator_bridge` converte uma fatoração aprovada em uma **proposta relacional tipada**:

```text
factor[0] → sujeito
factor[1] → predicado
factor[2] → objeto
```

A proposta carrega o resultado do Resonator, o residual, a margem e a indicação de fallback exato. O bridge aplica gates de qualidade, mas não insere o fato no reasoner. A camada `symbolic_dialogue` pode exibir a proposta e só aceita sua promoção depois de confirmação física explícita. Portanto, o vetor não ganha autoridade por parecer semelhante.

| Camada | Pode fazer | Não pode fazer |
|---|---|---|
| VSA/Resonator | Fatorar, comparar, recuperar candidatos e expor margem | Persistir, transmitir, agir ou declarar verdade universal |
| Bridge | Recusar residual alto, margem baixa, ambiguidade ou limite | Converter automaticamente uma proposta em memória |
| Reasoner | Compor fatos tipados e gerar provas relacionais | Atribuir confirmação física a uma hipótese |
| Dialogue | Perguntar, apresentar estado e encaminhar confirmação | Enviar HCP, abrir rádio ou guardar áudio/texto bruto |

## Resultados reproduzidos

A suíte host-only executa com `HV_LUT_POPCOUNT` para evitar que o benchmark dependa de um caminho de população de bits diferente do alvo documentado. Os resultados atuais são:

| Suíte | Resultado |
|---|---:|
| Fatoração VSA e limpeza | 9 invariantes |
| Ponte VSA→reasoner | 6 invariantes |
| Stress em três tamanhos de codebook e 36 trials | 37 invariantes |
| Reasoner, planner e dialogue após integração | 17 + 8 + 14 invariantes |
| Baseline total do HERUS | **42/42 suítes aprovadas** |
| Bancada virtual sistêmica | **111 invariantes aprovados** |
| Gate de mutação existente | **7/7 mutantes inseguros detectados** |

Nos 36 trials de stress, os 36 casos exatos convergiram; o cenário de corrupção de 2% não foi tratado como prova perfeita. Esse número é resultado desta campanha determinística específica, com codebooks e seeds definidos no teste; não é uma taxa universal de acurácia e não deve ser extrapolado para linguagem, áudio ou hardware.

## Limites e interpretação correta

Resonator Networks não eliminam a busca; elas a estruturam. A qualidade depende do codebook, da dimensão, do número de fatores, da interferência de composição, da margem entre candidatos, do ruído e do orçamento. Uma solução exata dentro de um codebook ainda pode ser semanticamente errada se a representação inicial estiver errada ou se houver colisão de significado.

O anexo sugeriu números como 91,5%, 2,7 iterações e 32.768 hipóteses. Esses números não foram incorporados ao HERUS porque não foram reproduzidos nesta campanha com protocolo completo, sementes, dimensão, codebook, critério de acerto e orçamento publicados. Eles permanecem hipótese externa, não resultado do projeto.

A contribuição prática desta PR é mais modesta e mais importante: o HERUS agora pode usar VSA como **memória de relações e gerador de candidatos**, reasoner simbólico como **compositor de provas**, planner como **gerador de sequências** e diálogo como **fronteira de confirmação**. Essa combinação aproxima o produto de uma inteligência local útil em domínios delimitados, mas ainda não oferece cobertura de linguagem aberta, conhecimento mundial ou fluência de LLM.

## Referências

[1]: https://arxiv.org/abs/2007.03748 "Resonator networks for factoring distributed representations of data structures — Frady, Kent, Olshausen e Sommer"

[2]: https://arxiv.org/abs/2203.00920 "Integer Factorization with Compositional Distributed Representations — Kleyko et al."

[3]: https://arxiv.org/abs/2405.14436 "LARS-VSA: A Vector Symbolic Architecture For Learning with Abstract Rules — Mejri, Amarnath e Chatterjee"

[4]: https://www.nature.com/articles/s42256-024-00848-0 "Neuromorphic visual scene understanding with resonator networks — Renner et al."
