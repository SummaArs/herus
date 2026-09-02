# Auditoria da proposta HERUS Critical Assurance Engine V5

## Decisão

A proposta V5 é útil, mas não deve ser copiada como um segundo runtime de autoridade nem como um produto paralelo. O núcleo aproveitável é uma camada host-only de assurance orientada à superfície crítica: requisitos explícitos, sinks declarados, guardas observáveis, evidência com digest, mutação e veredicto fail-closed.

A implementação desta rodada usa os sinks reais do HERUS. Ela não introduz `authority_gate.c`, `persist_state` ou `publish_message` fictícios, porque esses nomes não representam a arquitetura atual. A autoridade continua no fluxo existente de `assurance_decide`, confirmação física, revogação, `memory_vault` e enlace confiável.

## Classificação das ideias da V5

| Elemento V5 | Decisão | Motivo |
|---|---|---|
| Pipeline único PROVE/EVIDENCE/CHALLENGE | Incorporado como princípio | Alinha-se aos gates e à proveniência existentes sem criar runtime novo |
| Critical sink inventory | Implementado | Fecha a lacuna entre teste isolado e superfície de execução crítica |
| Guarda por sink | Implementado lexicalmente | Já produz `COVERED`, `UNCOVERED` e `UNKNOWN`; ainda não é prova de dominância |
| Mutação | Já existente no repositório; mantida | `tools/test_proof_fire_mutations.py` já testa remoção de controles reais |
| Freshness/digest | Já existente; integrado por proveniência | `tools/provenance_audit.py` já rejeita mismatch e inputs ausentes |
| CBMC obrigatório | Não ativado automaticamente nesta rodada | A ferramenta pode estar ausente; transformar sua ausência em PASS seria incorreto. O perfil futuro deve retornar `BLOCKED` |
| AST/interprocedural dominance | Reservado como próximo backend | Regex não prova todos os caminhos, chamadas indiretas ou pré-condições condicionais |
| HCAE como autoridade de envio | Rejeitado | Violaria a arquitetura HERUS e criaria divergência entre verificador e runtime |
| Certificação automática ISO/DO-178C/IEC 61508 | Rejeitado | A proposta corretamente declara que isso não pode ser alegado por um gate local |
| Dashboard/assurance case completo | Adiado | Não aumenta a força da prova antes de fechar a semântica de sinks e evidências |

## Implementação

O perfil `research/hcae_profile.json` declara quatro sinks existentes:

| Sink lógico | Função | Operação observada | Guardas exigidas |
|---|---|---|---|
| `interaction-send` | `interaction_take_send_assured` | `interaction_take_send(` | `assurance_decide(` |
| `memory-persist` | `memory_vault_seal` | `store_sealed(` | `auth_valid(` e `card_valid(` |
| `memory-review-persist` | `memory_consolidation_confirm_store` | `memory_vault_seal(` | `access_valid(` e sessão física compatível |
| `nucleus-seal` | `trust_seal_nucleus_intent` | `core_link_seal_nucleus_intent(` | `active_control_key(` |

O auditor `research/critical_sink_audit.py` extrai o corpo da função, verifica a presença da operação e exige que cada guarda declarada apareça antes dela. A classificação é deliberadamente limitada:

> `COVERED` significa cobertura lexical e ordenação local observável; não significa dominância interprocedural provada.

Quando o arquivo, a função ou a operação não existem, o resultado é `UNKNOWN`. Quando a guarda falta ou ocorre depois da operação, o resultado é `UNCOVERED`. Em ambos os casos, o comando encerra sem `PASS`.

## Evidência negativa obrigatória

Os testes cobrem o perfil real, arquivo ausente, guarda removida e guarda posicionada depois do sink. Isso evita o defeito que a V5 identifica: testar apenas o programa correto e nunca demonstrar que o assurance engine rejeita uma regressão.

A mutação real dos controles do firmware continua separada e é executada pelo gate existente. O auditor lexical não é apresentado como substituto de mutação nem como substituto de análise AST.

## Lacuna restante

A maior limitação atual é semântica, não de interface. A busca textual pode encontrar uma guarda no corpo sem provar que ela domina todas as chamadas do sink, especialmente com condicionais, retornos antecipados, chamadas indiretas, macros ou aliasing. O próximo aumento de rigor deve substituir ou complementar o extrator lexical por AST e grafo de chamadas, retornando `UNKNOWN` quando não houver prova de dominância.

Também permanece necessário ligar cada sink a uma claim e a evidências frescas específicas. O manifesto geral de proveniência já protege hashes de arquivos, mas ainda não é uma claim graph HCAE completa. Essa extensão deve reutilizar o ledger atual, não criar um modelo concorrente de confiança.

## Conclusão

A V5 contém uma direção correta: verificar a superfície crítica e não apenas funções isoladas. A contribuição real incorporada nesta rodada é um primeiro contrato executável para essa direção, aplicado ao código HERUS existente e com falha fechada. O resultado não é certificação, não é prova de dominância e não é estado da arte; é uma base verificável para o próximo backend interprocedural.
