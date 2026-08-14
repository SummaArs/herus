# HERUS — Protocolo de bancada física pré-registrado

**Status:** protocolo preparado; nenhuma medição física foi realizada por este documento.
**Revisão:** bench-01
**Branch de preparação:** `bench-01-physical-protocol`
**Baseline obrigatório:** `./prove.sh --quiet` com `ALL INVARIANTS HOLD`

## 1. Propósito e fronteira

Este documento define como a primeira campanha física deverá ser executada quando o adaptador e os instrumentos estiverem disponíveis. Ele não transforma datasheets em resultados e não aprova placa, rádio, energia, UX, ASR, modelo ou armazenamento. A finalidade é permitir que um resultado futuro seja **reprodutível, auditável e limitado ao setup que realmente foi medido**.

A orientação de resiliência de plataforma da NIST separa mecanismos de proteção, detecção e recuperação de firmware e dados [1]. A engenharia de sistemas confiáveis também trata verificação, validação, risco e integração como atividades próprias do ciclo de vida [2]. O HERUS adota essas referências como orientação de processo, não como certificação ou declaração de conformidade.

> **Regra de honestidade:** ausência de evidência recebe `blocked_by_missing_evidence`; nunca recebe zero, sucesso implícito ou estimativa.

## 2. Primeira bancada e alternativas

A bancada inicial usará, provisoriamente, **duas LilyGO T3 S3 V1.3 com a mesma variante SX1262**, uma identificada como `wearable-proxy` e outra como `puck-proxy`. A página oficial da placa declara ESP32S3FH4R2, 4 MB de flash, 2 MB de PSRAM, Wi-Fi/Bluetooth, OLED I2C, botões e variantes de rádio, mas a disponibilidade e a variante exata precisam ser verificadas na unidade recebida [3]. O esquema correspondente deve ser conferido antes do primeiro flash [4].

Essa escolha reduz risco de bring-up porque o port existente já separa `esp32s3/` e `sx1262/`. Ela não decide o produto final, não prova a geometria de wearable ou puck e não permite reutilizar um pin map de outra revisão. Uma unidade somente pode entrar no protocolo depois de sua marcação, revisão, variante de rádio, banda e esquema serem registrados.

| Candidato | Papel possível | Evidência primária disponível | Decisão nesta campanha |
|---|---|---|---|
| T3-S3 V1.3 + SX1262 | Par de bring-up e rádio inicial | ESP32-S3, flash/PSRAM e variantes declaradas pela LilyGO [3]; esquema da série [4] | **Selecionado provisoriamente** para reduzir risco de port |
| RP2350 + rádio LoRa separado | Puck/controlador com I/O determinístico | 520 KB SRAM, TrustZone, PIO, OTP e mitigação de fault injection declarados pela Raspberry Pi [5] | Alternativa de arquitetura, não primeiro flash |
| nRF54L15 + rádio LoRa separado | Wearable com prioridade de consumo | 1.5 MB NVM, 256 KB RAM, Cortex-M33, rádio 2.4 GHz, PDM/I2S e TrustZone declarados pela Nordic [6] | Alternativa de baixo consumo, requer port próprio |

O ESP32-S3 oferece 512 KB de SRAM interna, interfaces de áudio e aceleração vetorial no SoC, além de recursos de segurança declarados pela Espressif [7]. Esses dados ajudam a planejar o bring-up, mas não substituem medição do módulo/placa, da revisão concreta, do firmware ou do workload do HERUS.

## 3. Pré-condições de entrada

Antes de ligar uma placa, o operador deve registrar `board_revision`, `mcu_part_marking`, `radio_part_marking`, `frequency_profile`, `adapter_revision`, fotografia da placa, referência do esquema e identificação do instrumento. Também deve verificar continuidade/curto, polaridade, alimentação limitada, conector de antena e compatibilidade regional da frequência.

O comando `./prove.sh --quiet` deve passar no commit que será carregado. Em seguida, os alvos `make bench-evidence-validate` e `make bench-interruption-sim` devem passar. O primeiro valida o schema privado; o segundo exerce os oráculos C11 em host e declara explicitamente que não observou reset, mídia, GPIO, rádio ou evento de energia.

Nenhum teste físico começa se a revisão da placa não coincidir, o pin map não estiver documentado, o instrumento não possuir identificação e método, a terminação/antena não estiver correta ou o log não puder ser produzido sem campos proibidos.

## 4. Instrumentação mínima

| Domínio | Instrumentação | Registro permitido | Critério de bloqueio |
|---|---|---|---|
| Bring-up | USB/serial, multímetro, fonte com limite de corrente, osciloscópio e sondas adequadas | reset, boot, GPIO/SPI, tensão e corrente numéricas | pin map, revisão, alimentação ou trigger não observável |
| Rádio conduzido | duas placas, atenuador/dummy load apropriado, cabos e perfil regional congelado | pacotes, RSSI, SNR, distância declarada, latência e digest do log | variante/banda/antena/terminação não identificada |
| Energia | shunt ou monitor calibrado, taxa de amostragem e método de integração declarados | unidade, corrente, tensão, energia por workload e transientes | calibração, taxa, largura de banda ou workload ausente |
| Interrupção | switch/relé/MOSFET controlado, trigger, osciloscópio/logic analyzer e fixture de mídia | estado antes do corte, trigger, retorno, boot, ação, digest da transição | corte não observado, ponto escolhido a posteriori ou mídia sem revisão |
| Interação | botão, driver háptico, AFE/microfone somente em protocolo próprio | latência, energia, confirmação/cancelamento e digest anonimizado | protocolo A3, população, fonte ou unidade indefinidos |
| Modelo local | modelo/configuração congelado, monitor de RAM e workload fixo | digest, pico de RAM, p95, energia e resultado de agência | modelo alterado, prompt/conteúdo registrado ou workload incompleto |

A medição de RF irradiada, caminhada urbana, energia, UX e modelo é feita em campanhas separadas. Não se deve misturar uma falha de bring-up com um resultado de PDR ou autonomia.

## 5. Matriz de gates

| Gate | Primeiro experimento | Evidência mínima | Passa somente se | Falha ou bloqueia se |
|---|---|---|---|---|
| `board-pin-map` | inspeção, identificação e self-test sem transmissão | revisão, esquema, self-test | pin map real coincide com HAL medido | revisão ambígua, self-test ausente ou pino divergente |
| `mechanical-volume` | medição de stack sem alegar produto | paquímetro, shell/revisão, descrição | somente o stack declarado cabe na altura congelada | shell ou stack mudado, medida sem instrumento |
| `radio-bring-up` | dois nós a 1 m, perfil fixo | perfil, contagens, RSSI, self-test | linha numérica limpa no setup declarado | variante, antena, banda ou contagem não reconciliada |
| `urban-pdr` | rota pré-registrada e condições anotadas | rota, distância, contagens, digest | apenas a rota e condições daquela revisão atingem o gate | rota escolhida após observar resultados ou log incompleto |
| `tier05-comparison` | comparação pareada com configuração congelada | perfis pareados, PDR, airtime | hipótese é suportada ou rejeitada para aquele setup | perfis não equivalentes ou header/configuração ambígua |
| `energy-instrumentation` | workload sem rádio, depois rádio | instrumento, amostragem, energia e unidade | energia pertence ao workload declarado | unidade/calibração/workload ausentes |
| `interaction-io` | protocolo A3 de confirmação/cancelamento | revisão, digest anonimizado, latência/energia | somente protocolo e população declarados | não medir WER/conforto fora do protocolo |
| `companion-trust-port` | armazenamento/RNG/transporte/revogação em target | backend, RNG, transporte, digest de revogação | lifecycle do target testado | qualquer elo é simulado ou não identificado |
| `collection-crash-recovery` | corte em pontos pré-registrados da matriz | backend, método, digest da matriz, raiz/piso | cada ponto chega ao estado permitido | corte não observado, resultado escolhido depois ou raiz/piso divergente |
| `collection-physical-session` | boot, sessão nova, consumo, cancelamento, reboot e replay | adaptador, evento, clock/reset, quarentena, exaustão, cadeia e replay | somente o adaptador e método testados sustentam a afirmação | qualquer capacidade ativa atravessa reboot ou falta evidência |
| `local-model-profile` | carga congelada e adversarial sem conteúdo de produto | digest, peak RAM, p95, energia, manifesto | modelo/configuração passa orçamento e agência | modelo muda, prompt vaza ou orçamento não é medido |

## 6. Interrupções e recuperação

Cada execução recebe uma sequência determinada antes do corte. Os pontos mínimos são: antes de `PREPARED`; depois de `PREPARED`; depois da autenticação; depois de `COMMITTED`; durante cleanup; antes e depois da elevação do piso; depois da quarentena de boot; durante cancelamento; e em tentativa de replay após retorno. O fixture deve emitir um trigger observável para o osciloscópio/logic analyzer.

O operador lê o estado somente depois do boot completar. A ação esperada é comparada com a matriz C11; o backend físico não pode escolher uma ação diferente por conveniência. A entrada mínima de uma transição contém revisão do backend, estado anterior, ponto de corte, método de interrupção, trigger observado, estado pós-boot, ação do oráculo, digest do raw log e resultado. Raw logs não podem conter conteúdo de produto.

`PROMOTE_PREPARED` pode importar apenas o piso que o adaptador demonstrou ter queimado. Nunca importa nonce, propósito, usos, pessoa, evento ou sessão ativa. Um piso terminal deve bloquear ou seguir uma política de rotação comprovada; o manifesto exige `id_exhaustion_policy_digest`.

## 7. Schema, privacidade e digest

O schema versionado está em [`research/hardware_bench_evidence_schema.json`](../research/hardware_bench_evidence_schema.json), e seu validador em [`tools/bench_evidence_validate.py`](../tools/bench_evidence_validate.py). O registro aceita somente identificadores de revisão, tempos, unidades, contagens, distância, RSSI, SNR, energia, latência, resultado e digests. Ele rejeita recursivamente áudio, transcrição, embedding, identidade, localização, chave, conteúdo, prompts e respostas de modelo.

O digest é calculado sobre o JSON canônico sem o próprio `record_digest`. `null` não significa zero; ausência de medição não pode virar aprovação. Um resultado `pass` não carrega causa de falha; `fail` e `blocked_by_missing_evidence` precisam de código explicativo. Campos desconhecidos são rejeitados para impedir que um logger futuro introduza dados pessoais sem revisão.

## 8. Critérios de parada

A campanha para imediatamente quando houver risco elétrico, RF sem terminação adequada, revisão de placa incerta, instrumento fora da calibração declarada, qualquer campo proibido, divergência de digest, reset não observado, resultado incompatível com a matriz ou capacidade ativa após reboot. Também para quando a pergunta já não pode ser respondida pelo instrumento disponível.

O protocolo não autoriza otimizar a configuração até o resultado passar. Qualquer mudança de placa, antena, rádio, firmware, modelo, supply, distância, rota, instrumento ou algoritmo cria uma nova revisão de experimento.

## 9. O que este protocolo ainda não afirma

Nenhum gate físico está aprovado pela existência deste documento. Ainda não há no repositório resultado de PDR real, alcance, WER, autonomia, energia medida, replay pós-reset, secure boot de placa, armazenamento atômico, limpeza de RAM, evento humano, conforto, utilidade pessoal, qualidade de ASR ou utilidade de LLM. O próximo passo executável depende de hardware disponível, unidade identificada, instrumentos e autorização explícita para a bancada.

## Referências

[1] National Institute of Standards and Technology, *SP 800-193: Platform Firmware Resiliency Guidelines*. [Publicação oficial](https://csrc.nist.gov/pubs/sp/800/193/final).

[2] National Institute of Standards and Technology, *SP 800-160 Vol. 1 Rev. 1: Engineering Trustworthy Secure Systems*. [Publicação oficial](https://csrc.nist.gov/pubs/sp/800/160/v1/r1/final).

[3] LILYGO, *T3 S3 V1.3*. [Página oficial do produto](https://lilygo.cc/en-us/products/t3-s3-v1-3).

[4] Xinyuan-LILYGO, *T3_S3_V1.2 schematic*. [Esquema publicado](https://github.com/Xinyuan-LilyGO/LilyGo-LoRa-Series/blob/master/schematic/T3_S3_V1.2.pdf).

[5] Raspberry Pi, *RP2350*. [Página oficial do produto](https://www.raspberrypi.com/products/rp2350/).

[6] Nordic Semiconductor, *nRF54L15 System-on-Chip*. [Página oficial do produto](https://www.nordicsemi.com/Products/nRF54L15).

[7] Espressif Systems, *ESP32-S3 Series Datasheet*. [Datasheet oficial](https://documentation.espressif.com/esp32-s3_datasheet_en.pdf).
