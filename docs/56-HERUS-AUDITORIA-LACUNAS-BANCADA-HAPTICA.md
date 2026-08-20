# HERUS — Auditoria de lacunas para a bancada HAP-SEM

**Estado da auditoria:** host-only, baseada no commit publicado da branch `feat/herus-semantic-compiler`.  
**Conclusão:** o contrato semântico, o adaptador host-portátil, o validador privado e o port target I²C já existem e passaram a prova global 52/52; o board map mantém o haptic target desabilitado e nenhuma medição física foi realizada. Nenhum gate físico foi aprovado.

## 1. Estado observado

| Camada | Existe hoje | Evidência | Limite atual |
|---|---|---|---|
| Semântica HAP-SEM | encoder/decoder C11, profiles ERM/LRA, checksum, fragmentação e profile-bound decode | `firmware/core/haptic_language.{h,c}`; suites 17/17 e matriz 49/49 | prova estrutural em host; não mede waveform nem percepção |
| Ponte de autoridade | `hs_signal_t` com `confirmation_required`, `abstained` e `actionable=0` | `firmware/core/haptic_semantic_bridge.{h,c}`; suíte 9/9 | não existe emissão física; comportamento no pulso ainda não medido |
| ESP32-S3 | HAL de oito funções, SPI/GPIO para SX1262, console, self-test de rádio e port I²C target type-checked | `firmware/port/hal.h`, `firmware/port/esp32s3/hal_esp32s3.c`, `haptic_drv2605l_esp32s3.{h,c}`, `app_main.c` | `BOARD_HAS_HAPTIC_I2C=0`; não há probe físico, calibração, leitura real de status ou comando de waveform |
| Pin map | `PIN_I2C_SDA=18` e `PIN_I2C_SCL=17` estão reservados na placa | `firmware/port/esp32s3/board_t3s3.h` | os pinos são comentário de OLED/ATECC608A, não uma confirmação do hardware da revisão real |
| Instrumentação | schema genérico e extensão HAP-SEM para profile, atuador, calibração, waveform, energia, I²C e segurança | `research/haptic_bench_evidence_schema.json` e `tools/haptic_bench_evidence_validate.py` | ainda não há qualquer registro físico real |
| Prontidão | gates de board, rádio, energia, interação, segurança e modelo | `research/hardware_readiness_manifest.json` | o gate HAP-SEM ainda precisa ser promovido de contrato para gate físico executado |
| Protocolo | exige revisão, instrumento identificado, baseline verde, `null` distinto de zero e `blocked_by_missing_evidence` | `docs/36-PROTOCOLO-BANCADA-FISICA.md` | interação é genérica; não especifica a sequência HAP-SEM/DRV2605L |
| Privacidade | proíbe áudio, transcrição, embedding, identidade, localização, chaves e conteúdo | schema e manifesto atuais | a futura telemetria háptica deve registrar somente códigos técnicos e medidas agregadas |

## 2. Lacunas críticas, em ordem

A primeira lacuna continua elétrica e de integração no target real: o port ESP32-S3 agora contém uma implementação baseada no novo driver I²C master, mas `BOARD_HAS_HAPTIC_I2C=0` e `PIN_HAPTIC_ENABLE_VALID=0` bloqueiam sua execução por padrão. O CI exercita o caminho apenas com override explícito de compilação e stubs; isso não permite afirmar que um `effect_id` foi reproduzido nem que ERM/LRA foi configurado corretamente. No host, o contrato `haptic_adapter.{h,c}` já exercita a ordem de transações, busy, abort e faults. A segunda lacuna é de calibração física: o schema agora registra profile, revisão do atuador, amplitude/acceleration, duração, corrente, energia, temperatura e I²C, mas nenhum desses campos foi medido em hardware. A terceira é de evidência: o validador rejeita profile ausente, I²C não-zero, segurança não observada, digest inválido e conteúdo proibido, mas ainda não existe um registro físico real para aceitar.

A quarta lacuna é de autoridade: a futura camada física não pode aceitar um frame recebido ou uma resposta do driver como confirmação de ação. O driver deve ser um atuador de apresentação com estado de erro e pronto, nunca uma fonte de autorização. A quinta é de segurança de bancada: antes de vibrar no pulso, deve existir fixture de mesa, limite de corrente, corte físico, verificação de temperatura e caminho para interromper a saída independentemente do firmware.

## 3. Decisão de arquitetura

O próximo incremento deve ser **um adaptador DRV2605L pequeno, síncrono e fail-closed**, separado do compilador semântico. Ele deve aceitar apenas um `hl_encoded_t` já validado contra um `hl_profile_t`, traduzir os effect IDs para transações I²C declaradas e devolver estado técnico (`ready`, `played`, `fault`, `timeout`), sem produzir `ACK`, sem mudar memória e sem enviar rádio.

No host, o mesmo contrato deve ser exercido por um mock bus que registre endereço, escrita, ordem, timeout e erro, mas nunca alegue saída física. No target, a implementação I²C deve ficar no port ESP32-S3; o core continua portátil. A calibração e a telemetria devem emitir registros privados compatíveis com o schema, com digest do log e valores `null` quando o instrumento não existir.

## 4. Sequência segura proposta

| Ordem | Incremento | Gate que continua bloqueando |
|---:|---|---|
| 1 | contrato C11 do bus I²C e do adapter, com mock adversarial | nenhum hardware ligado |
| 2 | schema/regras de evidência háptica e gate `haptic-bring-up` | profile, atuador e instrumento ausentes |
| 3 | driver target I²C com init, standby, profile e erro | pin map e revisão da placa pendentes |
| 4 | fixture de mesa com DRV2605L e atuador fora do corpo | corte físico e calibração pendentes |
| 5 | medição elétrica e acelerométrica de cada efeito | nenhum claim de distinguibilidade |
| 6 | teste humano segundo `docs/55-HERUS-PROTOCOLO-VALIDACAO-HAPTICA.md` | ética, segurança e matriz de confusão pendentes |

A ordem não permite pular do encoder host para “vibração correta no pulso”. O primeiro alvo realista é demonstrar uma transação I²C observável, com o atuador em fixture de mesa e o firmware em estado seguro. O comando target `haptic-probe` foi criado para presença elétrica sem playback, mas só será habilitado depois da confirmação do schematic/pin map. Somente depois devem ser estudadas amplitude, duração, conforto ou reconhecimento.

## 5. Evidência host-only adicionada

O adaptador `firmware/core/haptic_adapter.{h,c}` e `test_haptic_adapter.c` passaram **17/17** invariantes em C11 estrito. A suíte cobre validação profile-bound antes do primeiro write, ordem `MODE → WAVEFORM → GO`, polling sem falso término, abort para standby, erro de bus, fault sem retry automático e rejeição de frame fragmentado. O validador de evidência `tools/haptic_bench_evidence_validate.py` passou **7/7** testes adversariais contra digest, conteúdo proibido, enum, I²C com erro, parada não observada e bloqueio sem motivo. O port `haptic_drv2605l_esp32s3.{h,c}` passou a suíte default/override **4/4 + 4/4** e o type-check do app ESP32-S3. Essas são provas de software e stubs; não são resultados do DRV2605L.

## 6. O que não foi alegado

Esta auditoria não afirma compatibilidade da revisão real da LilyGO T3-S3, não afirma que GPIO 17/18 estejam livres, não afirma que o DRV2605L esteja montado, não afirma que qualquer número da biblioteca tenha significado HAP-SEM, não afirma energia, latência, WER, conforto ou segurança física. O estado correto permanece `pre_hardware` e `local_unattested`.

## Referências internas

- [`firmware/core/haptic_language.h`](../firmware/core/haptic_language.h)
- [`firmware/core/haptic_semantic_bridge.h`](../firmware/core/haptic_semantic_bridge.h)
- [`firmware/port/hal.h`](../firmware/port/hal.h)
- [`firmware/port/esp32s3/board_t3s3.h`](../firmware/port/esp32s3/board_t3s3.h)
- [`research/hardware_bench_evidence_schema.json`](../research/hardware_bench_evidence_schema.json)
- [`research/hardware_readiness_manifest.json`](../research/hardware_readiness_manifest.json)
- [`docs/36-PROTOCOLO-BANCADA-FISICA.md`](36-PROTOCOLO-BANCADA-FISICA.md)
- [`docs/55-HERUS-PROTOCOLO-VALIDACAO-HAPTICA.md`](55-HERUS-PROTOCOLO-VALIDACAO-HAPTICA.md)
