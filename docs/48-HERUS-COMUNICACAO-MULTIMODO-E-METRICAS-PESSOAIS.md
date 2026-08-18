# HERUS multimodo: comunicação remota, urbana e telemetria pessoal

**Status:** arquitetura e contratos host-only para a próxima evolução; não é evidência de hardware, alcance, autonomia ou precisão biométrica.

**Objetivo:** permitir que o HERUS escolha entre LoRa para comunicação remota e enlaces Wi‑Fi/BLE para comunicação urbana ou local, sem alterar a autoridade, o escopo de privacidade ou a confirmação física exigida pelo sistema.

## 1. Decisão de produto

O HERUS não será um relógio com um único rádio. Será uma plataforma com uma interface semântica única e vários transportes especializados. A pessoa expressa uma intenção; o sistema calcula uma rota limitada; o transporte escolhido carrega somente o envelope autorizado. A escolha pode variar conforme disponibilidade, tamanho, urgência, energia, proximidade e ambiente, mas nunca conforme uma decisão autônoma de um modelo.

| Modo | Função | Payload adequado | Limite deliberado |
|---|---|---|---|
| **LoRa remoto** | Fallback off-grid, campo, estrada, área rural e store-and-forward limitado | Estado curto, pedido de ajuda, compromisso, confirmação, cartão semântico mínimo | Não transporta áudio, transcrição, embedding ou resposta longa |
| **ESP-NOW urbano** | Troca local de maior volume e baixa latência entre peers conhecidos | Cartões semânticos maiores, sincronização limitada e atualização local | Exige peer autenticado, canal compatível, ACK de aplicação e limites de tentativa |
| **BLE** | Descoberta, configuração, acessibilidade e conexão com acessórios próximos | Controle curto, revisão, ponte local e diagnóstico | Não é automaticamente uma rede confiável; toda entrega precisa de confirmação HERUS |
| **Wi‑Fi local** | Paper-Core, Dock, aplicativo local ou sincronização de maior volume | Pacotes locais versionados e cofre autorizado | Não é necessário para o valor básico e não concede autoridade adicional |

O SX1262 é documentado pela Semtech como um transceptor LoRa de longo alcance e baixo consumo, com suporte a LoRa, Long Range FHSS e FSK; seus números publicados de potência, sensibilidade, corrente e orçamento de enlace são características do componente, não resultados do HERUS [1]. O ESP32-S3 integra Wi‑Fi 4 em 2,4 GHz e Bluetooth 5 LE, incluindo Coded PHY e PHY de 2 Mbps [2]. O ESP-NOW é uma alternativa urbana plausível porque a Espressif o documenta como comunicação Wi‑Fi sem conexão, protegida por CCMP quando pares e chaves locais estão configurados [3].

## 2. Regra de seleção

A seleção é um **planejador de rota**, não um driver de rádio. Ele não abre sessão, cria HCP, escolhe identidade, toca em chave, serializa conteúdo nem transmite. Recebe apenas fatos tipados sobre ambiente, classe de payload, autorização física e autenticação do peer. Retorna uma recomendação limitada ou abstém.

A ordem não é simplesmente “sempre escolher o rádio mais rápido”. O HERUS deve preservar o objetivo sem aumentar exposição. Uma mensagem curta em ambiente remoto pode usar LoRa; um cartão maior com o Paper-Core próximo pode usar Wi‑Fi/ESP-NOW; uma sessão de configuração pode usar BLE. Se houver mais de uma rota equivalente, o planejador não deve expor a escolha como prova de entrega: o resultado é somente `route_selected` e o planner de entrega continua responsável por `SEND`, `RETRY`, `ACK`, expiração e cancelamento.

A cadeia de estados deve distinguir:

```text
route_unavailable
→ route_selected
→ frame_prepared
→ frame_sent
→ application_acknowledged
→ user_confirmed
```

`frame_sent` não significa `application_acknowledged`, e `application_acknowledged` não significa que a pessoa autorizou uma nova memória, exportação ou ação. A troca de LoRa por ESP-NOW nunca pode aumentar o escopo de autorização.

A documentação do ESP-NOW informa que o sucesso retornado na camada MAC não garante entrega na camada de aplicação e recomenda ACK de aplicação, retransmissão controlada e número de sequência para duplicatas [3]. Por isso, o contrato do HERUS exige a separação entre o evento do rádio e o sucesso semântico.

## 3. Privacidade por classe de payload

A camada de transporte não recebe texto livre nem dados de sensores brutos. O envelope superior fornece somente uma classe tipada, tamanho limitado, escopo e consentimentos já obtidos. O conteúdo continua sob os gates existentes.

| Classe | Pode usar LoRa? | Pode usar urbano? | Consentimento adicional |
|---|---:|---:|---|
| Estado essencial | Sim | Sim | Sessão e peer autenticado |
| Cartão semântico | Sim, mínimo | Sim | Confirmação de compartilhamento |
| Telemetria de bem-estar | Não por padrão | Somente com compartilhamento separado | Consentimento de coleta e de exportação |
| Localização | Não no contrato inicial | Não no contrato inicial | Deve ser uma evolução própria, expirada e explicitamente autorizada |
| Áudio/transcrição/embedding | Não | Não | Fora do produto de transporte |

Nenhum identificador estável, chave, localização, áudio, transcrição ou embedding entra em métricas de produto. Logs podem registrar contagens, estados terminais, classe abstrata e razões de abstention, mas não valores pessoais nem endereços.

## 4. Métricas pessoais no estilo de um wearable de fitness

O HERUS pode medir dados de movimento e bem-estar como um relógio esportivo, mas deve nomear isso corretamente: **telemetria pessoal estimada**, não diagnóstico. O aviso oficial de precisão da Garmin afirma que wearables dependem de sensores e que a precisão varia conforme ajuste, atividade e usuário; a própria Garmin declara que seus wearables não são dispositivos médicos e que os dados não são destinados a diagnóstico ou uso médico [4].

A primeira camada deve aceitar somente métricas derivadas e tipadas, nunca o sinal bruto do sensor. Exemplos compatíveis com um futuro hardware incluem passos, minutos ativos, distância estimada, frequência cardíaca estimada, duração de sono, temperatura de pele e gasto energético estimado. Cada amostra precisa informar origem, unidade, qualidade, janela temporal e consentimento. Um valor indisponível ou de baixa qualidade deve permanecer indisponível; o HERUS não pode preencher lacunas com inferência silenciosa.

| Campo | Exemplo | Pode persistir automaticamente? |
|---|---|---:|
| Tipo | `steps`, `heart_rate`, `active_minutes` | Não |
| Valor inteiro com unidade | `7420 steps`, `78 bpm` | Não |
| Qualidade | `unknown`, `low`, `usable` | Não |
| Origem | acelerômetro, PPG, entrada manual | Não |
| Janela | início e fim monotônicos | Não |
| Consentimento de coleta | sessão vigente | Não |
| Consentimento de retenção | gesto físico separado | Sim, somente após confirmação |
| Consentimento de compartilhamento | gesto e destinatário separados | Sim, somente para o envelope aprovado |

O contrato host-only desta etapa funciona como filtro de segurança para os futuros drivers. Ele não implementa PPG, acelerômetro, GNSS, sono, calorias ou classificação de atividade. Também não declara nenhuma acurácia, WER, consumo, bateria ou correlação com equipamento médico.

## 5. Consentimento e ciclo de vida

A coleta de telemetria não deve ser um interruptor permanente escondido em uma configuração. O usuário precisa escolher, separadamente, se permite observar uma métrica, se permite retê-la no cofre local e se permite compartilhá-la. A captura pode ser interrompida por mute físico, revogação, expiração, baixa qualidade ou perda da sessão.

A sequência segura é:

```text
sensor local → amostra tipada → validação de faixa → qualidade → revisão
→ confirmação física de retenção → cartão de telemetria local
→ confirmação separada de compartilhamento → envelope mínimo
```

O mesmo princípio vale para a troca de transporte. Um rádio disponível não é consentimento; uma conexão autenticada não é consentimento; uma inferência de relevância não é consentimento. O sistema pode propor, mas a pessoa decide.

## 6. Implementação desta evolução

A próxima PR adicionará dois contratos C11 host-only:

1. `transport_selector.{h,c}` selecionará LoRa, ESP-NOW, BLE ou Wi‑Fi somente a partir de fatos tipados, com limites de tamanho, latência, privacidade, peer e autorização. Ele nunca transmitirá.
2. `personal_telemetry.{h,c}` validará amostras derivadas, exigirá sessão e consentimento, manterá o valor somente em estado transitório e emitirá um registro persistível apenas após confirmação física. Ele não contém buffer de áudio, sinal bruto, identidade, localização ou chave.

As suítes adversariais testarão indisponibilidade, conflito entre rotas, payload excessivo, ausência de peer, tentativa de usar telemetria sem consentimento, valores fora de faixa, baixa qualidade, expiração, revogação, rejeição, compartilhamento indevido e limpeza transitória.

## 7. O que ainda depende de hardware

A arquitetura é verificável no computador, mas a utilidade real dependerá de bancada. Será necessário medir coexistência LoRa/Wi‑Fi/BLE, consumo por modo, latência, perda, interferência, canal, alcance em cidade e campo, conforto, cobertura da antena, erro de sensores, temperatura de pele, frequência cardíaca, GNSS, armazenamento e comportamento após reboot. Até lá, o HERUS deve dizer somente que os contratos host-only passam.

## Referências

[1] [Semtech — SX1262 LoRa Connect Transceiver](https://www.semtech.com/products/wireless-rf/lora-connect/sx1262)

[2] [Espressif — ESP32-S3 Wi‑Fi & BLE 5 SoC](https://www.espressif.com/en/products/socs/esp32-s3)

[3] [Espressif — ESP-NOW Programming Guide](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/network/esp_now.html)

[4] [Garmin — Activity Tracking and Fitness Metric Accuracy](https://www.garmin.com/en-US/legal/atdisclaimer/)

[5] [LoRa Alliance — LoRaWAN Specification v1.1](https://resources.lora-alliance.org/technical-specifications/lorawan-specification-v1-1)
