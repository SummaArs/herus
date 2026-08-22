# Achados acadêmicos — composição contrafactual e isolamento entre principais

## Causal Agent Replay

Fonte: Shah, *Causal Agent Replay: Counterfactual Attribution for LLM-Agent Failures*, arXiv:2606.08275v1, https://arxiv.org/html/2606.08275v1.

O trabalho modela uma execução de agente como um structural causal model e aplica intervenções `do(...)` a etapas, reexecutando a trajetória e medindo a mudança na distribuição de resultados. O artigo destaca que, sob política estocástica, uma intervenção gera uma distribuição, não uma única trajetória. Também introduz uma regra de *point-of-commitment* para evitar o confundimento causado por reamostrar uma etapa e, implicitamente, reamostrar todas as etapas posteriores. O método separa estimador contrastivo de uma única etapa e atribuição de interações entre várias etapas por um estimador de Shapley com orçamento limitado.

Aplicação ao HERUS: a unidade contrafactual mínima não deve perguntar somente se uma memória aparece na oferta; deve remover uma aresta de suporte e verificar se a oferta ou a autorização muda. Para o host determinístico, a intervenção será bounded: remover fonte, derivação, propósito ou principal; reexecutar a cadeia; exigir abstention ou mudança observável quando o suporte removido era necessário. Não será apresentada como estimativa causal estatística.

## Collaborative Memory

Fonte: Rezazadeh et al., *Collaborative Memory: Multi-User Memory Sharing in LLM Agents with Dynamic Access Control*, arXiv:2505.18279v1, https://arxiv.org/html/2505.18279v1.

O trabalho separa memória privada, visível somente ao usuário originário, e memória compartilhada, composta por fragmentos selecionados. Cada fragmento carrega atributos imutáveis de proveniência, incluindo agentes contribuintes, recursos acessados e timestamps, para permitir verificações retrospectivas de permissão. As políticas de escrita decidem retenção e compartilhamento; políticas de leitura constroem uma visão filtrada conforme as permissões atuais. O modelo formaliza grafos dinâmicos usuário-agente e agente-recurso, permitindo revogações e mudanças de escopo ao longo do tempo.

Aplicação ao HERUS: o vestível possui um principal local soberano; o Core, outro principal de suporte; e um terceiro principal pode ser um contato pareado. A memória privada do usuário não pode entrar em uma cadeia compartilhada somente porque houve contato físico ou porque uma derivação preservou conteúdo sem preservar escopo. Compartilhamento deve ser uma aresta explícita, atual e revogável. Reintrodução por outro principal deve falhar se a raiz não tiver uma concessão de compartilhamento compatível.

## Requisitos derivados

| Requisito | Origem | Forma testável no HERUS |
|---|---|---|
| Intervenção de suporte | Causal Agent Replay | Remover uma aresta necessária altera a oferta ou produz abstention |
| Ponto de compromisso | Causal Agent Replay | A ação só é atribuída no estágio local de confirmação, não no estágio de recuperação |
| Proveniência imutável | Collaborative Memory | Derivação não troca principal, fonte ou recurso |
| Separação privado/compartilhado | Collaborative Memory | Memória privada não aparece em visão de outro principal sem grant explícito |
| Políticas de leitura e escrita distintas | Collaborative Memory | Admissão e persistência têm gates independentes |
| Revogação atual | Collaborative Memory | Após revogar uma aresta, a recuperação de outro principal falha |

## Limites

Esses trabalhos usam agentes e avaliações que não são o firmware C11 do HERUS. Eles fundamentam requisitos e não demonstram a implementação local. O HERUS fará uma versão determinística, limitada e auditável da ideia contrafactual; não reivindicará causalidade geral nem equivalência a um estimador estatístico de agentes estocásticos.
