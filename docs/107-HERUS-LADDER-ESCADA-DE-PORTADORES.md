# HERUS — Ladder: um significado, todos os canais

**Prova:** `make -C firmware hsca-ladder`

## 1. A pergunta certa

`transport_selector.{h,c}` responde "qual dos quatro rádios leva este envelope?". A Ladder responde a pergunta que só existe depois que a unidade de comunicação vira um significado de 34 bytes: **quais canais do mundo são largos o bastante?**

| Degrau | Alcance declarado | Latência | Classe máxima | Precisa de |
|---|---:|---:|---|---|
| `touch` — toque NFC | 0 m | 120 ms | pacote | proximidade física |
| `glyph` — símbolo óptico lido por câmera | 5 m | 1,5 s | cartão | linha de visada |
| `sound` — acoplamento quase-ultrassônico | 10 m | 2,8 s | essencial | — |
| `ble` | 30 m | 40 ms | telemetria | par provisionado |
| `ble-coded` — LE Coded PHY S=8 | 120 m | 160 ms | telemetria | par provisionado |
| `esp-now` | 180 m | 20 ms | telemetria | par provisionado |
| `wifi` — Core / Paper-Core | 60 m | 15 ms | pacote | infraestrutura |
| `lora` — SX1262 SF9, dentro do dwell | 650 m | 247 ms | cartão | par provisionado |
| `lora-mesh` — Weave, três saltos | 1.900 m | 900 ms | cartão | par provisionado |
| `sat` — rajada curta pela antena do Core | 4.000 km | 20 s | essencial | Core + visada |
| `drift` — custódia | cresce com o tempo | horas | essencial | pessoas |

**Um significado de 34 bytes cabe em 11 dos 11 degraus. Quatro segundos de fala comprimida cabem em 8.** Os dois números são calculados sobre a tabela e asseridos na suíte — se um degrau parar de aceitar um significado completo, o build quebra.

## 2. Honestidade da tabela

Todo número acima é **declarado** a partir de datasheet ou especificação. Nenhum foi medido em hardware do HERUS. `ldr_unmeasured_count()` devolve **11 de 11** e a suíte assere esse valor, exatamente para que nenhum documento futuro esqueça disso.

Trocar esses 11 números declarados por 11 medidos é trabalho de Fase 0 em diante, um degrau por vez.

## 3. Elegibilidade

Um degrau só entra no plano se passar por todos os filtros:

```
disponível  ·  tamanho cabe  ·  classe do payload ≤ classe do degrau
peer provisionado se exigido  ·  Core presente se exigido
linha de visada se exigida  ·  alcance declarado ≥ distância do destinatário
```

O último filtro nasceu de um furo encontrado ao compor o sistema: sem ele, o planejador oferecia **NFC para alguém a quatro quilômetros**, porque toque é o degrau mais barato em energia. Alcance agora é conferido, não torcido.

## 4. Ordenação

- **Rotina** ordena por energia. **Urgente** ordena por latência. Alcance desempata: em igualdade, o significado vai mais longe.
- **Custódia é sempre o último degrau**, nunca ranqueada. Entregar uma cópia a quem passa não custa nada à pessoa e é o único degrau cujo alcance cresce depois do envio.
- O mesmo conjunto de fatos sempre produz a mesma escada. Sem sorteio, sem estado escondido.
- `widest_reach_dropped` diz quando um degrau mais largo existia mas não era elegível — e a suíte também prova que ele **não** grita quando nada mais largo era possível.

## 5. Fronteira de autoridade

`ldr_plan()` é conselho e pode ser chamado antes de qualquer confirmação — é isso que permite mostrar à pessoa por onde a mensagem iria **antes** de ela decidir. `ldr_commit()` é a única função que olha a confirmação, e ela recusa quando:

| Situação | Resultado |
|---|---|
| sem confirmação física | `LDR_E_UNCONFIRMED`, e nenhum portador é nomeado |
| classe confirmada ≠ classe do plano | `LDR_E_CLASS_DRIFT` |
| plano vazio | `LDR_E_EMPTY_PLAN` |

**Trocar de degrau nunca aumenta autorização.** A classe do payload é copiada dos fatos e nunca reescrita pelo planejador; perder um portador não muda o que pode ser enviado.

## 6. O que isto não é

- Não é driver, não abre sessão, não toca em chave, não serializa quadro e não transmite.
- Não substitui o `delivery_plan`, que continua dono de `SEND`, `RETRY`, `ACK`, expiração e cancelamento.
- `frame_sent` continua não significando `application_acknowledged`, como já dizia [48](48-HERUS-COMUNICACAO-MULTIMODO-E-METRICAS-PESSOAIS.md).
- Satélite depende da antena do Core por decisão de produto, não por limitação temporária: é a forma de o Core ser útil sem ser cérebro.
