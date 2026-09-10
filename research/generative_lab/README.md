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

## Skill Layer v1

A camada de Skills adiciona um objeto explícito para procedimentos reutilizáveis sobre estados tipados. Uma Skill contém tipos de entrada e saída, pré e pós-condições, programa em DSL fechado, dependências, orçamento de recursos, efeitos permitidos, proveniência, hash de evidência e estado de confiança.

O ciclo de confiança é `CANDIDATE → QUARANTINED → TESTED → VERIFIED`. O sintetizador enumera candidatos; o verificador independente interpreta somente o DSL conhecido; a biblioteca vincula a evidência ao hash de conteúdo antes de promover a Skill. `AUTHORIZED` e `ACTIVE` não podem ser concedidos por esta camada.

O primeiro experimento usa aritmética inteira em RPN. Ele examinou 6.713 candidatos, rejeitou 6.712 e verificou uma transformação `2x + 3` em três casos visíveis e três casos ocultos. O resultado completo está em [`../evidence/skill_layer_v1/limits.md`](../evidence/skill_layer_v1/limits.md). A execução é `make -C research generative-lab-skills`.

A camada ainda não sintetiza código executável, não toca HIR/Babel, não controla hardware e não demonstra raciocínio aberto. Seu papel atual é provar o mecanismo mínimo de crescimento verificável de capacidade.

A segunda prova usa a política `retrieve-first`: duas Skills verificadas (`double` e `add-three`) são recuperadas, compostas em `double-then-add-three` e verificadas em dois casos visíveis e dois ocultos. O planejador tentou uma composição e encontrou uma solução sem gerar novamente a regra final. A evidência está em [`../evidence/skill_layer_v1/composition.json`](../evidence/skill_layer_v1/composition.json), e a execução é `make -C research generative-lab-compose`. A composição continua como `CANDIDATE` até passar por uma atestação registrada; ela não recebe autoridade.

A memória v2 calcula utilidade a partir de confiabilidade, generalização, reutilização e custo. Skills exatamente redundantes podem ser arquivadas, mas não deletadas: o registro e seu hash permanecem disponíveis para auditoria. A fixture arquivou `memory-duplicate@1` e preservou seu hash; a evidência está em [`../evidence/skill_layer_v1/memory.json`](../evidence/skill_layer_v1/memory.json), e a execução é `make -C research generative-lab-memory`.

O segundo domínio usa políticas finitas sobre `SAFE`, `WAIT` e `ALERT`, com sinais `OK`, `TIMEOUT` e `CANCEL`. O sintetizador examinou 28 políticas, rejeitou 27 e encontrou uma política total que passou em dois casos visíveis e dois ocultos. Estado ou sinal fora do vocabulário falha fechado. As ações são rótulos simbólicos; nenhuma delas é chamada de atuador. A evidência está em [`../evidence/skill_layer_v1/policy.json`](../evidence/skill_layer_v1/policy.json), e a execução é `make -C research generative-lab-policy`.

## Não objetivos

A primeira versão não tenta resolver linguagem natural aberta, ontologia universal, conhecimento do mundo, aprendizagem, consciência, planejamento irrestrito, prova de completude, execução de programas arbitrários ou substituição demonstrada de um modelo de linguagem.

## Execução

A suíte específica pode ser executada com `python3 -m unittest -v research/test_generative_lab.py` e `PYTHONPATH=research python3 -m unittest -v research.test_skills`. O benchmark pode ser executado com `make -C research generative-lab`, o experimento de síntese com `make -C research generative-lab-skills`, o experimento de composição com `make -C research generative-lab-compose`, o experimento de memória com `make -C research generative-lab-memory`, o experimento de políticas com `make -C research generative-lab-policy`, e a medição de escala com `make -C research generative-lab-scale`. A suíte completa permanece em `make -C research test`.

Na primeira execução integrada, a suíte completa passou com 111 testes e o benchmark passou em 8/8 casos. Esses resultados são do host e devem ser repetidos em qualquer mudança do núcleo do laboratório.
