# HERUS — Achados acadêmicos da rodada de adaptação

## Korycki & Krawczyk (2021), *Class-Incremental Experience Replay for Continual Learning under Concept Drift*

Fonte primária: https://arxiv.org/abs/2104.11861

O trabalho explicita uma tensão central: aprendizado contínuo tende a assumir que conhecimento antigo deve ser preservado, enquanto data-stream mining assume que informação desatualizada deve ser descartada. A proposta combina replay orientado por centróides com um buffer de subespaço reativo para acompanhar ocorrências de concept drift e adaptar clusters. O resumo descreve um cenário de aprendizado contínuo de preferências como caso ilustrativo.

Implicação para o HERUS: mudança de preferência não deve ser tratada apenas como novo fato que substitui o antigo, nem como ruído a ser ignorado. O sistema precisa distinguir memória histórica, preferência vigente, contexto de validade e mudança detectada. A hipótese AGSC deve evoluir para um estado de supersession explícita, mantendo a história mas retirando autoridade atual do predecessor.

## Kirkpatrick et al. (2017), *Overcoming catastrophic forgetting in neural networks*

Fonte primária: https://www.pnas.org/doi/10.1073/pnas.1611835114

O trabalho propõe elastic weight consolidation: pesos importantes para tarefas anteriores são protegidos contra alterações rápidas durante tarefas novas. O texto também descreve a diferença entre aprendizado multitarefa e sequência de tarefas, e a necessidade de mecanismos de memória/replay quando experiências antigas não estão disponíveis durante a atualização.

Implicação para o HERUS: congelar todo o conhecimento antigo seria inadequado para uma identidade pessoal viva. O equivalente seguro não é tornar toda memória imutável; é proteger proveniência e histórico, enquanto a autoridade corrente pode mudar por uma transição verificável. História permanece auditável; validade e escopo atuais são revogáveis.

## Hipótese da nova rodada

Para o HERUS, mudança legítima deve ser uma operação de primeira classe:

```text
observação nova + autoridade adequada + compatibilidade temporal
    → candidata de mudança
candidata repetida ou explicitamente confirmada
    → supersession do predecessor
conflito não resolvido ou origem contaminada
    → histórico preservado, autoridade atual bloqueada
```

A contribuição a testar será **AGSC-D**, uma extensão adaptativa do AGSC que separa identidade histórica, validade corrente e autoridade de ação. O objetivo não é apagar o passado, mas impedir que o passado continue agindo como presente sem evidência.

## Reconsolidação, unlearning e revogação

A literatura biológica sobre reconsolidação descreve que uma memória reativada pode entrar em uma janela de sensibilidade e ser atualizada; isso não equivale a apagar o traço original. A referência institucional consultada foi a revisão do NIH sobre reconsolidação: https://pmc.ncbi.nlm.nih.gov/articles/PMC4588064/

A literatura de machine unlearning mostra que apagar ou suprimir uma representação não é automaticamente o mesmo que removê-la. Xu et al. (2025, versão v3 de 2026), *Unlearning Isn't Deletion: Investigating Reversibility of Machine Unlearning in LLMs*, relata que métricas de tarefa podem indicar esquecimento enquanto comportamento original é recuperado com pouco fine-tuning. O trabalho propõe análise em nível de representação, com PCA, CKA, informação de Fisher e relearning como sondas de reversibilidade. Fonte: https://arxiv.org/abs/2505.16831

Implicação para o HERUS: `forget` não deve ser implementado como simplesmente esconder um card de consultas. A revogação precisa remover a autoridade atual, impedir recuperação e impedir reconstituição automática por caches, índices, resumos ou sucessores derivados. A história pode permanecer como tombstone auditável quando permitido, mas nunca como memória ativa ou fonte de ação.

## Consequência para AGSC-D

A próxima extensão deve separar quatro operações:

| Operação | Efeito seguro |
|---|---|
| Supersession | O novo fato ganha validade corrente; o predecessor perde autoridade atual, mas sua linhagem permanece verificável |
| Revogação | O fato deixa de ser recuperável e acionável; derivados identificados também são invalidados |
| Expiração | O fato deixa de ser atual por tempo/contexto, sem ser tratado como falso histórico |
| Esquecimento solicitado | O conteúdo e os derivados são removidos ou tombstoned segundo a política, com teste de não-reconstrução |

O benchmark deve testar não apenas se a consulta direta falha, mas se o fato antigo reaparece via recuperação indireta, resumo, conflito, reboot, replay ou nova observação contaminada.

## Pulipaka et al. (2026), *Hidden in Memory: Sleeper Memory Poisoning in LLM Agents*

Fonte primária: https://arxiv.org/abs/2605.15338

O trabalho define sleeper memory poisoning: conteúdo externo manipula o assistente para gravar uma memória fabricada sobre o usuário; o payload permanece dormente e reaparece em conversas posteriores. O resumo relata avaliação de escrita, recuperação e uso agentivo da memória, destacando que persistência temporal é parte do ataque.

Implicação para o HERUS: um redteam não pode testar somente a entrada e a consulta imediatamente seguintes. Deve haver uma sequência intermediária de eventos neutros, reboot, mudança de contexto e múltiplas sessões antes do gatilho. A memória antiga precisa carregar sua origem e não pode ganhar autoridade apenas porque ficou dormente por tempo suficiente.

## Gao et al. (2026), *MemPoison: Uncovering Persistent Memory Threats and Structural Blind Spots in LLM Agents*

Fonte primária: https://arxiv.org/abs/2607.14651

O trabalho propõe 1.227 casos validados manualmente em quatro tipos de ataque, três canais de injeção e três substratos de memória. Sua taxonomia distingue L1, corrupção direta de um registro; L2, corrupção composicional de múltiplos registros; e L3, corrupção dormente condicionada a contexto. O resumo afirma que defesas de escrita reduzem L1, mas não suprimem de modo confiável L2 e L3, pois registros aparentemente benignos podem se tornar nocivos por composição ou ativação condicionada.

Implicação para AGSC-D: não basta validar cada card isoladamente. O HERUS precisa testar a composição de duas ou mais memórias, a ativação após mudança de contexto e a derivação de uma nova preferência a partir de registros sem autoridade suficiente. A política deve ser sensível ao contexto de ação e exigir que a autoridade do resultado composto seja no máximo a autoridade mínima de suas fontes.

## Nova superfície de ataque a implementar

O redteam AGSC-D deverá conter:

| Ataque | Propriedade requerida |
|---|---|
| Preferência única externa | Não ganha autoridade pessoal sem confirmação |
| Dois registros benignos que compõem instrução perigosa | Composição não amplifica autoridade |
| Memória dormente entre sessões | Gatilho futuro não revive autoridade antiga |
| Preferência antiga após nova confirmação | Supersession explícita retira validade atual do predecessor |
| Revogação seguida de replay/reboot | Conteúdo revogado não retorna por índice, cache ou floor |
| Mudança única e ruidosa | Não reescreve identidade pessoal por uma observação |
