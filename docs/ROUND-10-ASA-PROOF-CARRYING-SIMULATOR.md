# Rodada 10 — Contrato canônico, verifier independente e executor de simulador

## Hypothesis

Um contrato canônico, um verifier independente e um executor somente de simulador rejeitam planos forjados, stale, divergentes, ambíguos ou acima do orçamento em modo fail-closed.

## Scope and authority boundary

A implementação é **host-only e simulator-only**, com autoridade **proposal-only** fora do simulador. O `SimulatorExecutor` não importa nem alcança `HostAdapter`, rádio, rede, subprocessos ou atuadores físicos. Efeitos externos e irreversíveis são bloqueados antes da mutação.

O planner produz somente uma sequência declarativa. O verifier produz uma atestação vinculante. O executor exige atestação válida e capability token escopado. O resultado não concede autoridade física.

## Protocol

O `SkillContract` contém schema e versão de contrato, identidade do host, digest, epoch, estado inicial, assinaturas ordenadas de ações, argumentos, pré-condições, pós-condições, delta derivado, irreversibilidade, orçamento, expiração e nonce.

Cada evidência é assinada por um trust root determinístico do simulador. O verifier recalcula o efeito a partir de `before` e `after`, verifica host, epoch, sequência, nonce, ação, argumento, pré-condição, pós-condição, objetivo e expiração.

O executor revalida escopo, atestação, token, identidade, estado e prazo antes de cada passo. Ele não possui fallback, replanejamento ou matching por `effect.delta`. Em falha parcial, timeout, reentrância ou divergência, restaura o snapshot interno e retorna uma razão estruturada.

## Provenance and split

Esta rodada é uma prova de protocolo de software em simulador determinístico. Não usa telemetria de produção nem dados externos. O resultado foi produzido por 12 testes adversariais novos, pela suíte de pesquisa existente e pelo gate oficial. Os hashes e resultados estão em `research/evidence/asa_round10_results.json`.

## Baselines

O baseline sem atestação retorna `ATTESTATION_REQUIRED`. Contratos com efeito externo retornam `EXTERNAL_EFFECT_BLOCKED` antes de qualquer mutação. O executor não implementa fallback ou replanejamento por desenho.

## Metrics with denominators

The `unsafe-decision rate` is reported explicitly and remains zero in the bounded simulator scope.

A campanha focada passou **12 / 12 testes**. A suíte completa passou **173 / 173 testes executáveis**, com **1 teste skipped**. O gate oficial passou com **40 / 40 suites declaradas no script**, incluindo a suíte ASA Round 10. Casos adversariais com decisão insegura foram **0 / 10 categorias adversariais**; cada categoria rejeitou, bloqueou ou restaurou estado conforme o protocolo. O numerator de decisões inseguras foi 0 e o denominator foi 10 categorias.

A métrica de execução aceita foi **1 / 1 sequência válida no simulador**. A métrica de efeitos externos permitidos foi **0 / 1 contrato externo**, pois o contrato foi bloqueado antes da mutação. Não há estimativa estatística de segurança física.

## Adversarial cases

Foram testados: efeito forjado, replay, epoch misto, colisão de ação, ausência de atestação, falha parcial, timeout, reentrância, divergência de estado, efeito externo e atestação expirada.

A campanha também verifica que planner não possui métodos de execução, que a evidência mantém assinatura coerente e que a identidade de ação não pode colidir silenciosamente por `action_id`.

## Results

Todos os 12 testes focados passaram. O executor válido completou exatamente `arm -> commit` no simulador, atingindo o objetivo declarado. Falhas parciais restauraram o estado interno inicial. Efeitos externos foram bloqueados antes da mutação.

O primeiro passe encontrou um bug real na construção de `Evidence`: a implementação usava nomes de campos do dataclass em vez dos valores. O verifier rejeitava a evidência válida com `HOST_EPOCH_SEQUENCE`, e os testes do executor falhavam em cascata por ausência de atestação. A construção foi corrigida explicitamente e a campanha foi repetida com 12/12.

A primeira execução agregada do `prove.sh` também encontrou dois FAILs de proveniência porque a integração alterou `prove.sh` sem atualizar seu digest protegido. O digest foi atualizado no manifesto; nenhum gate foi desabilitado. A execução final passou.

## Failures and corrections

Os dois primeiros problemas foram preservados como evidência: erro de construção da evidência e digest stale do manifesto. A correção do primeiro alterou somente a construção canônica de `Evidence`. A correção do segundo alterou somente o SHA-256 declarado para o novo `prove.sh`.

Não foram relaxados testes, removidos casos adversariais ou permitidos efeitos externos para obter o resultado final.

## Limitations

A autenticação usa um trust root determinístico destinado ao simulador, não uma infraestrutura de identidade de produção. O executor não cobre hardware, rádio, movimento, energia, desgaste, concorrência distribuída, sensores reais ou efeitos irreversíveis.

O caminho legado `SymbiontRuntime.transfer` não foi substituído nem autorizado por esta rodada. O resultado comprova apenas o novo módulo isolado. O `prove.sh` continua sendo um gate de software/host; seu sucesso não prova segurança física.

A rodada não testa AGI, compreensão geral, planejamento aberto, generalização externa, produção ou segurança física geral.

## Claim matrix

| Claim | Status | Evidence | Limitation |
|---|---|---|---|
| Contrato canônico rejeita schema, budget e colisões inválidos | PROVADO NO ESCOPO | `research/test_asa_round10.py` | Tipos e domínio do simulador |
| Verifier deriva efeito e rejeita replay/forgery | PROVADO NO ESCOPO | 12 testes focados | Trust root é local ao simulador |
| Executor restaura estado interno em falha parcial | PROVADO NO ESCOPO | Tests for fail/timeout/reentrancy/divergence | Não é rollback físico |
| Efeitos externos são bloqueados | PROVADO NO ESCOPO | `EXTERNAL_EFFECT_BLOCKED` | Não testa hardware real |
| Runtime legado de transferência é seguro | FALSO/REJEITADO | Auditoria Wide Research | Não satisfaz o contrato novo |
| Segurança física geral | NÃO TESTADO | Escopo simulator-only | Nenhum efeito físico executado |
| Generalização de produção | NÃO TESTADO | Proveniência local | Sem usuários, dispositivos ou telemetria |
| Inteligência geral | NÃO TESTADO | Hipótese bounded | Não é benchmark de AGI |

## Reproduction

```bash
PYTHONPATH=. python3 -m unittest research.test_asa_round10
PYTHONPATH=research python3 -m unittest discover -s research -p 'test_*.py'
./prove.sh --quiet
```

O relatório deve ser validado pela Skill:

```bash
python /home/ubuntu/skills/herus-asa-proof/scripts/validate_asa_report.py docs/ROUND-10-ASA-PROOF-CARRYING-SIMULATOR.md
```
