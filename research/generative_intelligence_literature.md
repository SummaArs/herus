# Literatura para a inteligência generativa local do HERUS

**Data da consulta:** 2026-08-21

**Uso:** insumo para o contrato generativo e os benchmarks pré-hardware.

## Achados principais

### Composição sistemática

O trabalho de Wold et al. formaliza a generalização sistemática como generalização fora da distribuição em que componentes conhecidos aparecem em combinações ou posições novas. O artigo também propõe controlar a dificuldade por entropia da distribuição dos componentes: distribuições de baixa entropia podem permitir atalhos e tornam a generalização mais difícil, enquanto maior diversidade de combinações oferece mais informação estrutural. A consequência para o HERUS é que um benchmark generativo não pode usar apenas frases novas dentro da mesma distribuição; deve separar componentes conhecidos, combinações inéditas, posições novas, comprimento novo e suporte de vocabulário novo.

Fonte: https://arxiv.org/html/2505.13089v1
Título: *Systematic Generalization in Language Models Scales with Information Entropy*, Wold et al., 2025.

### Meta-aprendizagem e composição

Lake e Baroni mostram que a composição sistemática pode ser melhorada por uma rotina de meta-aprendizagem que apresenta episódios com exemplos de suporte e consulta. O resultado relevante para o HERUS não é importar o modelo ou alegar equivalência, mas separar duas propriedades que normalmente são confundidas: rigidez simbólica perfeita e flexibilidade estatística. O contrato do HERUS deve testar ambas: composição correta de regras e comportamento gracioso quando a regra é ambígua, incompleta ou fora de escopo.

Fonte: https://www.nature.com/articles/s41586-023-06668-3
Título: *Human-like systematic generalization through a meta-learning neural network*, Lake e Baroni, Nature, 2023.

### Groundedness e proveniência

Stolfo relata que, em geração longa fundamentada por recuperação, uma fração relevante das sentenças pode permanecer sem suporte mesmo quando a resposta contém fatos corretos. Portanto, correção superficial não basta como critério de geração. No HERUS, cada unidade gerada precisa ser classificável como derivada de memória, derivada de regra, proposta contrafactual ou abstinente; uma frase plausível sem suporte não pode receber confiança forte apenas por parecer fluente.

Fonte: https://aclanthology.org/2024.findings-naacl.100/
Título: *Groundedness in Retrieval-augmented Long-form Generation: An Empirical Study*, Stolfo, Findings of NAACL, 2024.

### Abstenção seletiva

Wen et al. organizam a abstenção em três perspectivas: respondibilidade da consulta, confiança do modelo na resposta e alinhamento com valores humanos. O artigo trata a abstenção como uma decisão composta, não apenas como um limiar de confiança. Para o HERUS, isso se traduz em gates independentes para suficiência de evidência, consistência entre fontes, escopo autorizado, privacidade e exigência de confirmação; o sistema não deve esconder uma falha de conhecimento atrás de uma resposta verbalmente cautelosa.

Fonte: https://direct.mit.edu/tacl/article/doi/10.1162/tacl_a_00754/131566
Título: *Know Your Limits: A Survey of Abstention in Large Language Models*, Wen et al., TACL, 2025.

## Consequências para o HERUS

1. “Generativo” será definido como produção de novas composições verificáveis a partir de componentes, regras, memória e contexto, não como produção de texto livre sem rastreabilidade.
2. A avaliação terá splits de composição: combinação inédita, permutação, profundidade, comprimento, vocabulário e suporte de poucos exemplos.
3. A fluência não será uma métrica de segurança. Groundedness, validade da derivação, precisão de desconhecido, recall de abstenção e zero violações de autoridade serão gates de release.
4. A resposta final deverá ter uma forma de proveniência e uma política de apresentação. O gerador pode propor, mas não pode executar, transmitir, persistir ou ampliar autoridade.
5. O objetivo prático será uma arquitetura híbrida: símbolos e regras para exatidão, estado semântico e memória para continuidade, composição para novidade controlada e um gerador superficial bounded para verbalização. Isso é uma hipótese de engenharia, não uma reivindicação de novo paradigma antes da comparação experimental.

### COGS e gaps estruturais

A fonte primária do COGS descreve um conjunto de parsing semântico com gaps sistemáticos, incluindo combinações novas de estruturas sintáticas familiares e combinações novas de palavras e estruturas familiares. O resumo reporta que Transformers e LSTMs obtiveram acurácia in-distribution próxima de 96–99%, mas generalização entre gaps de 16–35%, com sensibilidade de aproximadamente 6–8 pontos a seeds. A consequência para o HERUS é separar explicitamente desempenho dentro da distribuição de generalização composicional e repetir resultados determinísticos sem depender de seed.

Fonte: https://aclanthology.org/2020.emnlp-main.731/

### gSCAN e grounding situado

A fonte primária do gSCAN define uma linguagem grounded em estados de um grid world para avaliar generalização composicional situada. O benchmark testa propriedades como a interpretação de adjetivos em relação ao estado atual e a combinação de advérbios com verbos novos; o resumo informa que baselines multimodais fortes falharam dramaticamente quando a generalização exigia regras composicionais sistemáticas. Para o HERUS, a analogia útil não é um grid visual, mas o estado local: intenção, geração, memória, autoridade e canal háptico devem ser tratados como contexto que muda a interpretação da mesma composição.

Fonte: https://proceedings.neurips.cc/paper_files/paper/2020/hash/e5a90182cc81e12ab5e72d66e0b46fe3-Abstract.html

### CFQ e divergência de compostos

A versão arXiv do trabalho de Keysers et al. descreve um método para construir benchmarks em que a divergência entre compostos treino-teste é maximizada enquanto a divergência entre átomos é mantida pequena. O artigo apresenta o CFQ como conjunto realista de perguntas em linguagem natural e relata uma correlação negativa forte entre compound divergence e acurácia. A implicação direta para o HERUS é não usar apenas frases novas: o benchmark deve manter operadores, símbolos e primitivas conhecidos, mas combinar essas peças em estruturas que não apareceram no treino.

Fonte: https://arxiv.org/abs/1912.09713

A página OpenReview correspondente exigiu uma verificação de navegador e não foi usada como fonte factual; o artigo arXiv foi preferido para manter a pesquisa passiva e reproduzível.
