# Ambiente virtual pré-hardware do HERUS

**Status:** bancada virtual determinística integrada ao simulador existente.

**Resultado desta etapa:** o cenário `virtual` combina a rede LoRa já simulada com os contratos reais de seleção multimodo e telemetria pessoal. Ele passa em host, executa uma campanha combinada de rádio e é acompanhado por um gate de mutação que exige que controles removidos sejam detectados antes de ligar o primeiro devkit.

> **Definição de honestidade:** uma passagem virtual prova que o código e o modelo obedeceram às regras declaradas sob as entradas simuladas. Ela não prova que a antena, o sensor, a bateria ou o ambiente físico obedecerão ao modelo.

## 1. Por que não basta emular a CPU

Uma emulação de ESP32 pode ser valiosa para boot, memória, flash, UART, depuração e alguns periféricos. A documentação oficial do ESP-IDF descreve o fork de QEMU da Espressif como capaz de emular CPU, memória e vários periféricos, além de permitir executar e depurar aplicações sem gravá-las em um chip real [1]. Isso é uma trilha importante, mas não substitui a modelagem do sistema HERUS: o SX1262, a antena, a bateria, o corpo no pulso, PPG, acelerômetro, GNSS, temperatura, ruído acústico e coexistência espectral precisam de modelos separados.

A própria documentação oficial de testes do ESP-IDF separa testes no alvo, testes Linux-host com mocks e testes multi-dispositivo ou multi-estágio. Ela observa que o suporte host ainda cobre apenas uma fração dos componentes e explica que mocks controlam dependências, mas não transformam o mock em evidência do componente físico [2]. O ambiente virtual do HERUS segue essa distinção em vez de esconder a incerteza.

O Renode é uma possível trilha futura para firmware multi-nó: sua documentação expõe máquinas, redes, testes, modo headless, salvamento/restauração de estado e dados de sensores virtuais [3]. Nesta etapa ele não foi tornado dependência obrigatória, porque ainda seria necessário comprovar suporte exato ao ESP32-S3, à LilyGO T3-S3 e ao SX1262 específico do projeto.

## 2. Arquitetura em camadas

A bancada virtual não duplica o firmware. Ela reutiliza o simulador `sim/`, que já compila os módulos reais de `firmware/core` e `firmware/net` sem uma variante específica de simulação. Sobre esse mundo, o novo cenário injeta observações tipadas nos contratos do Watch e do transporte.

| Camada | O que é executado | Tipo de evidência |
|---|---|---|
| **Semântica e autoridade** | `transport_selector.c`, `personal_telemetry.c`, módulos existentes de memória e interação | Contrato C11 host-only e invariantes adversariais |
| **Rede LoRa** | `hcp`, `session`, `crypto`, `weave`, `beat`, `region` e `link` reais | Frames reais do firmware, com tempo, distância, perda e relógio modelados |
| **Ambiente** | Cenários remotos, urbanos, local, disponibilidade, atraso, falha, expiração e agressor | Execução determinística com entradas fixadas |
| **Sensores** | Amostras derivadas roteirizadas, com origem, faixa e qualidade | Prova de política; não precisão física |
| **Bateria** | Contabilidade abstrata em unidades virtuais e rejeição de overspend | Prova de ordenação e admissão; não consumo em mAh |
| **Hardware** | Ainda não substituído completamente | Lista de lacunas que exige Fase 0 de bancada |

A escolha foi deliberada: existe um único relógio de eventos para o rádio real simulado, mas o roteiro de Watch, Paper-Core, sensor e bateria é uma camada de composição. Ele não altera o algoritmo de criptografia ou o protocolo que o cenário LoRa executa.

## 3. Cenário `virtual`

Os comandos podem ser executados com:

```bash
cd sim
make virtual
make virtual-mutation
```

O primeiro executa a bancada; o segundo recompila sete versões deliberadamente inseguras e falha se qualquer uma sobreviver.

Ele também participa da execução padrão de `make` e de `../prove.sh`, porque foi registrado na tabela principal do executável. O código retorna falha se qualquer invariável falhar; não há um relatório que possa declarar sucesso sem testar uma condição.

A execução atual verifica 37 invariantes neste cenário e ainda executa uma campanha de mutação independente:

| Grupo | Verificações |
|---|---|
| **Roteamento** | LoRa para estado remoto, ESP-NOW para cartão urbano, BLE para controle local, abstention quando o modo remoto fica indisponível e ausência de fallback silencioso para outro contexto |
| **Telemetria** | Sessão consentida, amostra derivada utilizável, candidato transitório, confirmação física para retenção, compartilhamento separado e custo virtual delimitado |
| **Adversários** | Baixa qualidade, expiração, revogação física e recusa de transmissão quando a bateria virtual não comporta o custo declarado |
| **LoRa real dentro do modelo** | Mensagem remota entregue por `firmware/net`, peer autenticado abrindo o frame e zero falsa entrega |
| **Energia abstrata** | Caminho normal abaixo da capacidade roteirizada e rejeição explícita de overspend |
| **Campanha combinada** | Dezesseis mundos determinísticos combinam sombra, distância, canais, CAD, retries, relay, replay, forgery e jamming; nenhum produz falsa entrega |
| **Mutação** | Sete mutantes removem autoridade, privacidade, qualidade, sessão, confirmação ou wraparound; todos precisam ser mortos pelo cenário |

A mensagem mais importante do cenário é negativa: uma rota pode ser recomendada, mas a camada de energia pode recusá-la; uma amostra pode ser relevante, mas expirar antes da confirmação; uma telemetria pode ser retida localmente, mas continuar impedida de ser compartilhada sem outro consentimento.

## 4. O que já pode ser tratado como comprovado no ambiente virtual

A partir desta etapa, o HERUS tem evidência executável para afirmar que, sob as entradas do cenário, a seleção de transporte é determinística, a mudança de meio não aumenta autoridade, a telemetria de baixa qualidade não é inventada, a confirmação física é necessária para retenção, a expiração é terminal, a bateria virtual não sofre overspend e o enlace LoRa usa o caminho criptográfico real do firmware.

Essa evidência é mais forte que uma demonstração visual ou um mock sem falhas porque cada caso tem uma condição que pode falhar e porque o cenário inclui revogação, indisponibilidade, qualidade baixa, expiração e bateria insuficiente. Ainda assim, a força da afirmação é delimitada pelo domínio das entradas simuladas.

## 5. O que continua exclusivamente de hardware

| Propriedade | Por que a simulação não basta | Primeiro teste físico necessário |
|---|---|---|
| **Alcance LoRa** | Propagação, antena, corpo, orientação e interferência diferem do modelo | Duas placas, antena definida, rota medida, RSSI/SNR e perda por distância |
| **Coexistência urbana** | Wi‑Fi, BLE, ESP-NOW e LoRa compartilham um ambiente eletromagnético real | Capturar ocupação, latência, perda e consumo com rádios alternados |
| **Bateria** | Corrente de TX/RX/MCU depende de firmware, regulador, flash, PSRAM, temperatura e célula | Medição no trilho de alimentação durante cada estado e transição |
| **Métricas pessoais** | PPG, acelerômetro, temperatura e GNSS variam com ajuste, pele, movimento e sensor | Comparação pré-registrada com protocolo de repetição; sem chamar de diagnóstico |
| **Voz e ruído** | Microfone, caixa, vento, fala distante e wake word não são representados pelo contrato tipado | Gravação consentida de bancada, taxa de falso despertar, rejeição e privacidade |
| **Háptica** | Uma flag de confirmação não informa se a pessoa entendeu a vibração | Estudo de interação com padrões, erro e tempo de confirmação |
| **Ergonomia** | Peso, calor, carregamento, água, impacto e posição do pulso são físicos | Protótipo vestível com critérios de desconforto e falha |

Até que esses testes existam, o README deve continuar chamando o projeto de **pré-hardware**. O simulador prepara a bancada e reduz o número de incógnitas; ele não autoriza trocar uma hipótese de hardware por um número bonito.

## Referências

[1] [Espressif — QEMU Emulator for ESP32](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-guides/tools/qemu.html)

[2] [Espressif — Unit Testing in ESP32](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-guides/unit-tests.html)

[3] [Renode — Documentation](https://renode.readthedocs.io/en/latest/)

[4] [HERUS — Simulated Bench README](../sim/README.md)
