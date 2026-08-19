# HERUS — Núcleo generativo simbólico local

**Status:** primeira implementação host-only em C11; sem alegação de equivalência universal com uma LLM.

## Tese

O problema do HERUS não é apenas responder mensagens. É produzir uma resposta nova a partir de memória autorizada, regras, contexto e objetivo, sem transformar plausibilidade em verdade nem transformar inteligência em autonomia. A solução adotada é um núcleo de **álgebra simbólica generativa**: fatos tipados entram em uma base limitada; regras fazem unificação e composição; conclusões novas recebem prova; contradições, ambiguidade, ausência de evidência e orçamento insuficiente aparecem como estados explícitos.

> **Generativo, aqui, significa derivar uma estrutura nova que não estava armazenada literalmente.** A frase final, a vibração ou o HCP são renderizações de uma resposta semântica verificada; não são a própria fonte da inteligência.

A arquitetura foi inspirada por duas ideias estabelecidas. A síntese guiada por contraexemplos separa proposição de verificação e usa falhas para corrigir ou rejeitar a proposta [1]. O planejamento simbólico representa o mundo, as ações e os objetivos e procura uma sequência que satisfaça pré-condições e efeitos [2]. No HERUS, essas funções são realizadas localmente por tabelas fixas, unificação, saturação, busca limitada e provas parentais; não dependem de uma LLM hospedada.

## Equivalência funcional delimitada

Não é tecnicamente honesto afirmar que algumas regras C11 substituem uma LLM universal. O objetivo mensurável é equivalência em capacidades delimitadas que importam para o produto cotidiano.

| Capacidade funcional | Mecanismo local | Critério de sucesso |
|---|---|---|
| Conversar | Diálogo semântico sobre padrões tipados | Pergunta recebe estado direto, derivado, ausente, ambíguo, contraditório ou limitado |
| Recuperar memória | Fatos pessoais autorizados e consultas tipadas | Nenhum fato pessoal entra sem confirmação; ausência não vira invenção |
| Compor conhecimento | Regras com variáveis e múltiplas premissas | Conclusão nova tem pais, profundidade e custo registrados |
| Planejar | Ações com pré-condições, efeitos e custo | Sequência alcança objetivo ou retorna sem plano/limite |
| Revisar | Contradição, ambiguidade e contraexemplo | O sistema reduz confiança ou se abstém quando a base não decide |
| Generalizar | Variáveis aplicadas a entidades novas | Regra não precisa copiar literalmente o exemplo de treinamento |
| Permanecer seguro | Nenhuma função possui transmissão ou execução | Plano marcado para confirmação continua sendo apenas proposta |

A métrica não será “parece humano”. Será uma matriz de tarefas com cobertura estrutural, contradições detectadas, abstentions corretas, provas produzidas, nós explorados, memória usada e custo de cálculo. Voz, idioma natural, latência no ESP32-S3, consumo e ergonomia permanecem gates separados.

## Componentes implementados

O `symbolic_reasoner` contém uma base limitada de fatos positivos ou negativos, regras de até quatro premissas, variáveis limitadas, saturação até ponto fixo ou orçamento, prova parental, custo de derivação, detecção de contradição e consulta conservadora. Uma consulta variável com múltiplas respostas é ambígua; uma consulta sem evidência é ausente; evidência positiva e negativa para o mesmo fato é contraditória.

O `symbolic_planner` transforma um estado finito e ações tipadas em uma sequência limitada. Ele impede ciclos de estado, contabiliza custo, marca ações que exigem confirmação e retorna `SP_NO_PLAN`, `SP_E_LIMIT` ou `SP_E_CONTRADICTION` em vez de fabricar um plano.

O `symbolic_dialogue` fornece a fronteira de produto. Regras de fábrica são instaladas separadamente de fatos pessoais. Um fato pessoal sem confirmação explícita retorna erro de autoridade e não entra no motor. Perguntar é somente leitura; a resposta inclui o turno e a prova, mas não possui caminho para rádio, persistência ou ação.

## O que já foi provado em host

| Suíte | Resultado |
|---|---:|
| Reasoner | 17 invariantes |
| Planner | 8 invariantes |
| Dialogue | 10 invariantes |
| Total desta evolução | **35 invariantes** |

Os testes cobrem duplicação, composição de duas regras, geração de conclusão nova, prova, ausência de evidência, consulta ambígua, contradição, variável não ligada, orçamento de saturação, planejamento causal, ciclo, objetivo inalcançável, limite de nós, confirmação física e limite de derivação no diálogo.

## Limites atuais

Esta implementação ainda não interpreta fala, texto livre ou HCP automaticamente; ela recebe símbolos já tipados. Ela não possui vocabulário mundial, conhecimento geral aberto, percepção, modelo probabilístico amplo ou fluência de uma LLM. Portanto, o resultado atual é **um motor generativo semântico verificável**, não uma LLM equivalente em cobertura universal.

O próximo passo técnico é construir o compilador local que transforma uma entrada de linguagem restrita em padrões e objetivos, sem permitir que texto não verificado escreva diretamente na base. Depois virão memória pessoal seletiva, busca, revisão por contraexemplo, renderização de resposta e avaliação de generalização. Cada camada deverá manter a separação entre hipótese, prova e autoridade.

## Referências

[1]: https://arxiv.org/abs/2309.16436 "Neuro Symbolic Reasoning for Planning: Counterexample Guided Inductive Synthesis using Large Language Models and Satisfiability Solving — Jha et al., 2023"

[2]: https://ojs.aaai.org/index.php/AAAI/article/view/30277 "Symbolic Reasoning Methods for AI Planning — Gregor Behnke, AAAI 2024"
