# HERUS — AGSC-D: mudança, revogação e esquecimento sem reescrever a pessoa

**Estado:** protótipo host-side C11, pré-hardware.  
**Relação:** extensão adaptativa de AGSC (*Authority-Governed Semantic Continuity*).

## Pergunta

Como uma inteligência pessoal contínua pode aprender que a pessoa mudou sem ficar presa ao passado, sem apagar a história de forma falsa e sem permitir que uma observação externa, isolada ou dormente reescreva silenciosamente a identidade do usuário?

A pergunta é necessária porque aprendizado contínuo e concept drift tratam tensões diferentes. Aprendizado contínuo normalmente protege conhecimento anterior; data-stream mining pode descartar informação desatualizada. Korycki e Krawczyk formulam essa diferença e propõem combinar replay de experiências com adaptação a drift [1]. Em redes neurais, elastic weight consolidation protege parâmetros importantes de tarefas anteriores contra mudanças rápidas [2]. Para o HERUS, porém, congelar toda memória seria tão incorreto quanto apagá-la a cada evento: preferências humanas mudam, mas a história da mudança também pode ser relevante.

A hipótese AGSC-D é que o estado pessoal precisa de três camadas separadas: **histórico**, **validade corrente** e **autoridade de ação**. Uma mudança confirmada pode retirar validade corrente de um predecessor sem apagar sua linhagem. Revogação retira validade e ação; expiração retira atualidade sem declarar falsidade histórica; esquecimento solicitado remove o conteúdo ou o transforma em tombstone conforme a política, incluindo derivados identificáveis.

## Máquina de mudança

```text
observação nova
      │ confiança suficiente + confirmação física + confirmação explícita
      ▼
candidata de mudança
      │ compatibilidade temporal e origem não revogada
      ▼
supersession do predecessor → novo fato ativo

conflito / baixa confiança / origem revogada / epoch antigo
      └──────────────────────────────► rejeição ou abstention

revogação confirmada → fato + derivados → revogados
expiração temporal → fato não atual, história preservada
reboot → ativos em quarentena, epoch incrementado
```

A regra central é **não amplificação de identidade**. Uma observação externa não ganha autoridade pessoal por ser resumida. Um fato antigo não ganha validade porque foi recuperado muitas vezes. Um conjunto de registros benignos não pode gerar autoridade superior à menor autoridade de suas fontes. Um predecessor superseded não pode responder como presente. Um derivado de memória revogada não pode reentrar no modelo.

## Literatura e lacuna

A literatura sobre reconsolidação descreve atualização de memórias reativadas, mas não fornece, por si só, um contrato de autoridade de ação para um wearable soberano [3]. A literatura de machine unlearning mostra que supressão comportamental pode ser reversível: métricas de tarefa podem sugerir esquecimento enquanto relearning recupera o comportamento original [4]. Portanto, `forget` do HERUS não pode significar apenas esconder um card da consulta direta.

A segurança recente de agentes persistentes torna o problema mais urgente. Sleeper memory poisoning descreve memórias fabricadas que permanecem dormentes e reaparecem em conversas futuras [5]. MemPoison separa corrupção direta, corrupção composicional e corrupção dormente, observando que filtros no momento da escrita podem não bloquear combinações aparentemente benignas que se tornam nocivas durante recuperação [6].

A lacuna provisória não é “memória de longo prazo” isolada, pois isso já existe. A lacuna a testar é uma **continuidade pessoal adaptativa cuja validade atual, linhagem histórica e autoridade de ação são formalmente separadas**, com revogação transitiva, epoch pós-reboot e resistência a composição dormente.

## Implementação host-side

O módulo `sim/adaptive_change.{h,c}` implementa uma coleção limitada de fatos tipados e proveniência opaca. `sim/adaptive_change_scenario.c` usa o mesmo binário da bancada virtual e verifica mudança confirmada, baixa confiança, ausência de confirmação, supersession, linhagem histórica, revogação transitiva, expiração, derivação de memória revogada, replay pós-reboot e epoch.

| Evidência | Resultado |
|---|---:|
| Cenário AGSC-D inicial | **15/15** |
| Cenário AGSC-D após confiança e epoch | **16/16** |
| Redteam AGSC-D | **8/8 mutantes mortos** |
| Redteam AGSC | **8/8 mutantes mortos** |
| Redteam semantic-life | **8/8 mutantes mortos** |

Durante a implementação, um oráculo inicialmente esperava `AC_E_AUTH` para uma observação que também tinha confiança baixa. O contrato corretamente rejeitava primeiro por confiança. O cenário foi corrigido para separar as propriedades, preservando a descoberta em vez de mascará-la.

## O que ainda falta provar

O protótipo não aprende preferências de fala, não interpreta comportamento humano e não demonstra que uma pessoa considera uma mudança legítima. Ele apenas estabelece uma fronteira segura para quando um módulo de percepção ou diálogo fornecer uma observação tipada.

Também não provamos unlearning físico irreversível. O host prova que a memória não é recuperada por esta estrutura, mas não prova que nenhum cache externo, backup, flash ou derivação fora do módulo exista. No hardware, esse requisito dependerá de NVS/flash, energia interrompida, recuperação e política de erase verificáveis.

A próxima avaliação deverá inserir ataques L1, L2 e L3 inspirados em MemPoison: registro único, composição de registros e gatilho dormente após sessões neutras. Deve comparar AGSC-D com `latest-wins`, `similarity-only`, memória sem revogação e uma política de memória somente histórica.

## Referências

[1]: https://arxiv.org/abs/2104.11861 "Class-Incremental Experience Replay for Continual Learning under Concept Drift"

[2]: https://www.pnas.org/doi/10.1073/pnas.1611835114 "Overcoming catastrophic forgetting in neural networks"

[3]: https://pmc.ncbi.nlm.nih.gov/articles/PMC4588064/ "Reconsolidation and the Dynamic Nature of Memory"

[4]: https://arxiv.org/abs/2505.16831 "Unlearning Isn't Deletion: Investigating Reversibility of Machine Unlearning in LLMs"

[5]: https://arxiv.org/abs/2605.15338 "Hidden in Memory: Sleeper Memory Poisoning in LLM Agents"

[6]: https://arxiv.org/abs/2607.14651 "MemPoison: Uncovering Persistent Memory Threats and Structural Blind Spots in LLM Agents"
