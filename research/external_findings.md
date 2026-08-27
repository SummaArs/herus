# Evidências externas para a trilha experimental

## Symbolica

Fonte: [Symbolica Research](https://www.symbolica.ai/research), acesso em 2026-08-26.

A Symbolica descreve uma arquitetura "Neural + Symbolic" baseada em teoria das categorias e teoria dos tipos para raciocínio estruturado. Seu programa busca síntese de programas de propósito geral numa arquitetura categórica não-LLM, mas declara explicitamente que a síntese geral ainda não foi resolvida e que o sucesso não é garantido. A própria tese combina confiabilidade de execução simbólica com adaptabilidade de otimização neural.

Implicação: isso apoia uma fronteira honesta para o HERUS. A contribuição não deve ser vendida como síntese geral resolvida; pode ser uma solução finita, tipada, explicável e embarcada para um domínio específico.

## E-graphs

Fonte: [egg: e-graphs good](https://egraphs-good.github.io/), acesso em 2026-08-26.

A página oficial descreve e-graphs como uma representação compacta de muitos programas equivalentes e equality saturation como técnica aplicável a otimização e síntese. Também mostra que o ecossistema trata expressões potencialmente muito grandes, inclusive representações de muitas formas de um programa, e oferece sistemas mais sofisticados com análise e execução incremental.

Implicação: e-graphs são adequados para preservar alternativas e equivalências sem reescrita destrutiva, mas não são um gerador mágico de significado. O protótipo deve impor combustível, orçamento de nós, tipagem, seleção e critério de saída; sem isso, a saturação pode explodir ou permanecer sem direção.

## Fontes adicionais a consultar

- [The HoTT Book](https://homotopytypetheory.org/book/): fonte de referência para HoTT e fundamentos univalentes; não há implementação de HoTT no protótipo inicial.
- [Paraconsistent Logic, Stanford Encyclopedia of Philosophy](https://plato.stanford.edu/entries/logic-paraconsistent/index.html): referência conceitual para lógica não explosiva; o protótipo usa um fragmento de quatro estados, não um cálculo paraconsistente completo.

## HoTT

Fonte: [The HoTT Book](https://homotopytypetheory.org/book/), acesso em 2026-08-26.

A página apresenta HoTT como uma exposição sistemática dos fundamentos univalentes e de um estilo de raciocínio matemático, sem exigir assistente de prova. Isso confirma que HoTT é uma fundação formal rica; não é, por si só, um procedimento de busca/generação de programas nem um substituto para uma gramática, heurística, custo ou critério de seleção.

Implicação: o protótipo deve tratar HoTT como possível futura camada de especificação/verificação, não alegar que a usa apenas por ter tipos e equivalências. A primeira versão não implementa HoTT, univalência ou cardinalidade homotópica.

## Lógica paraconsistente

Fonte: [Paraconsistent Logic, Stanford Encyclopedia of Philosophy](https://plato.stanford.edu/entries/logic-paraconsistent/index.html), acesso em 2026-08-26.

A entrada define paraconsistência pela não-explosividade: uma lógica é paraconsistente quando contradições não implicam arbitrariamente qualquer conclusão. A página também discute aplicações em raciocínio automatizado, revisão de crenças e teorias inconsistentes.

Implicação: o quarto pilar é diretamente útil para o HERUS como contenção de evidência conflitante, mas não resolve grounding, busca ou seleção de uma ação. O protótipo representa apenas o fragmento operacional de quatro estados e conserva as evidências para explicação.
