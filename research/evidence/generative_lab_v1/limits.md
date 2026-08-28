# Primeiro resultado executável — laboratório generativo v1

## Resultado observado

O laboratório executou oito casos determinísticos, todos aprovados: construção tipada, símbolo desconhecido, aridade inválida, geração bottom-up, derivação declarada, contradição local, equivalência por saturação e estouro de orçamento.

| Medida | Resultado |
|---|---:|
| Casos do benchmark | 8 |
| Casos aprovados | 8 |
| Termos gerados pela fixture bounded_generation | 25 |
| Derivações declaradas | 1 |
| Termos na classe equivalente principal | 2 |
| Passos observados no caso de estouro | 2 |
| Autoridade | `none` |
| Tempo observado do benchmark inicial | 0,05 s |

## O que foi demonstrado

A implementação já demonstra uma forma executável de geração composicional: uma assinatura fechada produz 25 termos tipados dentro de profundidade e número de termos limitados. O type-checker rejeita símbolo desconhecido, aridade errada, saída incompatível e orçamento excedido. Regras explícitas derivam consequências e deixam uma trilha de derivação.

Contradições positivas/negativas são mantidas dentro do contexto em que surgem e retornam `CONFLICT`; não são convertidas em fatos arbitrários. A saturação acumula termos relacionados por regras declaradas, forma classes de equivalência e para com `BUDGET_EXCEEDED` quando o limite é atingido.

## O que não foi demonstrado

Isso ainda não é raciocínio livre geral, compreensão de linguagem, conhecimento do mundo, aprendizagem, consciência ou sucessor de um modelo de linguagem. A fixture é formal e pequena. Os símbolos, tipos e regras foram fornecidos pelo experimento; o sistema não descobriu uma semântica aberta nem resolveu grounding.

A saturação atual é um fechamento limitado por reescritas; ela não prova que uma classe seja semanticamente verdadeira fora das regras declaradas. A derivação é forward-chaining simples, sem quantificação, indução, planejamento geral ou busca heurística. A paraconsistência está representada como isolamento de polaridades por contexto, não como uma nova lógica completa.

## Decisão de pesquisa

A hipótese permanece viva, mas ainda sem evidência para a afirmação forte de substituir modelos de linguagem. O próximo aumento de capacidade deve ser medido contra tarefas composicionais ocultando combinações de operadores no teste, com custo de busca, profundidade, taxa de abstenção e contradições registrados. Não será permitido introduzir texto natural, rótulos externos ou um modelo neural silencioso para elevar a pontuação sem declarar a mudança.

O laboratório permanece fora do firmware. Nenhum resultado pode ser convertido em `ARRIVE`, `HELP`, `CANCEL`, proposta HCP ou ação de rádio. O caminho de integração permitido é documentação, benchmark e métricas agregadas.

## Execução integrada

A suíte específica do laboratório passou com 9 testes após incluir a geração bottom-up. A suíte de pesquisa integrada passou com 111 testes. Os alvos `generative-lab` e `generative-lab-scale` também passaram e produziram JSON válido. O resultado permanece host-only; nenhuma métrica foi tratada como previsão para um chip sem medição em hardware real.
