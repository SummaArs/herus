# Fontes externas consultadas

## Paraconsistência

[1] Stanford Encyclopedia of Philosophy, “Paraconsistent Logic”: https://plato.stanford.edu/entries/logic-paraconsistent/

A entrada define paraconsistência como uma relação de consequência não explosiva: de uma contradição não se segue arbitrariamente qualquer conclusão. Também ressalta que paraconsistência é uma propriedade de uma relação de consequência, não uma teoria única nem uma garantia de verdade. Uso no laboratório: `CONFLICT` deve ser local e não pode autorizar conclusão arbitrária.

## Igualdade por saturação

[2] Thomas Kœhler et al., “Guided Equality Saturation”, Proceedings of the ACM on Programming Languages, 2024: https://dl.acm.org/doi/10.1145/3632900

O resumo do artigo afirma que equality saturation é útil para reescrita, prova e compilação, mas que a busca automática não escala em espaços grandes. A técnica guiada usa pontos intermediários fornecidos por humanos para dividir o problema; nos estudos apresentados, isso reduziu casos de minutos para menos de um segundo e um caso de mais de uma hora/60 GB para segundos/menos de 1 GB. Uso no laboratório: saturação precisa de combustível, nós, termos, profundidade e eventualmente guias; não é uma busca livre sem custo.

## Programa neuro-simbólico verificável

[3] Debargha Ganguly et al., “Proof of Thought: Neurosymbolic Program Synthesis allows Robust and Interpretable Reasoning”, arXiv:2409.17270v2, 2024: https://arxiv.org/html/2409.17270v2

O trabalho descreve uma arquitetura que usa um modelo de linguagem para gerar uma DSL intermediária, converte essa DSL para lógica de primeira ordem e verifica com um provador. O próprio resumo condiciona as garantias à correção da base de conhecimento e das regras. Uso no laboratório: uma camada geradora não substitui a verificação; o caminho HERUS deve continuar sem modelo e sem autoridade externa.

## Limites atuais de raciocínio em LLMs

[4] Stanford CodeX, “Overcoming the Reasoning and Reliability Limitations of LLMs: A Neuro-Symbolic Approach”: https://law.stanford.edu/codex-stanford-center-for-legal-informatics/projects/overcoming-the-reasoning-and-reliability-limitations-of-llms-a-neuro-symbolic-approach/

A página descreve correção inconsistente de LLMs e lista como questões abertas representação, recuperação, generalização para domínios novos, alucinação e a integração entre flexibilidade neural e verificação simbólica. Uso no laboratório: a substituição de modelos de linguagem é uma hipótese a medir, não uma conclusão derivada do uso de tipos, e-graphs ou lógica paraconsistente.
