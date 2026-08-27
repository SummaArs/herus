# Wide Research 07 — baseline

## Estado

O baseline foi executado a partir do commit local e remoto `ff84ce6c16681e0e9115d4c96584d2edeca1ec64`, na branch `main`, com working tree limpo. O remoto foi consultado via GitHub API e coincidiu com o commit local.

## Gates

| Gate | Resultado |
|---|---|
| `make -C research test` | 93 testes, `OK` |
| `make -C firmware all` | `ALL FIRMWARE SUITES PASS` |
| `make -C firmware analyzer` | GCC analyzer sem defeitos no caminho analisado |
| `make -C firmware sanitizers` | ASan/UBSan sem defeitos nos caminhos analisados |
| `make -C sim sanitizers` | Suíte do simulador passou; alcance, propagação e energia continuam modelados |
| `./prove.sh --quiet` | `ALL INVARIANTS HOLD`; gates físicos continuam pendentes |
| JSON | Manifestos válidos |
| Proveniência | Manifesto válido: 1 gate ativo, 3 pendentes, inputs locais não atestados |
| `git diff --check` | Sem alterações no baseline |

Este resultado confirma contratos host e simulador. Não constitui medição de ESP32-S3, SX1262, RF, energia, temperatura, UX ou ASR embarcado.

## Ponto de partida arquitetural

O HERUS já separa, em comentários e APIs públicas, parser e Semantic IR de confirmação; adaptadores de ASR de transporte; cofre e coleção de cartões de candidatos; e contrato de persistência de backend. O protocolo HCP já declara invariantes de airtime, dwell, símbolos transmitidos, compatibilidade futura por papéis desconhecidos e replay/deduplicação. O próximo passo deve testar se essas separações formam uma política real de evolução, em vez de assumir que modularidade nominal garante longevidade.
