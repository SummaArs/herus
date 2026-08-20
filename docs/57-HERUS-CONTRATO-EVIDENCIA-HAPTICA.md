# HERUS — Contrato de evidência HAP-SEM/DRV2605L v1

**Estado:** contrato de instrumentação host-only; não representa uma medição já realizada.  
**Schema:** [`research/haptic_bench_evidence_schema.json`](../research/haptic_bench_evidence_schema.json).  
**Base:** [`research/hardware_bench_evidence_schema.json`](haptic_bench_evidence_schema.json) e [`docs/36-PROTOCOLO-BANCADA-FISICA.md`](36-PROTOCOLO-BANCADA-FISICA.md).

## 1. Pergunta do contrato

O contrato responde a uma pergunta estreita: **o firmware e o adaptador conseguem comandar um DRV2605L identificado, com um profile ERM/LRA identificado e calibrado, registrando transações I²C, saída física e segurança em um fixture fora do corpo, sem transformar a reprodução em autoridade?**

Ele não responde se o padrão é percebido por uma pessoa, se é confortável, se tem baixo WER, se economiza energia em um produto ou se funciona em qualquer revisão de placa. Essas perguntas permanecem gates separados.

## 2. Ordem de evidência

A bancada deve seguir a ordem abaixo. Um resultado posterior não pode compensar um bloqueio anterior.

| Ordem | Gate | Evidência mínima | Bloqueio explícito |
|---:|---|---|---|
| 0 | baseline | `./prove.sh --quiet` com `ALL INVARIANTS HOLD` | qualquer falha do host |
| 1 | board identity | revisão da placa, marcação do MCU, esquema e pin map | revisão ou GPIO ambíguo |
| 2 | bus safety | continuidade, alimentação limitada, níveis lógicos, corte físico e I²C observável | ausência de corte ou barramento não observado |
| 3 | driver identity | marcação DRV2605L, endereço I²C, init/standby e status | endereço errado, ausência de ACK ou driver não identificado |
| 4 | profile calibration | ERM/LRA, revisão do atuador, profile versionado e instrumento calibrado | profile ausente ou calibração não reproduzível |
| 5 | fixture playback | `hl_encoded_t` validado, transações I²C, waveform medida e sem erro | frame não validado, I²C falho, output ausente ou segurança ausente |
| 6 | electrical record | tensão, corrente, energia, temperatura, duração e método | unidade, amostragem ou instrumento ausente |
| 7 | human study | protocolo `docs/55-HERUS-PROTOCOLO-VALIDACAO-HAPTICA.md` | percepção confundida com resultado de bancada |

O primeiro ensaio físico deve usar um fixture de mesa. `fixture_only` deve ser `true` até que o gate elétrico, o corte independente e a avaliação de segurança permitam uma campanha separada. Nenhuma medição no pulso é implícita pela aprovação do fixture.

## 3. Campos e relações

O schema usa campos com unidade no nome para reduzir ambiguidade. `accel_rms_mps2` é a aceleração RMS medida no ponto de contato definido pelo fixture; `pulse_duration_us` e `inter_slot_pause_us` são valores observados, não apenas valores programados; `supply_current_ma` e `energy_uj` pertencem somente ao workload declarado; `latency_us` deve declarar no método se começa no pedido do core, no início da transação I²C ou no primeiro movimento observado.

`i2c_error_count` deve ser zero para qualquer resultado `pass`. Um erro que não foi observado não pode ser registrado como zero; quando a medição não foi feita, o valor é `null` e o resultado deve ser `blocked_by_missing_evidence`. Um registro `pass` ou `fail` precisa de `raw_log_digest`; o raw log permanece fora do repositório e é auditado somente pelo digest e pelo envelope de privacidade.

`haptic_profile_version` identifica a tabela que liga tokens semânticos a efeitos físicos. O valor nunca é significado universal por si só: o profile deve incluir atuador, revisão e condições de calibração no caderno privado da bancada. O frame semântico é validado pelo core antes de chegar ao adaptador; o adaptador não pode aceitar dados truncados, profile incompatível ou fallback de efeito.

## 4. Segurança e autoridade

O adaptador deve ter uma máquina de estados mínima: `IDLE`, `VALIDATED`, `PLAYING`, `DONE`, `FAULT` e `ABORTED`. Apenas `VALIDATED` pode iniciar playback; `FAULT` e `ABORTED` devem impedir novas transações até reset seguro. O corte físico deve interromper o atuador sem depender de um callback do core.

A saída do driver é apresentação técnica. `DONE` não significa confirmação semântica, não altera `hs_signal_t`, não grava memória, não chama rádio e não executa plano. Uma falha do driver também não deve produzir `ACK`; deve produzir erro técnico e, na ponte, permanecer em estado de abstinência ou pendência conforme a política já provada.

O campo `safety_stop` não certifica segurança por si mesmo: ele registra se a capacidade de parada foi testada na execução. Se a parada não foi testada, o valor é `null` e o gate fica bloqueado. A temperatura, corrente e duração devem ser comparadas com limites pré-comprometidos da bancada, não escolhidos depois de ver a saída.

## 5. Privacidade e armazenamento

O registro público contém somente revisões, códigos de gate, valores numéricos, unidades, contagens, falhas categóricas e digests. Não entram áudio, fala, transcrição, texto livre, identidade, localização, embeddings, chaves, conteúdo de mensagem ou identificadores de participante. Para o futuro estudo humano, o código temporário da sessão deve ficar fora deste repositório e não pode aparecer no campo `free_text` ou em um raw log público.

A ausência de conteúdo não significa ausência de evidência: o raw log privado pode ser preservado pelo responsável da bancada durante o prazo definido no protocolo, mas apenas seu digest e seus metadados técnicos entram no registro verificável. Um digest não permite recuperar o conteúdo, mas também não prova sozinho que o instrumento estava calibrado; por isso, calibration due, método e revisão continuam obrigatórios.

## 6. Critérios de resultado

`pass` significa que o baseline passou, o fixture estava isolado, o profile e o atuador foram identificados, a calibração foi registrada, o I²C não teve erro, a saída foi medida, o corte foi disponível e os campos obrigatórios têm unidades válidas. `fail` significa que a pergunta foi executada e uma condição foi reprovada. `blocked_by_missing_evidence` significa que a execução não permite concluir; não deve ser convertido em falha física nem em aprovação.

O contrato não permite “passar” apenas porque o DRV2605L respondeu no endereço I²C. Resposta elétrica, waveform, calibração, segurança e integridade semântica são evidências diferentes. A reprodução de um efeito de biblioteca também não prova que aquele efeito é distinguível ou que possui o significado atribuído pelo HAP-SEM.

## Referências

[1]: https://www.ti.com/lit/ds/symlink/drv2605l.pdf "Texas Instruments — DRV2605L datasheet"

[2]: https://www.ti.com/lit/an/sloa189/sloa189.pdf "Texas Instruments — Haptic Driver and Actuator Considerations"

[3]: https://www.mdpi.com/2076-3417/14/1/43 "Yeganeh et al. — Discrimination Accuracy of Sequential Versus Simultaneous Vibrotactile Stimulation on the Forearm"
