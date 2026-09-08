# HERUS — Ciclo 08: significado verificável e release candidate host-only

**Estado:** `HOST_RELEASE_CANDIDATE`

**Escopo:** branch de endurecimento empilhada sobre a camada do significado e HSCA. `main` não foi alterada. Nenhuma propriedade física é declarada como provada por este documento.

## Tese congelada nesta rodada

O HERUS trata uma intenção formalizada como uma proposta limitada por domínio, propósito, autoridade, contexto, condições, efeitos permitidos, delegação e evidência. O contrato mínimo está em `docs/25-significado/201-HERUS-CONTRATO-MINIMO-DE-INTENCAO.md` e no schema `research/intent_envelope_v0.schema.json`.

O schema valida forma. Ele não concede autoridade nem prova semântica por si só. A autoridade continua sendo uma decisão do HSCA e dos contratos de domínio. Essa separação é intencional.

## Correções e extensões executadas

O workflow agora usa `pipefail`, de modo que uma falha real de `prove.sh` não pode ser mascarada por `tee`. O comando bruto `cmd_send` do alvo ESP32 permanece fail-closed até existir uma cadeia Babel→HSCA→Aether ligada ao alvo; a contraprova `research/test_meaning_transport_bypass.py` impede a reintrodução do bypass.

O Loom agora aplica efetivamente o orçamento `table_bytes` sobre o artefato gerado. A contraprova `tools/test_loom_envelope.py` reduz artificialmente o envelope e exige falha. A linguagem da propriedade L5 foi restringida ao conjunto efetivamente enumerado pelo gate, em vez de alegar exaustão de todo HIR possível.

Foi criada a cadeia C11 `Babel → HIR → Aether → HIR → Babel` em `firmware/core/meaning_pipeline.c`. O transporte semântico mantém `authority_granted = 0`; significado e autoridade não são confundidos.

Foi criado um probe C e uma referência Python independente. A suíte `research/test_meaning_interoperability.py` compara frame Aether, wire HIR e renderização em três vetores reais do corpus. Uma divergência entre C e Python falha o gate.

A demonstração `tools/demo_meaning_pipeline.py --check` mostra uma intenção, seu HIR canônico, um frame de transporte, a reconstrução no idioma de destino e a rejeição de uma adulteração de cinco bytes. Ela usa probes host-only; não é transmissão por rádio.

## Evidência reproduzida

| Prova | Resultado |
|---|---|
| `prove.sh --quiet` | `ALL INVARIANTS HOLD` |
| Ledger global | 100/100 etapas concluídas |
| Proveniência estrita | Manifesto válido; 1 entrada ativa e 3 pendentes |
| Pipeline C11 | 7 invariantes passaram |
| Interoperabilidade C↔Python | 3 vetores: frame, HIR e render iguais |
| Aether | 23 testes; canal adversarial sem entrega errada |
| Aether red-team | 12/12 mutantes detectados |
| Babel | 59 testes; 0 falhas |
| Babel cross | 681.530 comparações C↔referência; 0 divergências |
| Babel red-team | 16/16 mutantes detectados |
| Web parity | 1.637 vetores reproduzidos; 0 divergências |
| Loom núcleo + campo | 157 conceitos; 370.128 idas-e-voltas; 22.348 B de tabelas |
| Loom envelope | 21.581 B medidos; envelope subdimensionado rejeitado |
| Documentação | 129 documentos; 446 links; 0 quebrados; 0 órfãos na hierarquia |
| Simulação | 276 invariantes do sistema passaram |

## Demonstração observada

Entrada humana host-only:

`emergencia estou aqui`

O pipeline produziu um HIR de 24 bytes e um frame Aether de 33 bytes. O receptor reconstruiu:

`全員に着いた救助`

Uma adulteração deliberada em cinco bytes produziu:

`REJEITADO (UNCORRECTED)`

Esse resultado prova a propriedade local do demonstrador: dentro do perfil aberto simulado, quadro corrompido acima da capacidade de correção não vira outro significado aceito. Não prova que um microfone, alto-falante, LED, rádio ou antena reais reproduzirão a mesma cadeia.

## O que este ciclo prova

O HERUS agora possui uma demonstração host-only que conecta um significado canônico a um transporte limitado e a uma recusa verificável. A implementação C e a referência Python concordam nos vetores selecionados. A camada de significado não concede autoridade implícita. A regressão de envio bruto do console está bloqueada. O orçamento do Loom é observado e testado.

## O que este ciclo não prova

O ciclo não prova secure boot, debug lock, eFuse, raiz criptográfica física, power-loss, brownout, atomicidade real de flash, desgaste, energia, latência ou transmissão pelo ar. Também não prova interoperabilidade completa entre duas implementações independentes do envelope de intenção v0; a paridade atual cobre frame Aether, HIR e renderização em um subconjunto real do corpus.

O envelope JSON é uma especificação de forma. Ainda falta ligá-lo integralmente ao HIR e ao certificado HSCA sem permitir que campos de autoridade, efeitos ou evidência sejam aceitos apenas porque são sintaticamente válidos.

## Decisão de release candidate

O HERUS está pronto para ser tratado como **release candidate host-only de demonstração**, não como produto físico ou sistema crítico em operação. A próxima confrontação de maior valor é física: dois dispositivos, um canal real e uma sequência controlada de aceitação, corrupção, perda, reset e recuperação. Essa confrontação só deve começar depois de preservar este estado como baseline.
