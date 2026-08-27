# Síntese da caça paralela — Wide Research 08

As cinco frentes foram executadas independentemente sobre o baseline do commit `cad734c298719bc376b0b997f5b85366dd3278d7`.

| Frente | Verificações relevantes | Resultado |
|---|---|---|
| Parser/interação | ocorrência de comandos, estados `DRAFT`/`AWAIT_CONFIRM`, transcript, PTT, confirmação e timeout | Encontrada lacuna fora do parser: hint contextual sem teto de confiança; confirmação física permaneceu obrigatória |
| Semantic IR/adapters | schema versionado, enums, `additionalProperties`, `PROPOSAL_ONLY`, estados de abstenção | Fechado; nenhuma rota externa para autoridade |
| Core-Link/HCP | versão, comprimento, replay, expiração, par, direção, P4 e papéis desconhecidos | Fechado nos casos auditados; versão/comprimento já tinham regressões no ciclo 07 |
| Cofre/coleção/recuperação | floor, geração, prepared/committed, rollback, sessão e bloqueio | Fechado nos oráculos existentes; sem migração persistente entre versões alegada |
| Release/proveniência | inputs protegidos, `source_data`, strict, hashes e redaction | Nenhuma dependência nova; evidência e stage serão auditados novamente |

O achado promovido para reprodução foi `intent_context_hint_t.confidence_pct=255`: o código aceitava o valor porque verificava somente o piso. `available=2` já era rejeitado. O patch mínimo adicionou o teto `confidence_pct <= 100u`; a regressão anterior falhou e a posterior passou.

Os outputs intermediários de cada frente foram consumidos nesta síntese e serão removidos antes do stage; a evidência publicada mantém somente o resumo consolidado, os logs de reprodução antes/depois e os gates finais. Nenhum deles contém corpus, mídia, credenciais ou URLs assinadas.
