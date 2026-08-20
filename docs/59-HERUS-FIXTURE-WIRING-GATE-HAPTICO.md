# HERUS — Fixture, wiring e gate de bring-up HAP-SEM/DRV2605L

**Estado:** plano de bancada não executado.  
**Regra principal:** nenhum componente é ligado nesta etapa sem placa/variante identificada, esquema conferido e operador capaz de interromper a alimentação.

## 1. Objetivo do primeiro fixture

O primeiro fixture não testa linguagem nem percepção. Ele responde somente se um conjunto identificado — placa, bus, driver, atuador e fonte — pode ser energizado sem erro evidente, responder ao endereço I²C esperado e permanecer sob controle de uma parada independente. O primeiro comando do firmware é `haptic-probe`; ele não executa `ha_play()`, não escreve waveform, não inicia GO e não deve fazer o atuador vibrar.

Qualquer playback posterior permanece em uma campanha separada. O fixture inicial deve manter `fixture_only=true`, ficar fora do corpo humano e possuir uma barreira mecânica para que o atuador não se solte durante um teste.

## 2. Topologia proposta

A topologia usa uma única alimentação lógica comum e uma fonte de bancada limitada para o módulo háptico. A decisão de usar 3,3 V é uma convenção de fixture para compatibilizar níveis com o ESP32-S3, não uma exigência exclusiva do DRV2605L; o datasheet aceita VDD entre 2 e 5,2 V [1]. A tensão real deve ser registrada e não pode ser inferida do nome da placa.

| Nó | Ligação proposta | Condição antes de ligar |
|---|---|---|
| GND ESP32-S3 | GND do módulo DRV2605L e retorno da fonte | continuidade verificada, sem curto entre VDD e GND |
| VDD DRV2605L | saída limitada da fonte de bancada; alvo inicial do fixture: 3,3 V | módulo confirma faixa e capacitores; limite de corrente anotado |
| SDA | GPIO confirmado no esquema/continuidade → SDA do DRV2605L | nível alto não excede VDD; pull-up externo identificado |
| SCL | GPIO confirmado no esquema/continuidade → SCL do DRV2605L | nível alto não excede VDD; pull-up externo identificado |
| ENABLE | jumper/sinal GPIO verificado → EN; estado inicial baixo | estado baixo medido antes de VDD; caminho de desligamento independente |
| IN/TRIG | GND se não for usado | não deixar entrada flutuante; confirmar no módulo |
| REG | capacitor de 1 µF para GND conforme aplicação do fabricante | capacitor, polaridade e referência conferidos |
| OUT+ / OUT− | somente ao atuador identificado, respeitando polaridade/recomendação do fabricante | sem osciloscópio comum ligado diretamente ao par diferencial |

O DRV2605L tem endereço I²C de 7 bits `0x5A`. O acesso aos registradores requer ENABLE alto; a documentação do fabricante alerta que o dispositivo pode responder de forma enganosa quando está desabilitado, portanto um ACK isolado não fecha o gate [2]. O target do HERUS mantém ENABLE baixo na inicialização e só o eleva dentro do probe explícito.

A placa LilyGO T3-S3 V1.3 documenta GPIO18 como SDA e GPIO17 como SCL do OLED [3]. A documentação de hardware da série também marca essas linhas como compartilhadas e não livres, e recomenda confirmar a variante e a forma de acesso; por isso, o wiring do HERUS deve usar o esquema da revisão efetivamente adquirida, não somente os números no header [4].

## 3. Instrumentos mínimos

A bancada deve registrar identificação e validade de calibração. Um instrumento ausente produz `null` e bloqueia o gate correspondente; não pode ser convertido para zero.

| Instrumento | Uso no gate | Mínimo aceitável |
|---|---|---|
| fonte CC com limite de corrente | energização inicial e detecção de curto | limite configurável, leitura de tensão/corrente e saída desligável |
| multímetro | continuidade, VDD, ENABLE, SDA/SCL em repouso | categoria e validade adequadas ao circuito |
| analisador lógico ou osciloscópio | verificar START/STOP, endereço, ACK e clock I²C | captura digital com taxa suficiente para 100 kHz |
| osciloscópio + ponta diferencial | somente campanha de waveform OUT± | entrada diferencial apropriada; nunca medir OUT± com ground clip comum |
| acelerômetro/fixture | campanha posterior de amplitude e duração | montagem repetível e eixo documentado |
| corte físico | interromper VDD/ENABLE sem firmware | acessível ao operador e testado antes do playback |

O osciloscópio não é necessário para o primeiro probe se o analisador lógico comprovar o barramento e a fonte/multímetro comprovarem os níveis. Ele passa a ser obrigatório antes de alegar waveform. A TI recomenda um arranjo de filtro específico para observar OUT± porque a saída contém modulação PWM; o filtro e a impedância devem seguir a documentação, não uma ligação improvisada [1].

## 4. Sequência de segurança

A sequência começa sem atuador conectado e com VDD desligado. Primeiro identifica-se a placa e confere-se o esquema; em seguida mede-se continuidade e ausência de curto. Depois se confirma o estado baixo do ENABLE, confere-se VDD configurado e liga-se a fonte com limite de corrente. O firmware só pode inicializar o bus quando o gate de pin map estiver promovido localmente.

Em seguida, o operador executa `haptic-probe`. O target sobe ENABLE, aguarda o startup documentado, executa o probe I²C em `0x5A`, registra resultado técnico e baixa ENABLE antes de desligar/liberar o bus. A documentação do DRV2605 informa startup interno da ordem de centenas de microssegundos e orienta retry controlado em caso de nACK inicial; o HERUS deve registrar a ocorrência e não repetir indefinidamente [2].

Nenhum `haptic-probe` deve transmitir rádio, abrir sessão, escrever memória, executar planner, alterar perfil pessoal ou tocar waveform. Durante toda a sequência, o atuador fica desconectado ou montado em fixture isolado. Se houver aquecimento inesperado, corrente anormal, endereço diferente, SDA/SCL preso, ENABLE incerto ou falha de corte, a fonte é desligada e o resultado é bloqueado.

## 5. Critérios de promoção

| Gate | Só pode ser `pass` se | Caso contrário |
|---|---|---|
| identidade da placa | variante, revisão, esquema e foto conferem | `BOARD_REVISION_UNVERIFIED` |
| pin map | SDA/SCL/ENABLE têm caminho confirmado e não conflitam com função ativa | `PIN_MAP_UNVERIFIED` |
| alimentação | VDD, GND, REG e capacitores conferem; corrente limitada | `POWER_ENVELOPE_UNVERIFIED` |
| bus idle | SDA/SCL sobem, clock é 100 kHz nominal configurado e não há erro | `I2C_BUS_UNVERIFIED` |
| presença | probe em 0x5A com ENABLE alto durante a janela, depois ENABLE baixo | `DRV2605L_NOT_FOUND` |
| segurança | corte físico interrompe VDD/ENABLE e foi testado | `SAFETY_STOP_UNAVAILABLE` |
| calibração | somente para campanha de playback; atuador e folha técnica identificados | `ACTUATOR_DATASHEET_MISSING` |
| waveform | somente com ponta diferencial/fixture e captura privada | `WAVEFORM_INSTRUMENT_MISSING` |

`pass` no probe significa apenas presença elétrica no envelope declarado. Não significa DRV2605L correto, atuador correto, waveform correta ou semântica percebida. Para o gate HAP-SEM, a promoção deve ocorrer em estágios: `probe_electrical`, `driver_config`, `actuator_calibration`, `fixture_playback`, `waveform_measurement` e, muito depois, `human_perception`.

## 6. Checklist de parada

Antes de qualquer ligação, o operador deve conseguir responder “sim” a todas as perguntas de segurança: a fonte pode ser desligada sem software; o ENABLE pode ser mantido baixo sem depender de boot; o atuador está fixado; o fio OUT± não está em contato com lógica; o osciloscópio não está ligado de modo comum ao par diferencial; não há bateria conectada em paralelo; não há transmissão de rádio durante o probe; e toda pessoa está fora do caminho mecânico do atuador.

Se uma resposta for “não sei”, o resultado correto é bloqueio. Esta regra é intencional: o HERUS deve falhar antes da bancada, não descobrir no primeiro ensaio que um comentário de pin map não era um esquema.

## Referências

[1]: https://www.ti.com/lit/gpn/drv2605l "Texas Instruments — DRV2605L datasheet"

[2]: https://www.ti.com/lit/pdf/sloa189 "Texas Instruments — DRV2605 Setup Guide"

[3]: https://lilygo.cc/en-us/products/t3-s3-v1-3 "LILYGO — T3 S3 V1.3 product page"

[4]: https://github.com/Xinyuan-LilyGO/LilyGo-LoRa-Series/blob/master/docs/en/t3_s3_sx1278/t3_s3_sx1278_hw.md "LilyGo-LoRa-Series — T3-S3 hardware documentation and pin map"
