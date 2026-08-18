# HERUS — Guia de montagem física, BOM e bring-up

**Status:** guia de preparação; nenhuma medição física foi realizada por este documento.  
**Revisão:** assembly-01  
**Data de preços consultados:** 14 de agosto de 2026  
**Baseline de software:** `./prove.sh --quiet` deve retornar `ALL INVARIANTS HOLD` antes de qualquer campanha física.

> **Regra central:** o que está descrito aqui é um plano de construção e teste. Não é evidência de alcance, PDR/WER, autonomia, consumo, qualidade de modelo, segurança física ou adequação de produto. Todo resultado não medido permanece `pending` ou `blocked_by_missing_evidence`.

## 1. Decisão de engenharia

A montagem deve acontecer em camadas, e não como uma tentativa prematura de fabricar o produto final. A primeira camada é uma bancada simétrica com duas placas iguais, uma rotulada `wearable-proxy` e outra `puck-proxy`. A segunda camada acrescenta periféricos físicos para testar energia, háptica, entrada e recuperação. A terceira camada cria protótipos mecânicos separados para o vestível e para o puck circular. Somente depois de medir essas camadas deve existir uma decisão de PCB customizada, enclosure final, certificação ou arquitetura de produção.

A escolha provisória para a bancada é **duas LilyGO T3 S3 V1.3 com a mesma variante SX1262**. A página oficial declara ESP32-S3, 4 MB de flash, 2 MB de PSRAM e variantes de rádio, mas a unidade recebida ainda precisa ser conferida por marcação, revisão de PCB, esquema, conector e banda [1]. A decisão reduz o risco porque o repositório já possui port para ESP32-S3/SX1262; ela não prova que ESP32-S3 seja a melhor plataforma final.

A arquitetura permanece aberta. O **nRF54L15** é uma alternativa forte para o wearable de baixo consumo, mas o kit tem rádio 2,4 GHz, não LoRa, e exige port próprio e um rádio separado para o enlace de longo alcance [3]. O **RP2350** é um candidato para puck determinístico, com I/O e PIO, mas também não integra LoRa; rádio, armazenamento, alimentação e áudio permanecem externos [2]. Módulos RAK como o RAK11300, RAK13300 e RAK3172 podem separar MCU e rádio, com preços publicados de referência, mas exigem placa base e integração diferentes [9].

## 2. Taxa de conversão e disciplina de custos

Os valores em BRL abaixo usam **US$1 = R$5,2318**, taxa mid-market exibida pela Xe às 18:11 UTC em 14 de agosto de 2026 [10]. Essa taxa serve apenas para comparação. O usuário não receberá necessariamente essa taxa ao comprar; frete, imposto, câmbio do cartão, importação, disponibilidade e garantia local podem dominar o preço final. As faixas sem fonte de varejo são explicitamente marcadas como **premissas de planejamento**, não como cotação.

A moeda não deve influenciar a escolha de engenharia de forma isolada. Para HERUS, uma placa mais barata que não tenha o port, a documentação ou o método de medição pode custar mais em tempo e risco do que a placa provisória mais cara.

## 3. BOM por estágio

### 3.1 Estágio 0 — bancada mínima de bring-up

Este estágio não é wearable nem puck. Ele serve para verificar alimentação, boot, identificação, pin map, SPI/GPIO, reset e rádio sob um procedimento controlado. Não ligar microfone, bateria ou motor antes de o boot de bancada estar estável.

| Item | Quantidade | Referência | Preço unitário de referência | Subtotal USD | Subtotal BRL aproximado | Situação |
|---|---:|---|---:|---:|---:|---|
| Placa ESP32-S3 + SX1262 | 2 | LilyGO T3 S3 V1.3 | US$15,31 | US$30,62 | R$160,25 | preço observado na loja oficial [1] |
| Antena de bancada correta para a banda | 2 | Exemplo 915 MHz RP-SMA 1/4 wave, 2 dBi | US$18,95 | US$37,90 | R$198,32 | exemplo publicado pela SparkFun; 868 MHz exige outra referência [8] |
| Pigtail/adaptador RF compatível | 2 | U.FL/IPEX ↔ SMA/RP-SMA, conforme a placa real | US$6–12 | US$12–24 | R$62,78–125,56 | premissa de planejamento; confirmar conector antes de comprar |
| Cabo USB de dados | 2 | USB-C/USB conforme placa e computador | US$4–8 | US$8–16 | R$41,85–83,71 | premissa de planejamento; cabo deve transmitir dados |
| Fonte de bancada com limite de corrente | 1 | saída compatível com a entrada documentada | US$40–80 | US$40–80 | R$209,27–418,54 | premissa de planejamento; não usar fonte sem limitação para o primeiro power-on |
| **Total da bancada mínima** |  |  |  | **US$128,52–188,52** | **R$672,39–986,30** | sem frete, impostos e instrumentos adicionais |

A antena da linha da tabela é apenas um exemplo de 915 MHz. O HERUS não deve transmitir em 915, 868 ou 433 MHz por hábito ou disponibilidade comercial. A banda regional, potência, duty cycle, conector e legislação precisam ser definidos antes do teste. Uma antena incompatível, um adaptador com perda ou um rádio sem terminação apropriada invalida o resultado de RF.

Para uma bancada realmente capaz de produzir evidência numérica, acrescentar os instrumentos abaixo. Um multímetro barato ajuda no diagnóstico, mas não deve ser tratado como medidor de autonomia; o método de energia precisa declarar faixa, taxa, shunt, calibração e workload.

| Instrumento | Faixa de planejamento | Uso | Limite |
|---|---:|---|---|
| Monitor de energia INA228 Qwiic | US$14,95 / R$78,22 | corrente, tensão, energia, carga e alertas por I2C; 20 bits declarados pelo fabricante [5] | não substitui calibração independente nem osciloscópio |
| Multímetro | US$20–60 / R$104,64–313,91 | continuidade, polaridade, tensão DC e diagnóstico | não captura transientes rápidos nem prova autonomia |
| Logic analyzer simples | US$15–40 / R$78,48–209,27 | SPI, I2C, reset, GPIO e triggers | largura de banda e amostragem precisam ser adequadas ao sinal |
| Osciloscópio | US$250–600 / R$1.307,95–3.139,08 | transientes de alimentação, reset, interrupção e trigger físico | a faixa é somente planejamento; modelo e calibração ainda precisam ser escolhidos |

A bancada mínima mais INA228, multímetro e logic analyzer fica aproximadamente em **US$178,47–303,47**, ou **R$933,72–1.587,69**, antes do osciloscópio. Com osciloscópio, a faixa de planejamento passa aproximadamente a **US$428,47–903,47**, ou **R$2.241,67–4.726,77**. Esses totais não são preço de compra; são uma forma de separar o custo de ter placas do custo de medir com alguma seriedade.

### 3.2 Estágio 1 — protótipo wearable sobre a placa da bancada

O wearable de estágio 1 continua sendo um protótipo de engenharia. A placa T3-S3 fica temporariamente fora do corpo ou em uma caixa grande; ainda não se deve alegar conforto, segurança para uso prolongado, autonomia ou geometria final. O primeiro teste de entrada deve ser feito em modo **sem microfone**. O microfone só entra depois de a política de mute, indicação e descarte ter sido observada no firmware e no circuito.

| Item adicional | Quantidade | Referência | Preço de referência | Finalidade e restrição |
|---|---:|---|---:|---|
| Bateria LiPo protegida | 1 | 3,7 V, 150 mAh; alternativa próxima de 350 mAh | US$5,95–6,95 / R$31,13–36,36 | a Adafruit declara proteção, 3,7 V nominal e carregamento CC/CV; não carregar sem supervisão [4] |
| Carregador/PMIC LiPo | 1 | módulo CC/CV compatível com o pack e o caminho de energia | US$8–15 / R$41,85–78,48 | premissa de planejamento; confirmar corrente, proteção, power-path e conector |
| Driver háptico | 1 | Adafruit DRV2605L | US$7,95 / R$41,59 | driver I2C para ERM/LRA com efeitos hápticos declarados [6] |
| Motor vibratório | 1 | motor ERM tipo moeda | US$1,95 / R$10,20 | item separado do driver; fixar mecanicamente e limitar corrente [6] |
| Cabo háptico | 1 | STEMMA/Qwiic ou fios equivalentes | US$0,95 / R$4,97 | confirmar pinagem e tensão antes de conectar [6] |
| Microfone MEMS digital opcional | 1 | SPH0645LM4H I2S | US$6,95 / R$36,36 | 1,6–3,6 V e saída digital I2S declarados; usar apenas em protocolo separado [7] |
| Botão físico, LED, resistores, fios e protoboard/perfboard | 1 conjunto | componentes genéricos | US$2–6 / R$10,46–31,39 | premissa de planejamento; o botão de confirmação não pode ser só software |
| Pulseira, strain relief e enclosure provisório | 1 conjunto | material não condutivo e removível | US$10–20 / R$52,32–104,64 | premissa de planejamento; não chamar de enclosure final |
| **Acréscimo do wearable** |  |  | **US$42,80–65,75** | **R$223,92–343,99** |

Somando o Estágio 1 à bancada mínima, o conjunto de prova fica aproximadamente em **US$170,27–253,27**, ou **R$890,82–1.325,06**, ainda sem frete, impostos e osciloscópio. A faixa é ampla porque o custo real depende principalmente de compra local, carregador, enclosure e conectores.

O microfone precisa de uma barreira física de privacidade: chave de mute ou desconexão visível, estado indicado e firmware que não possa transformar áudio bruto em memória sem confirmação física. O protocolo de bancada pode medir corrente, latência e resultado categórico; não deve registrar áudio, transcrição, embedding, identidade, localização, prompt ou resposta de modelo.

### 3.3 Estágio 2 — protótipo puck circular

O puck deve ser uma estação-base de bolso experimental: rádio, alimentação, confirmação física, display e eventualmente armazenamento. A forma circular só deve ser congelada depois de medir a placa, os conectores, a bateria, a antena, a dissipação e a altura dos componentes. Não imprimir uma carcaça final em escala arbitrária.

A tabela abaixo descreve o **acréscimo sobre um `puck-proxy` T3-S3 já comprado no Estágio 0**.

| Item adicional | Quantidade | Referência | Faixa de planejamento | Observação |
|---|---:|---|---:|---|
| LiPo maior | 1 | 3,7 V, 1.200 mAh como referência de catálogo | US$9,95 / R$52,05 | capacidade publicada pela Adafruit entre as variantes do pack [4]; ainda exige validação térmica e de carga |
| Carregador/power-path | 1 | CC/CV, proteção, load switch e medição | US$10–20 / R$52,32–104,64 | premissa de planejamento; o puck não deve alimentar o wearable sem limite validado |
| Display | 1 | OLED/LCD de baixo consumo | US$15–30 / R$78,48–156,95 | premissa de planejamento; a placa T3 já pode ter OLED, mas isso não define a mecânica do puck |
| Armazenamento removível e socket | 1 | microSD ou flash externa conforme ameaça | US$10–25 / R$52,32–130,80 | premissa de planejamento; não é memória de produto até testar criptografia, atomicidade e recuperação |
| Enclosure circular provisório | 1 | impressão 3D ou usinado em protótipo | US$20–60 / R$104,64–313,91 | premissa de planejamento; dimensões dependem de medição real |
| Botões, LED, espaçadores e proteção mecânica | 1 conjunto | confirmação, cancelamento e manutenção | US$10–25 / R$52,32–130,80 | premissa de planejamento; confirmação física deve permanecer dominante |
| Antena, pigtail, suporte e alívio de cabo | 1 conjunto | banda regional escolhida | US$10–30 / R$52,32–156,95 | premissa de planejamento; não transmitir sem carga/antena correta |
| **Acréscimo do puck** |  |  | **US$84,95–194,95** | **R$444,44–1.019,94** |

Se for necessário construir um puck independente, acrescentar uma placa de controle. O **Raspberry Pi Pico 2** aparece oficialmente a partir de US$5 e é baseado no RP2350 [2]. Ele não substitui o rádio LoRa, portanto uma configuração RP2350 + módulo LoRa + alimentação + armazenamento não deve ser comparada ao preço de uma T3-S3 completa. Uma alternativa modular é o RAK11300, publicado a US$6,95, ou o módulo SX1262 RAK13300, publicado a US$11,00, mas ambos exigem uma base e um port de firmware diferentes [9].

O puck pode hospedar uma LLM local em uma fase posterior, mas o Estágio 2 não prova que isso é possível. A primeira execução deve usar carga congelada, digest de modelo e medição de pico de RAM, latência e energia. O conteúdo de pessoa não entra no log. Se o modelo exceder o orçamento, a operação deve recusar, pedir outro dispositivo ou permanecer inerte; não deve produzir uma decisão autônoma para contornar o limite.

### 3.4 Estágio 3 — produto final

O produto final não tem um preço honesto neste momento. Ainda faltam esquemático elétrico, seleção de MCU e rádio, PMIC, armazenamento, regras de certificação, volume de produção, enclosure, tolerâncias, testes EMC/RF, bateria aprovada, firmware seguro, ferramental e cadeia de suprimentos. Dar um número único agora criaria precisão falsa.

| Bloco do produto final | Estado necessário antes de cotar | Não fazer ainda |
|---|---|---|
| PCB wearable | pin map congelado, orçamento de energia, RF, I/O e segurança | não derivar layout da protoboard sem revisão de alimentação e RF |
| PCB puck | interface rádio, bateria, display, armazenamento e confirmação física definidos | não assumir que a T3-S3 é o layout de produção |
| Rádio/antena | banda, potência, matching, conector, região e ensaios | não alegar alcance nem conformidade a partir de datasheet |
| Bateria e carregamento | célula, proteção, termistor/power-path, testes térmicos e transporte | não usar pack genérico sem documentação e proteção |
| Enclosure | stack real, queda, vedação, contato com corpo, dissipação e antena | não fechar a antena entre bateria e corpo sem estudo de RF |
| Certificação | laboratório e requisitos do mercado-alvo | não chamar protótipo de produto certificado |

## 4. Arquitetura elétrica provisória

### 4.1 Wearable

A interligação inicial deve ser simbólica até o gate `board-pin-map` confirmar o pinout da unidade. Não gravar números de GPIO em fios ou no enclosure com base em uma placa semelhante.

```text
LiPo protegido
    │
    ├── carregador CC/CV + power-path + load switch
    │                         │
    │                         └── alimentação documentada da T3-S3
    │
T3-S3 + SX1262 ── SPI/GPIO/IRQ ── rádio LoRa ── pigtail ── antena correta
    │
    ├── I2C verificado ── DRV2605L ── motor ERM/LRA
    ├── I2S verificado ── MEMS opcional, com mute físico
    ├── GPIO verificado ── botão físico de confirmação
    └── LED/estado ── indicação de energia, mute e confirmação
```

O rádio deve ser energizado e transmitido somente depois do self-test, da identificação da banda e da conexão a antena ou carga apropriada. O barramento I2C pode ser compartilhado com OLED ou outros periféricos apenas depois de os endereços, pull-ups e níveis lógicos serem verificados. O I2S do microfone não deve ser presumido a partir do nome do SoC; o pin map da placa e o clock real devem ser confirmados.

No vestível, a bateria deve ficar em compartimento separado do rádio e do ponto de contato com a pele, com barreira mecânica não condutiva e sem compressão da célula. O motor deve transferir vibração por um elastômero ou suporte que não rasgue os fios. O microfone precisa de abertura acústica externa e um mute físico que possa ser verificado sem software. A antena deve ficar afastada de bateria, cabos longos e áreas que possam ser cobertas pelo corpo; isso é uma hipótese mecânica, não uma prova de RF.

### 4.2 Puck

O puck circular deve privilegiar acesso, dissipação, serviço e confirmação física, não apenas estética. Uma organização provisória é colocar a bateria e o power-path na camada inferior, a PCB sobre espaçadores, o display e os botões na face superior e a antena em perímetro ou região externa onde o material e a distância possam ser medidos.

```text
Face superior: display + confirmação + cancelamento + indicação de estado
                         │
            PCB de controle/radio/armazenamento
          ┌──────────────┼──────────────┐
          │              │              │
     SPI/GPIO        storage        SX1262 ── feed ── antena
          │              │              │
      display      mídia removível   perfil regional
                         │
              power-path + medição INA228
                         │
                 LiPo protegido
```

A presença de armazenamento no puck não autoriza persistência automática. O sistema deve continuar sujeito à regra central do HERUS: nenhum caminho pode persistir, enviar ou criar HCP sem confirmação física. Até o gate de armazenamento físico, a mídia é apenas uma fixture de teste, e os dados de bancada permanecem numéricos e sem conteúdo.

## 5. Sequência de montagem segura

### Passo 0 — congelar o objeto do experimento

Criar uma folha de montagem com `assembly_id`, placa, revisão, variante do rádio, frequência, antena, adaptador, firmware, commit, instrumento e operador. Separar fisicamente as caixas `wearable-proxy` e `puck-proxy`. Fotografar a marcação dos componentes e guardar o digest do firmware; a fotografia não deve entrar no log de produto.

Antes de seguir, executar `./prove.sh --quiet`, `make bench-evidence-validate` e `make bench-interruption-sim`. O protocolo físico existente exige que o esquema, o pin map, a alimentação, a instrumentação e o método estejam definidos antes de ligar a placa [11].

### Passo 1 — inspeção sem alimentação

Inspecionar soldas, conectores, polaridade, antena, cabos, parafusos, espaçadores e possíveis pontes de solda. Confirmar a revisão e a variante do rádio contra o esquema correspondente. Não conectar bateria. Não inserir microSD. Não conectar o motor diretamente a um GPIO. Não aplicar tensão para descobrir a polaridade.

Medir continuidade somente nos nets previstos. Entre alimentação e terra, observar se há curto persistente; capacitores podem produzir leituras transitórias, portanto um bip momentâneo não é critério suficiente. Se a leitura estabilizar em comportamento compatível com curto ou se a polaridade for incerta, parar e investigar.

### Passo 2 — primeiro power-on limitado

Usar fonte de bancada com limite de corrente e o caminho de alimentação documentado pela placa. Subir a alimentação de forma controlada, registrar tensão, corrente e estado do boot, e interromper se houver aquecimento, cheiro, consumo anômalo, reset repetitivo ou tensão fora do esperado. Não instalar bateria nesta etapa.

O primeiro firmware deve fazer somente identidade, boot, reset, leitura de alimentação disponível e testes locais de GPIO/SPI sem transmissão. Não ligar o microfone, não gravar na mídia e não ativar a LLM. O objetivo é transformar uma placa em uma unidade identificada, não demonstrar inteligência.

### Passo 3 — self-test sem RF irradiado

Testar USB/serial, botão de boot, reset, LED, display se presente, I2C, SPI e linhas de interrupção com o pin map confirmado. Para o SX1262, o firmware de bring-up deve verificar as transações e estados necessários ao port sem transmitir. O log dessa fase contém apenas revisão, estados, contagens, tensão, corrente, latência e digests.

Se o rádio precisar ser energizado para o teste, manter a transmissão desabilitada. Nenhum transmissor deve operar sem antena ou carga apropriada. Não usar o corpo, um fio solto ou uma entrada de osciloscópio como substituto de carga RF.

### Passo 4 — rádio conduzido e depois irradiado

Começar por um caminho conduzido com atenuador, dummy load e cabos apropriados, quando o método e a potência permitirem. Só depois fazer o teste irradiado a curta distância, com duas unidades idênticas, banda congelada, antenas identificadas e contagens pré-registradas. Registrar RSSI, SNR, pacotes, latência, potência configurada e falhas numéricas.

O resultado pertence somente à revisão, frequência, potência, antena, distância, ambiente e firmware testados. Ele não pode ser transformado em alcance geral, PDR universal, resistência a interferência, segurança de comunicação ou autonomia.

### Passo 5 — medir energia sem rádio

Instalar o monitor de energia em uma configuração que não altere o comportamento elétrico relevante. Validar sua orientação, shunt, taxa e unidade. Medir boot, idle, display, háptica, captura desabilitada e armazenamento separado. Depois repetir com workload congelado e registrar energia por workload, picos e latência.

A medição não deve misturar USB, bateria, rádio e carregador sem declarar o caminho. Não usar a duração de uma bateria de catálogo para deduzir autonomia. Autonomia só poderá ser calculada depois de workload, temperatura, taxa de transmissão, duty cycle, capacidade efetiva e cutoff serem observados no setup real.

### Passo 6 — integrar háptica

Conectar o DRV2605L ao I2C somente depois do pin map e dos níveis lógicos serem confirmados. Usar o motor ERM ou LRA especificado para o driver; não substituir por motor de corrente desconhecida sem medir. Fixar o motor mecanicamente, proteger os fios e medir corrente de repouso, efeito curto, efeito longo, cancelamento e comportamento após reset.

O padrão de interação deve ser deliberadamente limitado: uma vibração pode indicar que uma ação está **pronta para confirmação**, mas não pode confirmar, persistir ou enviar por conta própria. O botão físico deve produzir confirmação ou cancelamento observável e independente do modelo.

### Passo 7 — integrar o microfone somente em protocolo próprio

O microfone deve permanecer fisicamente desconectado durante o bring-up básico. Quando entrar, usar alimentação de 1,6–3,6 V conforme a especificação publicada do breakout, verificar clock/data/word-select e instalar mute físico. A primeira campanha deve usar sinais de teste não pessoais ou uma fonte controlada, nunca conversas privadas.

O software deve ter modo de microfone desabilitado por padrão. A bancada pode medir presença de dados, overflow, latência, energia e resultado categórico de um protocolo sintético. Não guardar áudio, transcrição, embedding, identidade, localização, prompt ou resposta. A captura não deve criar memória complementar sem confirmação física explícita.

### Passo 8 — integrar bateria e carregamento

Somente depois de a alimentação via fonte passar o gate, conectar a LiPo protegida pelo conector correto. Usar carregador CC/CV para LiPo; a referência da Adafruit alerta que a bateria deve ser carregada a no máximo 500 mA, não deve usar carregador NiMH/NiCad/chumbo-ácido e não deve ser carregada ou usada sem supervisão [4].

Inspecionar a célula antes de cada uso. Não dobrar, esmagar, furar, soldar diretamente em uma célula sem procedimento apropriado ou deixá-la pressionada pelo enclosure. Medir temperatura, corrente de carga, cutoff e comportamento de desconexão. Se houver inchaço, dano, aquecimento anormal ou cabo solto, retirar a bateria da campanha.

### Passo 9 — montagem mecânica provisória

No wearable, manter a antena, o pack, a placa e o ponto de contato háptico acessíveis para inspeção. Usar suporte removível e strain relief; não encapsular a primeira unidade. No puck, usar espaçadores, tampa removível, acesso USB, botão físico identificável e uma posição de antena que possa ser alterada sem destruir a unidade.

Medir paquímetro, massa, altura, posição da antena e temperatura. O resultado mecânico deve ser registrado como `mechanical-volume` apenas para aquele stack. Não inferir conforto, resistência a queda, impermeabilidade, segurança para a pele ou alcance a partir de uma caixa de protótipo.

### Passo 10 — recuperação e sessão física

Com o software de prova passando, usar a fixture de interrupção para cortar a alimentação em pontos pré-registrados: antes de `PREPARED`, depois de `PREPARED`, depois de autenticação, depois de `COMMITTED`, durante cleanup, antes e depois da elevação do piso, depois da quarentena de boot, durante cancelamento e em tentativa de replay após retorno.

O operador deve ler o estado somente depois do boot completar. O resultado físico é comparado ao oráculo host C11 e ao schema de evidência. Se uma capacidade ativa atravessar reboot, se o corte não for observado, se o digest divergir ou se surgir campo proibido, parar a campanha e marcar o resultado como falho ou bloqueado, nunca como média favorável.

## 6. Gates de aceitação e critérios de parada

| Gate | Pergunta | Evidência mínima | Estado inicial |
|---|---|---|---|
| `board-pin-map` | a unidade real corresponde ao esquema e ao HAL? | revisão, fotografia de identificação, esquema e self-test | `pending` |
| `power-safe-boot` | a placa inicializa sem risco elétrico? | limite de corrente, tensão, corrente, temperatura e boot | `pending` |
| `radio-bring-up` | o SPI/radio opera no perfil congelado? | variante, banda, terminação, contagens e estados | `pending` |
| `energy-instrumentation` | a energia é medível com método declarado? | instrumento, taxa, unidade, workload e integração | `pending` |
| `interaction-io` | confirmação, cancelamento e háptica são físicos? | trigger, latência, energia e resultado categórico | `pending` |
| `microphone-privacy` | a entrada pode ser desabilitada e não vaza para log? | mute físico, teste sintético e validação do schema | `pending` |
| `collection-crash-recovery` | interrupções seguem a matriz prevista? | trigger observado, pós-boot, ação e digest | `pending` |
| `collection-physical-session` | reboot não reativa capacidade indevida? | sessão nova, quarentena, exaustão, replay e digest | `pending` |
| `local-model-profile` | o workload local cabe no orçamento? | digest, pico de RAM, p95, energia e resultado de agência | `pending` |
| `mechanical-volume` | o stack medido cabe na caixa declarada? | paquímetro, stack e revisão mecânica | `pending` |

A campanha para imediatamente diante de risco elétrico, bateria danificada, RF sem terminação, revisão ambígua, instrumento sem método, reset não observado, resultado incompatível com o oráculo, campo proibido no log ou capacidade ativa após reboot. Uma mudança de placa, antena, bateria, firmware, modelo, supply, distância, rota, instrumento ou algoritmo cria nova revisão; não se pode otimizar depois de olhar o resultado e continuar chamando a série de igual.

## 7. O que não alegar antes de medir

Não alegar que o HERUS tem alcance longo porque existe um SX1262, que possui PDR ou WER porque o host passou, que tem autonomia porque a bateria diz 500 ou 1.200 mAh, que a LLM entende a pessoa porque responde em um caso, ou que a memória é privada porque o log não mostrou conteúdo. Também não alegar que a arquitetura é segura por ter TrustZone, secure boot ou flash encryption disponíveis no SoC; essas propriedades precisam ser configuradas, verificadas e testadas na unidade real.

Não alegar que o wearable é confortável, seguro em contato com a pele, resistente a queda, impermeável ou invisível. Não alegar que o puck é uma antena de longo alcance, carregador seguro ou estação de produção antes de medir RF, térmica, carga, isolamento, mecânica e recuperação. O puck pode vir a ser tudo isso, mas neste momento é uma hipótese de arquitetura.

> **Resultado honesto da etapa atual:** existe uma lista de montagem, uma bancada de baixo risco relativo e uma sequência reproduzível. Ainda não existe resultado físico.

## 8. Ordem prática de compra

A compra deve ser feita em ondas. Primeiro, adquirir as duas placas da mesma revisão, cabos de dados, antenas e fonte limitada. Depois, adquirir ou reservar multímetro, INA228, logic analyzer e, se disponível, osciloscópio. Só após o self-test comprar bateria, carregador, driver háptico e motor. O microfone deve ser a última parte do wearable, porque ele abre um protocolo de privacidade e não é necessário para provar boot, rádio, energia ou confirmação física.

Para o puck, começar com carcaça de papelão técnico, acrílico ou impressão 3D removível, sem colar componentes. A carcaça circular final, PCB customizada, storage de produto, bateria definitiva e certificação devem esperar os resultados dos gates. Essa ordem evita gastar em estética antes de saber se a alimentação, o rádio, a recuperação e a confirmação física realmente funcionam.

## Referências

[1] LILYGO, **T3 S3 V1.3**, página oficial do produto: <https://lilygo.cc/en-us/products/t3-s3-v1-3>.

[2] Raspberry Pi, **Buy a Raspberry Pi Pico 2 / RP2350**, página oficial: <https://www.raspberrypi.com/products/raspberry-pi-pico-2/>; RP2350: <https://www.raspberrypi.com/products/rp2350/>.

[3] Makerdiary, **nRF54L15 Connect Kit**, página do produto: <https://makerdiary.com/products/nrf54l15-connectkit>.

[4] Adafruit, **Lithium Ion Polymer Battery — 3.7 V 500 mAh**, página do produto: <https://www.adafruit.com/product/1578>.

[5] SparkFun, **Current Sensor — INA228 (Qwiic)**, página do produto: <https://www.sparkfun.com/sparkfun-current-sensor-ina228-qwiic.html>.

[6] Adafruit, **DRV2605L Haptic Motor Controller**, página do produto: <https://www.adafruit.com/product/2305>.

[7] Adafruit, **I2S MEMS Microphone Breakout — SPH0645LM4H**, página do produto: <https://www.adafruit.com/product/3421>.

[8] SparkFun, **915 MHz LoRa Antenna RP-SMA — 1/4 Wave 2 dBi**, página do produto: <https://www.sparkfun.com/915mhz-lora-antenna-rp-sma-1-4-wave-2dbi.html>.

[9] RAKwireless, **LoRa/LoRaWAN Store**, catálogo de módulos e preços publicados: <https://store.rakwireless.com/collections/lora-lorawan-fr>.

[10] Xe, **USD to BRL mid-market converter**, taxa consultada em 14 de agosto de 2026: <https://www.xe.com/en-us/currencyconverter/convert/?Amount=1&From=USD&To=BRL>.

[11] HERUS, **Protocolo de bancada física pré-registrado**, documento do repositório: [`docs/36-PROTOCOLO-BANCADA-FISICA.md`](36-PROTOCOLO-BANCADA-FISICA.md).

**Autor:** Manus AI
