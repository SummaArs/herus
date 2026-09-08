# HERUS — Avanço de inteligência: composição e abdução restrita

**Estado:** host-only, C11 estrito, sem rede, sem modelo hospedado e sem autoridade de execução.  
**Baseline global:** 54/54 suítes, proveniência válida, bancada simulada 111/111 e mutação 7/7.

## 1. O gargalo encontrado

A auditoria mostrou que o reasoner já aceitava até quatro premissas e mantinha prova/contradição/limites, mas o compilador semântico expunha somente regras de uma premissa. O sistema tinha capacidade interna maior do que a linguagem de entrada conseguia expressar. O primeiro avanço foi liberar essa capacidade de forma controlada, em vez de criar outro reasoner.

A segunda lacuna era a ausência de uma resposta segura para “o que falta para isso ser verdade?”. Responder com um fato inventado seria perigoso; recusar sempre perderia uma informação útil. Foi implementada uma abdução restrita: ela propõe um único fato ground ausente que tornaria um objetivo demonstrável, informa regra, índice da premissa e quantidade de suportes, mas não altera a base.

## 2. Implementação

O compilador agora aceita conjunções explícitas em regras, por exemplo:

> `Se alguem possui caderno e alguem esta em casa, entao alguem pode estudar.`

A saída contém duas premissas, variável compartilhada, predicados canônicos e custo igual ao número de premissas. Segmento vazio, mais de quatro premissas e sintaxe adicional continuam falhando fechado.

O reasoner ganhou `sr_abduce()`. A API exige objetivo ground, respeita `max_candidates`, deduplica a mesma hipótese, retorna `SR_ABDUCTION_AMBIGUOUS` quando há fatos ausentes diferentes e `SR_ABDUCTION_LIMIT` quando o orçamento termina. A hipótese não é inserida no reasoner.

O diálogo ganhou `sd_abduce()`. Ele trabalha sobre uma cópia bounded do estado, satura somente o scratch dentro do orçamento e converte proposta, ambiguidade, limite e objetivo não ground em estados públicos distintos. Mesmo com a confirmação física fornecida a uma consulta, a hipótese não é persistida: a confirmação de uma hipótese deve ser uma operação externa e explícita, nunca um efeito colateral da sugestão.

## 3. Evidência executada

| Gate | Resultado | Evidência |
|---|---:|---|
| compilador semântico | **54/54** | composição, erro de conjunção, limite e integração |
| reasoner | **26/26** | abdução única, read-only, ambiguidade, orçamento e regressões |
| planner | **9/9** | custo, no-plan, ciclos, limite e confirmação |
| diálogo simbólico | **20/20** | abdução read-only, ambiguidade, limite e autoridade |
| prova global | **54/54** | todas as suítes do ledger |
| proveniência | **válida** | `prove.sh` e `firmware` reconciliados |
| mutação adversarial | **7/7** | controles críticos continuam detectáveis |

O teste ponta a ponta fornece duas memórias pessoais confirmadas, instala uma regra de duas premissas e deriva a conclusão com duas evidências. Quando uma premissa é removida, o diálogo retorna ausência/abstenção, não uma conclusão parcial. Quando duas hipóteses são igualmente válidas, o resultado é ambiguidade e nenhuma delas é escolhida.

## 4. Relação com o estado da arte

A direção é inspirada em ProofWriter, que separa resposta com prova, enumeração de implicações e abdução restrita em teorias de mundo aberto [1]. O HERUS não reproduz os resultados do artigo nem usa o modelo T5 descrito ali. A contribuição local é manter a parte verificável no núcleo C11, com símbolos numéricos, orçamento e autoridade humana; a tradução de linguagem continua sendo controlada e limitada.

O avanço melhora **expressividade composicional** e **utilidade epistemológica**, mas não transforma o HERUS em uma LLM. Abdução restrita não é descoberta livre: ela só propõe um fato que satisfaz uma regra já fornecida. O benchmark v1 ainda precisa testar generalização fora do conjunto de regras, atualização temporal e memória multi-sessão antes de qualquer comparação séria.

## 5. Limites preservados

Não há aprendizagem de regras a partir de linguagem aberta, não há modelo local treinado, não há embedding, não há conhecimento mundial implícito e não há execução de planos. A hipótese abduzida não é memória pessoal, não é verdade e não é comando. Ela é uma **proposta verificável sob a base atual**.

## Referências

[1]: https://arxiv.org/html/2012.13048v2 "ProofWriter: Generating Implications, Proofs, and Abductive Statements over Natural Language"
