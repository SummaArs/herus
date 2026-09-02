# HERUS — MVP de Síntese e Verificação de Máquinas de Estado

## Tese operacional

O primeiro marco falsificável do HERUS é demonstrar que uma máquina de estados finita, uma especificação de segurança e uma gramática limitada de políticas podem ser combinadas para produzir um artefato verificável ou uma saída explícita de falha/`UNKNOWN`. O sistema não poderá promover um candidato apenas porque ele ajusta exemplos, e não poderá autorizar atuadores.

A tese não é que o HERUS verifique o mundo real. A tese é mais estreita: **dado um modelo finito declarado, um conjunto de estados alcançáveis finito e invariantes explícitos, o verificador independente consegue enumerar o espaço relevante e decidir se uma política satisfaz os invariantes no modelo**.

## Escopo do MVP

O modelo conterá estados enumeráveis, entradas discretas, ações pertencentes a um vocabulário finito e uma função de transição total ou explicitamente incompleta. Uma política candidata será uma tabela pura de decisão `(estado, entrada) -> ação`. O MVP não terá sensores reais, rede, persistência mutável, aprendizado online, geração de código não verificado ou acesso a atuadores.

A especificação mínima terá um estado inicial, estados permitidos, entradas, ações, transições, estados proibidos, ações proibidas por estado e um limite máximo de passos. Transições ausentes serão classificadas como desconhecidas, nunca como seguras.

## Estados de saída

| Saída | Significado | Pode autorizar ação? |
|---|---|---|
| `VERIFIED` | Todos os caminhos enumerados respeitam os invariantes | Não diretamente; apenas produz evidência para revisão humana |
| `COUNTEREXAMPLE` | Foi encontrado um caminho que viola um invariante | Não |
| `UNKNOWN` | O modelo é incompleto, o orçamento acabou ou há ambiguidade | Não |
| `INVALID_SPEC` | A especificação tem contradições ou limites inválidos | Não |

## Critérios de sucesso

O MVP será considerado tecnicamente válido somente se satisfizer simultaneamente os seguintes critérios:

| Critério | Teste exigido |
|---|---|
| Verificação positiva | Uma política conhecida válida retorna `VERIFIED` em todos os caminhos do modelo |
| Contraexemplo | Uma política inválida retorna um caminho mínimo reproduzível até a violação |
| Falha fechada | Transição ausente, ação desconhecida, estado proibido e orçamento excedido não retornam `VERIFIED` |
| Independência | O verificador não reutiliza a função de pontuação da síntese para decidir validade |
| Determinismo | A mesma especificação e política produzem o mesmo resultado e certificado |
| Limites | Nós visitados, profundidade e tamanho do certificado têm orçamento explícito |
| Não-autoridade | Nenhum módulo chama firmware, rede, atuador ou confirmação física |

## Hipóteses e limites

A garantia é relativa à especificação. Ela não demonstra que a especificação representa adequadamente o sistema físico, que os sensores são verdadeiros ou que o compilador C11 é correto. O certificado deverá declarar o hash da especificação, o hash da política, o limite de exploração e o resultado do verificador.

O uso de otimização, reforço, surpresa causal ou síntese estrutural será permitido apenas para gerar e priorizar candidatos. Esses mecanismos não poderão alterar o verificador nem converter `UNKNOWN` em aprovação.

A transferência da garantia do modelo abstrato para uma implementação concreta exige uma relação de refinamento explícita. O mapa de estados deve ser total, o estado inicial concreto deve mapear para o estado inicial abstrato e cada transição concreta deve corresponder a uma transição abstrata com a mesma entrada e ação. Uma prova do modelo sem esse vínculo não é uma prova da implementação.

## Critério de impacto científico

O resultado terá relevância além de um exercício local somente se for comparado com enumeração ingênua e pelo menos um método de busca guiada sob o mesmo orçamento, com especificações públicas, seeds registradas, contraexemplos publicados e reprodução independente. Ganho de desempenho sem preservação de cobertura e segurança não será considerado avanço.

## Estado do marco

O verificador independente e a síntese enumerativa já foram implementados e testados. A relação abstrato-concreto está disponível em `research/critical_state_refinement.py` e rejeita mapa incompleto, estado inicial incompatível, transição ausente e sucessor não refinado. A comparação externa SyGuS agora possui execução limitada e reproduzível: sobre 9.719 arquivos públicos, 526 passaram no inventário lexical, mas apenas 8 foram traduzidos pelo subconjunto semântico atual e receberam `BOUNDED_VERIFIED` em `[-2,2]`, profundidade 2 e orçamento de 500 candidatos. Os 518 restantes retornaram `UNKNOWN` por recursos fora do contrato. Os 8 casos positivos são duplicatas aparentes; não representam oito problemas independentes nem sustentam superioridade sobre SyGuS-Comp.
