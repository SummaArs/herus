# Compatibilidade, migração e reversibilidade — Wide Research 07

## Resultado

Os contratos reais existentes foram exercitados sem alterar a autoridade do firmware. A suíte completa do firmware passou após a adição de duas regressões Core-Link: wire truncado/estendido e versão futura. A suíte de pesquisa específica passou com 22 testes, incluindo Semantic IR, política e inventário de compatibilidade.

| Superfície | Ensaio | Resultado |
|---|---|---|
| Semantic IR | `schemaVersion` diferente, propriedades desconhecidas, enum fechado, `PROPOSAL_ONLY`, contradição e limiares | Rejeição ou proposta sem autoridade |
| HCP | versão desconhecida, bits reservados e papel desconhecido | Versão/bits inválidos rejeitados; P4 ignora papel desconhecido sem rejeitar o frame |
| Core-Link | comprimento diferente de 43 bytes | `CORE_LINK_E_ARG`, saída zerada e sequência RX inalterada |
| Core-Link | versão futura no header | `CORE_LINK_E_FORMAT`, saída zerada e sequência RX inalterada |
| Core-Link | autenticação, par, direção, replay e expiração | Suíte existente passou; falhas não promovem observação |
| Coleção | floor stale, geração pulada, registro não autenticado, preparado órfão e presença não canônica | `BLOCKED` ou descarte determinístico; nenhum estado inseguro é promovido |
| Interação | ASR antes de PTT, draft sem confirmação, confirmação única, timeout e handoff | Transições preservam `AWAIT_CONFIRM` e envio one-shot |

## Falha de execução corrigida

A primeira execução do subconjunto de pesquisa a partir da raiz falhou por contexto de import (`semantic_ir` não estava no `PYTHONPATH`). O comando correto da suíte é executado dentro de `research`, como o Makefile já faz. A reexecução nesse contexto passou com 22 testes. Isso foi uma falha do comando de ensaio, não uma falha do contrato; o diagnóstico foi mantido somente durante a execução e não contém dados externos.

## Limite encontrado

O firmware possui recuperação transacional e versionamento de coleção, mas não possui uma migração de formato persistente entre duas versões de produto para testar como implementação concreta. Os testes atuais exercitam a fronteira real disponível: versão atual, registros inválidos, floor, prepared/committed, rollback e bloqueio. Portanto, este ciclo **não alega migração persistente já implementada**. Uma futura mudança de formato deverá adicionar fixtures de versão anterior, migração autenticada, rollback e interrupção antes de ser promovida.

A alteração de runtime deste ensaio foi mínima: somente testes Core-Link foram adicionados. Nenhuma nova rota de memória, rádio, confirmação ou adapter recebeu autoridade. A política resultante classifica Semantic IR e persistência como `VERSIONED_MIGRATION`, HCP como base imutável com extensão P4 específica e voz/interação como extensões limitadas.
