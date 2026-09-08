# HERUS — AGSC: Continuidade Semântica Governada por Autoridade

**Estado:** protótipo host-side em C11; hardware ainda pendente.  
**Nome de trabalho:** *Authority-Governed Semantic Continuity* (AGSC).  
**Status científico:** hipótese arquitetural implementada e falsificável; nenhuma prioridade ou “novo paradigma” é reivindicada sem comparação adicional.

## Resumo

A investigação acadêmica do HERUS examinou attention/Transformer, RAG, aprendizado contínuo, memória neuro-simbólica, memória agentiva, abstention, proatividade e wearables de inferência local. A literatura já contém precedentes para quase todos os componentes isolados. O trabalho do HERUS, portanto, não pode ser apresentado honestamente como a invenção de memória episódica-semântica, abstention ou personalização.

O avanço implementado é uma hipótese de composição: a continuidade pessoal é representada como uma **máquina de transições de autoridade semântica**. Uma observação, uma memória, uma recuperação, uma oferta e uma ação são estados distintos. Cada transição exige evidência própria; nenhuma etapa pode conceder silenciosamente autoridade da etapa seguinte. Proveniência permanece anexada ao estado. Conflito e expiração são estados bloqueadores, não detalhes de ranking. Reboot muda o epoch e invalida ofertas antigas. O Core pode ser fonte de conhecimento, mas não é fonte de autoridade de execução.

## Relação com o estado da arte

O Transformer mostrou que uma mudança profunda pode surgir ao trocar a unidade central de computação de recorrência/convolução por attention [1]. RAG separou memória paramétrica e não paramétrica para melhorar acesso, atualização e proveniência de conhecimento [2]. EWC separou plasticidade de estabilidade para reduzir esquecimento catastrófico [3]. A literatura on-device explicita o custo de treinamento, memória e energia em dispositivos restritos [4]. NS-Mem já combina memória episódica, semântica e lógica [5]. Trabalhos recentes formalizam abstention sequencial [6], preferências de proatividade [7], personalização implícita [8] e memória de longo prazo [9]. Wearables como o estudo clínico on-device da Nature [10] e Memoro [11] demonstram, em tarefas e configurações específicas, que inferência local contínua e memória vestível podem ter valor.

A conclusão crítica é dupla. Primeiro, o HERUS não possui novidade automática por combinar palavras conhecidas. Segundo, a literatura consultada não forneceu, até esta rodada, um sistema que reúna no mesmo núcleo local e soberano a promoção autorizada de memória, a não amplificação de proveniência, o conflito como abstention, a separação entre oferta e ação, o bloqueio pós-reboot e a proibição de o Core executar ações. Essa é uma lacuna provisória que precisa de revisão sistemática mais ampla, não uma reivindicação de prioridade.

## Contrato AGSC

```text
observação → memória → recuperação → oferta → ação local
      │          │          │          │          │
      └────────── proveniência não pode ser amplificada ──────────┘

conflito / expiração / falta de confirmação / epoch antigo → bloqueio
```

O contrato implementado mantém cinco propriedades centrais. A origem da observação continua visível quando ela é promovida. Promoção para memória exige confirmação física independente da origem. Recuperação não acrescenta autoridade nem escopo. Oferta não é ação. Ação exige confirmação física recente e somente pode usar escopo local explicitamente concedido; `AT_SCOPE_CORE_EXECUTE` é rejeitado.

A máquina de transição também vincula cápsulas a um epoch. Depois de reboot, uma oferta ou ação copiada da sessão anterior não pode recuperar autoridade por ter os mesmos campos semânticos. A nova sessão precisa reconstruir uma cadeia válida.

## Implementação e evidência host-side

O protótipo está em `sim/authority_transition.{h,c}` e é exercitado por `sim/authority_scenario.c`, usando o mesmo binário da bancada virtual que compila o firmware real sem cópia simulada. O cenário cobre fonte Core, promoção física, recuperação, oferta, ação local, escopo proibido, conflito, expiração e reboot.

| Evidência | Resultado |
|---|---:|
| Cenário AGSC | **16/16 propriedades** |
| Benchmark AGSC versus baselines | **7/7 propriedades** |
| Redteam AGSC | **8/8 mutantes mortos** |
| Redteam semântico anterior | **8/8 mutantes mortos** |
| Mutator global | **7/7 mutantes mortos** |
| Bancada virtual completa | **179 invariantes simuladas** |
| Regressão global | **78 suítes** |
| Proveniência | **válida; local não atestada** |

Durante a implementação, foram encontrados dois problemas que não foram mascarados como vitórias. Primeiro, o linker do simulador não incluía o novo módulo AGSC. Segundo, o redteam semântico antigo não incluía `authority_scenario.c` e produzia falhas de compilação, não mutantes mortos. Ambos os harnesses foram corrigidos; somente depois da recompilação completa as campanhas foram consideradas válidas.

## Falsificação e baselines

AGSC será falsificada se um baseline simples de recuperação atingir a mesma redução de memória falsa, conflito incorreto, ação não autorizada e intrusão com menor custo; se ataques de poisoning ou provenance laundering atravessarem a cadeia; se o custo de estado exceder o alvo; ou se usuários não conseguirem distinguir oferta, confirmação e ação.

A avaliação seguinte deverá comparar pelo menos: ausência de memória; último fato vence; recuperação por similaridade sem conflito; memória governada sem etapa de autoridade separada; e AGSC. As métricas principais serão precisão de promoção, memória falsa, preservação de conflito, resistência a stale facts, precisão de autoridade, abstention apropriada, custo de intervenção, vazamento, recuperação e custo físico. O número de invariantes continuará sendo um indicador de cobertura, nunca a medida principal de inteligência.

## Limites

Os resultados são exclusivamente host-side. Eles não provam funcionamento do ESP32-S3, DRV2605L, SX1262, NVS/flash sob brownout, consumo, latência elétrica, qualidade de percepção, compreensão humana da vibração, conforto, utilidade cotidiana ou aceitação social. Também não provam que AGSC é um paradigma novo no sentido histórico de Transformer.

A formulação honesta é mais forte que um slogan: **AGSC é uma hipótese arquitetural original do projeto, inspirada e restringida pela literatura, implementada como contrato local e ainda sujeita a prioridade acadêmica, baselines e contato humano.** Se sobreviver a essas comparações, poderá se tornar uma contribuição; antes disso, é uma direção promissora e falsificável.

## Referências

[1]: https://arxiv.org/abs/1706.03762 "Attention Is All You Need"

[2]: https://arxiv.org/abs/2005.11401 "Retrieval-Augmented Generation for Knowledge-Intensive NLP Tasks"

[3]: https://www.pnas.org/doi/10.1073/pnas.1611835114 "Overcoming catastrophic forgetting in neural networks"

[4]: https://arxiv.org/html/2212.00824v3 "On-device Training: A First Overview on Existing Systems"

[5]: https://arxiv.org/html/2603.15280v1 "Advancing Multimodal Agent Reasoning with Long-Term Neuro-Symbolic Memory"

[6]: https://arxiv.org/html/2606.28733v1 "Agentic Abstention: Do Agents Know When to Stop Instead of Act?"

[7]: https://arxiv.org/html/2602.04000v1 "After Talking with 1,000 Personas: Learning Preference-Aligned Proactive Assistants"

[8]: https://arxiv.org/abs/2512.06688 "PersonaMem-v2"

[9]: https://arxiv.org/abs/2305.10250 "MemoryBank"

[10]: https://www.nature.com/articles/s41467-025-67728-y "Wearable AI for on-device frailty assessment"

[11]: https://arxiv.org/html/2403.02135v1 "Memoro: Using Large Language Models to Realize a Concise Interface for Real-Time Memory Augmentation"

[12]: https://arxiv.org/html/2606.04329v1 "From Untrusted Input to Trusted Memory: A Systematic Study of Memory Poisoning Attacks in LLM Agents"

[13]: https://arxiv.org/abs/2607.29167 "Memory Provenance Laundering in LLM Agents"
