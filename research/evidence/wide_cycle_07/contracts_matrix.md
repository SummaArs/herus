# Matriz de contratos e autoridade — HERUS

## Superfícies de contrato

| Superfície | Arquivo principal | Estabilidade | Evolução permitida |
|---|---|---|---|
| **HCP Wire** | `docs/02-PROTOCOL.md` | **Imutável** | Novos papéis (P4); extensões de pad (E-P1) |
| **Semantic IR** | `semantic_ir.schema.json` | **Migrável** | O schema exige `schemaVersion=1`, `additionalProperties=false` e enum fechado; novos eventos/campos exigem versão/migração explícita |
| **Voz/Comandos** | `voice.h` | **Compatível por extensão limitada** | Novas frases podem ser aditivas dentro da gramática; nova classe de comando exige adaptador/versionamento, pois consumidores antigos não a conhecem |
| **Persistência** | `memory_collection.h` | **Migrável** | Mudança de formato exige nova versão e migração |
| **Core-Link** | `core_link.h` | **Compatível** | Extensão de payload mantendo tamanho de wire |
| **Interação** | `interaction.h` | **Compatível** | Novos estados de UX ou métricas |
| **Proveniência** | `software_provenance_manifest.json` | **Experimental** | Mudança de schema ou algoritmo de digest |

## Caminhos de autoridade

| Autoridade | Onde nasce | Onde é bloqueada | Onde é confirmada |
|---|---|---|---|
| **Proposta** | Adaptador (ASR/Sensor) | `intent_gate.c` | N/A |
| **Draft** | `voice.c` | `interaction.c` | N/A |
| **Envio (HCP)** | `interaction.c` | `assurance.c` | **Física (PTT)** |
| **Escrita (Cofre)** | `memory_consolidation.c` | `memory_vault.c` | **Física (Review)** |
| **Recuperação** | `memory_retrieval.c` | `memory_collection.c` | **Física (Session)** |
| **Telemetria** | `interaction.c` | `interaction.h` | N/A (Redacted) |

## Fronteiras de composição

1. **Adaptadores:** Podem ser substituídos ou adicionados (ex: nova LLM local) desde que terminem em `intent_observation_t` ou `IndependentAnnotation`. Não possuem autoridade sobre rádio ou cofre.
2. **Extensões:** Podem adicionar novos papéis ao HCP sem quebrar receptores antigos (P4).
3. **Mecanismos:** Podem evoluir internamente (ex: novo algoritmo de matching) se preservarem o contrato de Semantic IR e a exigência de confirmação.
4. **Calibração:** Parâmetros de limiar e orçamento podem ser ajustados por aprendizado local sem alterar a estrutura do núcleo.
