# HERUS — Pacote de bancada DRV2605L/HAP-SEM

**Estado:** preparação de bancada; nenhum ensaio físico executado.  
**Data da pesquisa:** 20 de agosto de 2026.  
**Conclusão operacional:** o repositório não contém placa, módulo DRV2605L, atuador, instrumento, fotografia, esquema anexado ou raw log. O gate físico permanece `pending` e o board map continua desabilitado.

## 1. Inventário observado

| Item | Encontrado nesta sessão | Consequência |
|---|---|---|
| LilyGO T3-S3 V1.3 real | não | revisão, variante de rádio, estado de solda e acesso a pinos não podem ser confirmados |
| DRV2605L real ou breakout | não | não há presença elétrica, endereço, waveform ou calibração observáveis |
| Atuador ERM/LRA | não | não há profile físico, rated voltage, overdrive ou back-EMF para configurar |
| Fonte limitada/corte independente | não | nenhum ensaio pode ser energizado com segurança |
| Osciloscópio/sonda diferencial | não | waveform OUT± não pode ser medida |
| Acelerômetro/fixture | não | aceleração e distinguibilidade não podem ser avaliadas |
| Raw log de bancada | não | nenhum registro deve ser fabricado |

A ausência não é tratada como zero. O estado correto é `blocked_by_missing_evidence`, não `fail` físico nem `pass`.

## 2. Evidência oficial sobre a placa

A página oficial da LILYGO descreve a T3 S3 V1.3 como ESP32-S3FH4R2, com 4 MB de flash, 2 MB de PSRAM e interface I²C do OLED SSD1306 em SDA/GPIO18 e SCL/GPIO17 [1]. A documentação de hardware do repositório oficial LilyGo-LoRa-Series afirma que V1.2 e V1.3 usam os mesmos pinos, mas também marca SDA 18 e SCL 17 como não livres porque compartilham o OLED e observa que essas linhas não estão convenientemente expostas; ela lista como recurso o esquema V1.3 [2].

Isso é suficiente para justificar o comentário atual do HERUS, mas **não** para habilitar o DRV2605L. Há três questões que somente uma placa e o esquema da variante exata resolvem: se os sinais 17/18 estão acessíveis no conector usado, se o OLED deve permanecer no mesmo barramento e se existem resistores pull-up adequados para a nova carga. A fonte oficial também distingue variantes SX1262/SX1276/SX1280/LR1121; portanto, “T3-S3 V1.3” sozinho não identifica a variante de rádio nem o layout montado [1].

A decisão atual é manter o target desabilitado e exigir fotografia frente/verso, marcação da placa, variante de rádio, esquema correspondente e continuidade do caminho SDA/SCL antes de alterar `BOARD_HAS_HAPTIC_I2C`.

## 3. Evidência oficial sobre o DRV2605L

O datasheet do TI descreve o DRV2605L como driver I²C para ERM e LRA, com alimentação VDD de 2 a 5,2 V, entrada ENABLE e pinos SDA/SCL [3]. O guia de configuração informa o endereço I²C de 7 bits `0x5A`, exige ENABLE alto para acessar corretamente os registradores e descreve um estado de standby quando ENABLE está baixo [4]. O HERUS não deve interpretar um ACK com ENABLE baixo como atualização confirmada.

O mesmo guia descreve auto-calibração como um procedimento que depende do atuador conectado: configura parâmetros de rated/overdrive, feedback e controles, inicia o modo de auto-calibração pelo registrador de modo e GO, espera GO limpar, lê diagnóstico e salva `ACalComp`, `ACalBEMF` e ganho de feedback [4]. O tempo de até aproximadamente 2 segundos indicado na documentação é um limite de procedimento do driver, não uma medição do HERUS. Nenhum valor de rated voltage, overdrive, frequência LRA ou compensação deve ser inventado sem o datasheet do atuador específico.

O datasheet também identifica capacitores de 1 µF em VDD e REG no esquema de aplicação e fornece a faixa de frequência de LRA e limites elétricos do componente [3]. Esses valores orientam a revisão do breakout, mas não substituem o esquema do módulo que será comprado ou montado.

## 4. Contrato de identidade antes de ligar

Antes do primeiro `haptic-probe`, o caderno privado da bancada deve conter os itens abaixo. O registro público deve conter somente os campos técnicos permitidos pelo schema, seus códigos de falha e digests.

| Identidade | Obrigatório antes do probe | Como obter |
|---|---|---|
| Placa | variante de rádio, revisão e foto legível | marcação física + esquema oficial correspondente |
| SDA/SCL | GPIO, acessibilidade e compartilhamento do barramento | continuidade e inspeção do conector |
| Pull-ups | valor, referência e alimentação do barramento | esquema + medição sem energia |
| Driver | marcação/package do DRV2605L e endereço esperado | inspeção + probe controlado |
| ENABLE | GPIO ou jumper verificado, estado baixo inicial | continuidade + osciloscópio/medidor lógico |
| Atuador | ERM/LRA, modelo, rated/overdrive e folha técnica | etiqueta e datasheet do fabricante |
| Instrumentos | identificação, faixa e validade de calibração | certificado/cadastro interno |
| Corte | interrupção independente do firmware | teste em fixture, sem corpo humano |

Se qualquer identidade estiver ausente, o resultado é `blocked_by_missing_evidence` com código específico, por exemplo `BOARD_REVISION_UNVERIFIED`, `PIN_MAP_UNVERIFIED`, `ACTUATOR_DATASHEET_MISSING` ou `SAFETY_STOP_UNAVAILABLE`.

## 5. Runner e decisão desta fase

O runner `tools/haptic_bench_runner.py` agora prepara e valida um envelope privado de evidência com `execution_origin`. Nesta sessão ele produziu somente `result=blocked_by_missing_evidence`, `execution_origin=blocked_no_hardware` e `failure_reason_code=HARDWARE_NOT_ATTACHED`; nenhum I²C foi emitido e nenhum valor físico foi preenchido. A suíte global passou **53/53**, incluindo os testes de origem do validador, mas isso continua sendo prova de software.

O pacote de software está preparado, mas esta sessão não pode fechar o gate físico porque não possui componentes nem instrumentos. O próximo trabalho útil é especificar o fixture e o wiring exatos para uma placa identificada, não habilitar GPIOs por inferência. Uma vez que o usuário forneça a placa e o módulo/atuador — ou os dados de compra e fotografias — o HERUS poderá executar somente o bring-up elétrico, manter `fixture_only=true` e produzir o primeiro registro privado conforme `research/haptic_bench_evidence_schema.json`.

## Referências

[1]: https://lilygo.cc/en-us/products/t3-s3-v1-3 "LILYGO — T3 S3 V1.3 product page"

[2]: https://github.com/Xinyuan-LilyGO/LilyGo-LoRa-Series/blob/master/docs/en/t3_s3_sx1278/t3_s3_sx1278_hw.md "LilyGo-LoRa-Series — T3-S3 hardware documentation and pin map"

[3]: https://www.ti.com/lit/gpn/drv2605l "Texas Instruments — DRV2605L datasheet"

[4]: https://www.ti.com/lit/pdf/sloa189 "Texas Instruments — DRV2605 Setup Guide"
