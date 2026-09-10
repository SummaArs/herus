# HERUS — Hardware Entry Gate v1

## Propósito

Este documento define a fronteira entre o que o HERUS já demonstra no host e o que só pode ser afirmado depois de uma bancada física. Nenhum ensaio físico promove automaticamente uma Skill a `AUTHORIZED` ou `ACTIVE`; o hardware apenas mede se a implementação restrita sobrevive ao mundo real.

> **Regra de passagem:** sem `./prove.sh --quiet` com `ALL INVARIANTS HOLD`, nenhum firmware pode ser gravado como release de bancada.

## Estado atual

O release permanece `pre_hardware`. O wire format host-only é determinístico e limitado, mas ainda não houve medição de memória, tempo de execução, alimentação, rádio, reset, persistência ou recuperação de falha em uma placa real.

## Critérios de aborto

O ensaio é abortado sem interpretação favorável quando a revisão da placa diverge do mapa de pinos, o perfil de rádio não é identificado, a calibração do instrumento está ausente, a base temporal não é declarada, um log contém conteúdo proibido, ocorre reset não observado ou qualquer campo obrigatório é preenchido com zero em vez de `null` quando o valor não foi medido.

| Código | Condição | Resultado obrigatório |
|---|---|---|
| `PROVE_BASELINE_FAILED` | A suíte host não passa | Bloqueado antes do flash |
| `BOARD_REVISION_MISMATCH` | A placa não corresponde ao mapa verificado | Bloqueado |
| `PIN_MAP_UNVERIFIED` | O mapa elétrico não foi confirmado | Bloqueado |
| `RADIO_VARIANT_MISMATCH` | O rádio real não corresponde ao perfil | Bloqueado |
| `INSTRUMENT_CALIBRATION_MISSING` | Instrumento sem calibração declarada | Bloqueado para energia |
| `CLOCK_BASIS_UNDECLARED` | Não há base temporal declarada | Bloqueado |
| `RAW_LOG_CONTAINS_FORBIDDEN_FIELD` | Log contém áudio, identidade, chave ou conteúdo | Rejeitado e descartado |
| `INTERRUPTION_NOT_OBSERVED` | A interrupção não foi realmente observada | Sem alegação de recuperação |
| `RESET_NOT_OBSERVED` | O reset não foi confirmado | Sem alegação de recuperação |
| `GATE_CRITERION_FAILED` | O critério quantitativo falhou | `fail`, nunca `pass` |

## Gate físico B1–B10

| Gate | Ensaio | Evidência mínima | Passa quando |
|---|---|---|---|
| B1 | Identidade da placa | revisão, marcação do MCU, esquema, self-test | A revisão real coincide com a configuração registrada |
| B2 | Mapa de pinos e boot | pin map, commit de firmware, log de boot | O firmware inicia sem pino não verificado |
| B3 | Wire round-trip | frame, digest, programa decodificado | O frame válido retorna ao mesmo programa |
| B4 | Rejeição de corrupção | frames adulterados, motivo de recusa | Toda corrupção conhecida é recusada sem efeito |
| B5 | Rádio a 1 m | perfil, enviados, recebidos, RSSI/SNR | A troca mínima ocorre no perfil declarado |
| B6 | Distância registrada | rota, distância, PDR, digest do log | O resultado é atribuído somente à rota testada |
| B7 | Energia e latência | instrumento, calibração, método, unidade | Cada valor tem unidade e workload declarado |
| B8 | Reset e perda de energia | método, reset observado, matriz de estados | O receptor volta a estado permitido, ou o gate falha |
| B9 | Autoridade e replay | revogação, replay, boot quarantine | Nenhum frame ou Skill cria autoridade por repetição |
| B10 | Sessão de evidência | record canônico, digest SHA-256 | O registro passa o schema e não contém conteúdo proibido |

## Registro mínimo

Cada execução usa um registro por gate com `gate_id`, revisão do protocolo, revisão da placa, adapter, marcações do MCU e rádio, commit do firmware, veredito `ALL INVARIANTS HOLD`, instrumento, calibração, método, unidade, tempo, contadores, medições disponíveis, interrupção, reset, digest do log, resultado e digest final do registro. Ausência é `null`, não zero. O digest é calculado sobre o JSON canônico sem o próprio digest.

## Critério de declaração

Mesmo que B1–B10 passem, a declaração permitida é limitada à placa, revisão, firmware, perfil de rádio, instrumento, rota e condições medidas. Não se pode declarar “seguro em geral”, “universal”, “à prova de falhas” ou “sucessor de LLMs” a partir de uma bancada.

## Próximo gate operacional

Antes da compra, o único trabalho pendente de decisão é confirmar a lista de peças compatível com o mapa de pinos e com o perfil de rádio do `hardware_readiness_manifest.json`. Depois da compra, o primeiro ensaio deve ser B1; nenhum teste de alcance, energia ou produto deve começar antes de B1 e B2 passarem.
