# HERUS — Readiness pré-hardware

## Estado

**Pronto para validação física controlada. Não pronto para operação crítica.**

Este documento registra o limite exato da evidência obtida sem hardware. O resultado não transforma simulação, análise estrutural ou fixture em prova de comportamento físico.

## Evidências host-only

| Gate | Resultado |
|---|---|
| Suíte completa de pesquisa | 201 testes passaram; 1 teste pulado |
| Firmware | `ALL FIRMWARE SUITES PASS` |
| Análise estática | GCC `-fanalyzer` sem defeitos no caminho alterado |
| Sanitizers | ASan/UBSan sem defeitos nos caminhos alterados |
| Simulação | 74 invariantes passaram; limitações físicas explicitamente impressas |
| Call paths | Sink `store_sealed` restrito a `memory_vault_seal`, com `auth_valid` e `card_valid` antes do sink |
| Extração estrutural | `EXTRACTED_MATCH`; falha de autenticidade em `memory_vault_open` exige bloqueio |
| Certificado composto | `ASSURED`; extração estrutural e call path auditado participam da promoção |
| Proveniência | Manifesto estrito válido; entradas locais não assinadas permanecem classificadas |
| CI remoto | Ubuntu, macOS e CodeQL passaram no workflow `prove` do commit `a82156d` |

## Correções que fecharam a rodada

O auditor de call paths deixou de verificar apenas a identidade do caller. Perfis agora podem declarar guards obrigatórios, e o auditor exige que cada guard exista no wrapper permitido e ocorra antes do sink. O caso do memory vault passou a executar esse auditor real a partir de `research/memory_vault_call_path_profile.json`; a lista declarativa de call paths não é mais a única evidência.

O resultado serializado do assurance inclui o veredicto e a razão da extração estrutural. A política de promoção bloqueia o certificado quando a extração não retorna `EXTRACTED_MATCH`.

## O que não foi provado

Ainda não há evidência física de power-loss, brownout, atomicidade de flash ou NVS, secure boot, debug bloqueado, raiz protegida, eFuse, contador monotônico durável, desgaste, energia, latência, capacidade térmica, observabilidade do alvo ou comportamento de sensores e rádio em ambiente real. A simulação de distância, propagação e consumo continua sendo modelo.

Também não foi provado que o HERUS substitui LLMs, realiza raciocínio livre generativo geral ou possui segurança operacional para sistemas críticos. O escopo demonstrado é um núcleo finito de coordenação e assurance com bloqueio fail-closed e intervenção humana explícita.

## Decisão

O HERUS está **pronto para ser confrontado pelo hardware**. A próxima etapa deve ser uma bancada reversível e instrumentada. Nenhum sink crítico deve ser habilitado antes de validar reset, armazenamento, falhas de energia, observabilidade e autoridade no alvo. Um resultado físico positivo deve ser tratado como evidência específica daquele alvo, firmware, configuração e protocolo; não como garantia geral.

## Proveniência

Commit de integração: `a82156d`.

Workflow remoto: `prove`, run `33684843309`, com `prove (ubuntu-latest)`, `prove (macos-latest)` e `CodeQL (C/C++)` concluídos com sucesso.
