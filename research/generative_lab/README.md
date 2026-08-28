# Laboratório de raciocínio simbólico generativo

Este diretório é um laboratório host-only para testar a hipótese de raciocínio generativo por composição simbólica. Ele não é firmware, não recebe texto natural, não consulta modelos, não acessa rádio, não cria `Semantic IR` e não possui caminho para confirmação ou envio no HERUS.

## Tese testável

Um sistema simbólico pode gerar estruturas novas por composição de símbolos tipados, preservar hipóteses conflitantes sem explosão e encontrar equivalências por saturação limitada, desde que o vocabulário, as assinaturas, o orçamento e o critério de aceitação sejam explícitos.

O laboratório não assume que essa tese produz um sucessor geral dos modelos de linguagem. O primeiro objetivo é medir se há geração, derivação, equivalência e generalização composicional reais em um domínio formal controlado.

## Contrato v1

A linguagem é fechada por uma assinatura declarada. Cada símbolo possui um tipo de saída e cada operador possui tipos de entrada e saída. Um termo só pode ser construído se todas as entradas existirem na assinatura e os tipos coincidirem. Não há coerção implícita, chamada dinâmica, reflexão, parsing de linguagem natural ou execução de código gerado.

A composição é limitada por `max_depth`, `max_nodes`, `max_steps` e `max_terms`. Quando qualquer orçamento é excedido, o resultado é `BUDGET_EXCEEDED`, nunca uma aproximação silenciosa. Termos inválidos resultam em `TYPE_ERROR` ou `UNKNOWN_SYMBOL`.

Hipóteses são nomeadas por contexto. Uma contradição é registrada como `CONFLICT` no contexto local; ela não autoriza inferências de outro contexto e não produz conclusão arbitrária. A ausência de prova é `UNKNOWN`, não negação.

Igualdade por saturação acumula classes de equivalência somente a partir de regras orientadas e declaradas. A saturação é limitada por passos e número de termos. O extrator escolhe uma representação determinística por custo; isso é uma escolha de normalização, não uma prova de verdade externa.

Nenhum rótulo, classe ou resultado do laboratório pode ser convertido automaticamente em `ARRIVE`, `HELP`, `CANCEL`, proposta HCP ou comando de firmware. A integração permitida é somente evidência de pesquisa e métricas agregadas.

## Critérios de sucesso da primeira versão

| Capacidade | Evidência mínima |
|---|---|
| Geração | Produzir termos compostos inéditos dentro da assinatura |
| Tipagem | Rejeitar símbolo desconhecido, aridade errada e tipos incompatíveis |
| Raciocínio | Derivar uma consequência por regra explícita e fornecer a trilha |
| Contradição | Isolar `CONFLICT` sem derivar qualquer fato arbitrário |
| Equivalência | Colocar termos equivalentes na mesma classe sob orçamento finito |
| Limite | Retornar estado explícito ao exceder orçamento |
| Segurança | Nenhum módulo importa firmware ou possui função de envio |

## Não objetivos

A primeira versão não tenta resolver linguagem natural aberta, ontologia universal, conhecimento do mundo, aprendizagem, consciência, planejamento irrestrito, prova de completude, execução de programas arbitrários ou substituição demonstrada de um modelo de linguagem.

## Execução

A suíte específica pode ser executada com `python3 -m unittest -v research/test_generative_lab.py`. O benchmark pode ser executado com `make -C research generative-lab`, e a medição de escala com `make -C research generative-lab-scale`. A suíte completa permanece em `make -C research test`.

Na primeira execução integrada, a suíte completa passou com 111 testes e o benchmark passou em 8/8 casos. Esses resultados são do host e devem ser repetidos em qualquer mudança do núcleo do laboratório.
