# 11 — Gateway semântico de confiança

**Avanço 5 de 10 · HERUS-A5-001 · contrato de intenção local e não-autônoma**

> Um resultado de ASR não é uma mensagem. É uma observação local, com fonte, sessão, comando e incerteza. No HERUS, essa observação só pode virar um rascunho depois de passar por um gate de confiança; um rascunho só pode virar envio após confirmação física já imposta pelo runtime.

O Avanço 5 fecha a lacuna entre adaptadores reais de ASR e o runtime do Avanço 2. A implementação não recebe áudio, transcrição, nuvem, chave, radio ou estado de transporte. Ela recebe um comando tipado de uma fonte de ASR **local**, sua pontuação primária/alternativa, a sessão de botão que está ativa e, opcionalmente, uma dica de contexto já aprendida pelo Núcleo.

A escolha de comandos controlados é intencional. A documentação do ESP-SR descreve o MultiNet como reconhecimento offline de múltiplos comandos no ESP32, com entrada processada por AFE a 16 kHz/16-bit mono e identificação de comando configurável. [1] Isso é compatível com um contrato de comando tipado; não é prova de que uma frase livre, uma transcrição ou qualquer score isolado possa acionar transmissão.

## 1. Limite de autoridade

| Camada | Pode fazer | Não pode fazer |
|---|---|---|
| Adaptador ASR local | Produzir `command_id`, confiança primária, confiança alternativa e `session_id` | Criar frame, confirmar, enviar, mudar léxico ou persistir áudio no gateway |
| Gateway de confiança | Aceitar, rejeitar ou declarar ambiguidade de um comando da sessão ativa | Criar uma intenção ausente, elevar baixa confiança, enviar ou alterar o Núcleo |
| Núcleo | Fornecer dica de intenção já aprendida com suporte/confiança explícitos | Substituir comando ASR, inventar intenção, aceitar confirmação ou transmitir |
| Runtime | Criar rascunho, pedir confirmação, expirar e liberar handoff único | Chamar rádio, reter áudio ou pular a confirmação física |
| Aplicação/rádio | Preencher transporte e chamar `link_send` após handoff | Reutilizar handoff, contornar `READY_SEND` ou tratar sugestão como comando |

A separação responde ao problema de avaliações de fala que tratam diferentes tarefas separadamente. O plano OpenSAT do NIST diferencia detecção de fala, busca de palavras e ASR; a WER é uma métrica de transcrição em relação a referência, não uma autorização de ação. [2] O HERUS mede confiança de comando, ambiguidade, falsos rascunhos e confirmação como propriedades distintas.

## 2. Contrato de sessão

A cada `push-to-talk`, o runtime cria um `session_id` monotônico, não nulo. Um resultado de ASR deve carregar esse mesmo identificador. Resultado de outra sessão, anterior ou futura, é **stale**: é ignorado sem parar a captura e sem produzir vibração, rascunho ou mudança de estado. Assim, uma resposta atrasada do Núcleo não pode contaminar uma nova interação.

| Campo de entrada | Regra |
|---|---|
| `source` | Somente `core` ou `nucleus`; ambos devem executar localmente |
| `session_id` | Deve coincidir exatamente com a sessão ativa de botão |
| `command_id` | `arrive`, `help` ou `cancel`; nenhum comando livre é inferido |
| `confidence_pct` | 0–100; abaixo de 80% falha fechado |
| `runner_up_pct` | 0–100 e não superior ao principal; margem deve ser pelo menos 15 pontos |
| `context_hint` | Opcional, local, de uma intenção já autorizada e com suporte ≥ 3/confiança ≥ 70% |

## 3. Regra de decisão

O gate aplica as condições abaixo na ordem apresentada. Cada caminho é determinístico, auditável e independente de rádio.

| Condição | Resultado | Efeito no runtime |
|---|---|---|
| Sessão incompatível | `stale` | Ignora; continua escutando |
| Fonte/comando/score malformado | `rejected` | Encerra captura; vibração de rejeição |
| Confiança < 80% | `low_confidence` | Encerra captura; vibração de repetição/indefinido |
| Confiança ≥ 80% e margem ≥ 15 | `accepted_direct` | Cria rascunho local, pede confirmação física |
| Confiança ≥ 80% e margem < 15, sem dica qualificada que concorde | `ambiguous` | Encerra captura; vibração de repetição/indefinido |
| Confiança ≥ 80% e margem < 15, dica qualificada que concorde | `accepted_context` | Cria rascunho local, marca métrica de contexto; pede confirmação física |

A dica do Núcleo só quebra um empate quando o comando primário já está acima do limiar e a dica aponta para **o mesmo** significado. Ela nunca resgata baixa confiança, troca o comando, adiciona tempo, cria ajuda ou altera uma mensagem. Mesmo em `accepted_context`, o padrão háptico apenas apresenta um rascunho e o envio permanece bloqueado por confirmação física. O Núcleo já foi projetado para aprender apenas a partir de significados autorizados e devolver sugestões com suporte e confiança; o gateway não amplia essa autoridade. [3]

## 4. Telemetria e investigação

O gateway acrescenta apenas contadores: aceites diretos, aceites assistidos por contexto, ambiguidades, baixa confiança, resultados obsoletos e rejeições. Ele não registra comando, intenção, transcrição, áudio, score individual, contexto, chave, endereço ou localização. A investigação A4 mede os contadores agregados por fonte; resultados de ASR usados para ajustar limiares permanecem exploratórios até que um novo plano seja congelado. [4]

## 5. Critérios de prova

A suíte do gateway prova, em host, que resultado obsoleto não toca o runtime, baixa confiança não vira rascunho, contexto não promove comando fraco ou diferente, e toda aceitação ainda precisa de confirmação/handoff único. A prova não afirma desempenho de Microfone, AFE, MultiNet ou Núcleo físico; ela garante que, quando esses adaptadores existirem, não terão um atalho lógico para transmissão.

## Referências

[1] [Espressif ESP-SR — MultiNet Command Word Recognition](https://docs.espressif.com/projects/esp-sr/en/latest/esp32/speech_command_recognition/README.html)
[2] [NIST OpenSAT20 Evaluation Plan](https://www.nist.gov/document/2020opensat20evaluationplanv16)
[3] [Núcleo HERUS — contrato local](06-NUCLEO.md)
[4] [HERUS-A4-001 — investigação pré-registrada](10-INVESTIGACAO-PREREGISTRADA.md)
