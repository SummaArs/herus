# Protótipo de raciocínio finito e generativo controlado

Este diretório contém uma linha experimental **host-only**, sem autoridade sobre o firmware, o cofre, o rádio, a confirmação física ou a memória operacional do HERUS.

## O que foi criado

`finite_reasoner.py` implementa uma versão mínima e auditável de quatro ideias do adendo:

| Pilar proposto | Implementação experimental | Limite assumido |
|---|---|---|
| Sintaxe por operads coloridos | Assinatura finita de operações com cor, sort, aridade e tipos de entrada | Não é um sistema de operads completo nem gera linguagem aberta. |
| HoTT/univalência | Não implementado como fundação completa; a primeira versão preserva apenas identidade tipada e equivalência explícita | Não há alegação de HoTT, univalência ou cardinalidade homotópica. |
| E-graphs/equality saturation | Grafo de igualdade com união, histórico estrutural, combustível e orçamento de nós | Saturação é limitada; não há busca não terminante nem ponto fixo ilimitado. |
| Lógica paraconsistente | Estados `TRUE`, `FALSE`, `BOTH`, `NEITHER`, com evidências positivas e negativas separadas | Contradição não autoriza ação e não produz conclusão por explosão. |

A operação generativa não é uma licença para inventar símbolos. Toda expressão nova precisa ser construída por uma operação conhecida, com entradas conhecidas, cor compatível e sort compatível. O e-graph pode preservar alternativas equivalentes, mas o orçamento é explícito e o esgotamento falha fechado.

## Como executar

```bash
cd research
python3 -m unittest -v test_finite_reasoner.py test_semantic_ir.py test_semantic_ir_fuzz.py
python3 semantic_ir_fuzz.py
python3 finite_reasoner.py
```

## Critério de integração futura

O protótipo somente poderá influenciar o HERUS se produzir uma saída convertível em cartão de contexto canônico, passar pela política de memória, receber autorização humana quando necessário e respeitar os gates físicos existentes. Nenhuma igualdade, hipótese contraditória ou resultado de busca pode enviar rádio, gravar cofre ou abrir sessão.

A adoção de uma LLM local permanece uma hipótese posterior. Se existir, ela atuará antes deste núcleo, como camada de linguagem que propõe termos tipados; não poderá substituir a assinatura, a saturação limitada, a política, a autorização nem a explicação.

## Hipótese de pesquisa

> Um núcleo simbólico finito pode oferecer composição nova e explicável dentro de um vocabulário fechado, preservando garantias de tipagem, orçamento e falha fechada; ele não resolve compreensão ou geração abertas sem uma fonte adicional de grounding e aprendizagem.

Essa hipótese é falsificável por testes de composição, generalização combinatória, taxa de rejeição, crescimento do e-graph, custo de memória, conflitos paraconsistentes e conversão para cartões canônicos. O protótipo não deve ser apresentado como solução para síntese de programas de propósito geral.

## Campanha adversarial atual

`semantic_ir_fuzz.py` executa uma campanha determinística com seed `0x48525553`: 100 casos-base válidos e 27 mutadores inválidos, totalizando 2.700 casos mutados. O relatório JSON bruto fica em `evidence/semantic_ir_fuzz_raw.json`; os logs completos ficam em `evidence/adversarial_validation_raw.txt`. O critério de aprovação é zero caso válido rejeitado e zero caso mutado aceito.
