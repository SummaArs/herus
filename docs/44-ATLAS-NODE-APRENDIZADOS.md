# Aprendizados do Atlas_Node para o HERUS

**Status:** análise comparativa e incremento host-only; não é certificação do Atlas_Node nem prova física do HERUS.  
**Fonte principal:** [`DiyYari/Atlas_Node`](https://github.com/DiyYari/Atlas_Node), branch `main`, commit analisado `51e593f`, observado em 17 de agosto de 2026.  
**Resultado aplicado:** planner C11 de entrega limitada após autorização explícita.

## 1. Escopo da análise

O Atlas_Node é um pacote source-only multiplataforma com firmware ESP32-S3, aplicativos Android, Apple e Windows, integração BLE, rádio SX1262/RA-01SH, GPS, voz, arquivos, mapas e modelos locais/externos. O próprio repositório declara que modelos de IA, áudio salvo, bases locais, binários e frameworks pré-compilados foram removidos do pacote público [1] [5].

A análise não trata README, comentários ou números declarados como medições. Código presente demonstra intenção e uma implementação possível; somente build, testes, instrumentação e bancada demonstram comportamento. Não foram copiados arquivos do Atlas_Node para o HERUS.

## 2. O que o Atlas faz de forma concreta

| Área | Implementação observada | Relevância para o HERUS |
|---|---|---|
| Firmware | Arduino/PlatformIO em `lolin_s3_mini`, com RadioLib, NimBLE e TinyGPSPlus | Mostra uma ponte prática ESP32-S3 ↔ SX1262 ↔ BLE |
| Rádio | SX1262/RA-01SH, SPI, DIO1, BUSY, reset e RXEN/TXEN | Reforça que o RF switch deve estar no pin map e no bring-up |
| Aplicação | Windows, Android e Apple reutilizam comandos e frames textuais | Mostra necessidade de uma camada de transporte multiplataforma |
| IA | Ollama/Whisper/GGUF no host ou telefone, não no rádio | Compatível com a decisão de concentrar modelo maior no Core/Paper/telefone |
| Dados | SQLite para mensagens, contatos, perfil e histórico de chat | Útil como exemplo de isolamento de persistência; inseguro como default HERUS |
| UX | estados de envio, entrega, leitura, retry, reconnect e watchdog | Fonte prática para uma futura camada HERUS Link |
| Operação | perfis de rádio e diagnósticos controláveis pelo aplicativo | Útil somente se submetido a região, autorização e gates de bancada |

## 3. Aprendizados que devemos absorver

### 3.1 Estado explícito de transporte

O Atlas mantém filas, tráfego de comando separado de voz, timeout de escrita, retry finito, cancelamento e estados de entrega. Esse padrão é valioso porque torna uma conexão BLE imperfeita observável e evita várias escritas simultâneas [2] [3].

No HERUS, a mesma ideia deve ser reimplementada sobre envelopes binários e tipados. O transporte pode organizar uma ação já autorizada, mas nunca criar autorização, HCP, chave, memória ou mensagem por conta própria.

### 3.2 Prontidão antes de transmitir

O gerenciador BLE do Atlas aguarda descoberta de características e notificação antes de marcar o enlace como pronto, mantém watchdog e tenta recuperação limitada [3]. Isso é uma boa regra para o futuro HERUS Link: “conectado” não significa “pronto para uma transação”; a capacidade de escrever, receber confirmação e cancelar precisa ser observável.

### 3.3 Disciplina do RF switch

O firmware Atlas possui uma função dedicada para alternar RXEN/TXEN e trata BUSY antes de mudar o rádio [4]. Isso é uma prática de bring-up que o HERUS deve preservar, mas os pinos não podem ser copiados entre placas. Cada montagem exige pin map, esquema, datasheet do módulo e teste com corrente limitada.

### 3.4 Diagnósticos separados do caminho normal

O Atlas mantém diagnósticos de rádio, contadores de TX/RX, fila e erros. O HERUS pode aproveitar a ideia de um perfil de bancada separado do perfil de produto. Diagnósticos não devem ativar CW, mudar frequência ou elevar potência de transmissão sem um modo de laboratório explicitamente autorizado e protegido.

### 3.5 IA concentrada fora do wearable

O Atlas coloca Whisper e o modelo generativo no host, usando o microcontrolador como ponte de rádio/BLE. Isso reforça a arquitetura HERUS Paper/Core/One: o wearable deve executar interação e protocolo pequeno; o modelo pode viver no Core ou no telefone, sempre atrás do compilador de proposta tipada.

## 4. O que não deve ser copiado

### 4.1 Gramática textual ampla como protocolo de confiança

O firmware aceita comandos textuais como `SET_POWER`, `SET_FREQ`, `SET_REGION`, `SET_PROFILE`, diagnósticos, GPS, voz, arquivos e mensagens dentro de uma única ponte [4]. A flexibilidade é útil para um protótipo pessoal, mas torna difícil provar autoridade, delimitação, versionamento e rejeição.

O HERUS deve transportar envelopes tipados, com versão, finalidade, escopo, sequência, expiração e autenticação. Um texto recebido nunca deve ser interpretado diretamente como mudança de configuração, memória ou ação.

### 4.2 Relay oportunista e GPS por padrão

O Atlas inclui cabeçalho de mesh, cache de duplicatas, TTL e relay de tráfego, além de forwarding de GPS. Esses recursos podem ser úteis em um cenário específico, mas criam risco de amplificação, rastreamento e divulgação involuntária de presença. No HERUS, relay precisa ser um perfil explícito, autenticado e medido; GPS não faz parte do ledger de produto e não deve ser habilitado por default.

### 4.3 Criptografia de aplicação insuficiente para soberania

O Atlas possui AES-GCM opcional, mas o módulo analisado também deriva chave de senha com SHA-256 e salt fixo, expõe uma representação hexadecimal da chave, imprime preview da chave e mantém uma chave default de grupo em outro caminho. O HMAC de grupo é truncado e aplicado como sufixo de texto. Isso pode proteger um protótipo cooperativo, mas não constitui uma identidade soberana nem um protocolo de sessão robusto [2] [6].

O HERUS deve usar o modelo já existente de sessão: AEAD, chaves por sessão/direção, anti-replay, expiração, rotação e armazenamento protegido. A chave nunca deve aparecer em logs, UI, mensagens ou schema de evidência.

### 4.4 Persistência de conteúdo bruto

O Atlas grava mensagens textuais e histórico de chat em SQLite; o fluxo de áudio cria arquivos WAV e o rádio mantém diretórios de áudio, downloads e arquivos recebidos [2] [5]. Isso é contrário ao objetivo do HERUS de memória seletiva e logs numéricos. O HERUS deve persistir apenas cartões autorizados e a proveniência mínima necessária; áudio, transcrição, embedding, localização e conteúdo de produto ficam fora do ledger de diagnóstico.

### 4.5 Números sem ensaio reproduzível

O `pin.txt` declara valores de alcance, corrente, potência e comportamento de GPS, mas a análise do repositório não encontrou uma suíte própria de CI/conformance que transforme essas declarações em evidência de produto. O HERUS não adotará esses números como resultado. Alcance, PDR, WER, energia, autonomia, temperatura e latência continuam pendentes de bancada.

### 4.6 Licença e proveniência

O pacote não possui um arquivo de licença na raiz. Há licenças dentro de partes vendorizadas, e as próprias notas recomendam revisar llama, ggml, codec2, MapLibre e outras dependências [5]. O HERUS não incorpora código Atlas; uma eventual reutilização futura exigirá auditoria de licença, atribuição e cadeia de proveniência.

## 5. Comparação com os contratos HERUS

| Propriedade | Atlas_Node | HERUS atual | Decisão |
|---|---|---|---|
| Unidade de comunicação | linha textual `MSG`/comandos/voz/GPS | HCP tipado em frame de tamanho constante | manter HCP |
| Confidencialidade | opcional, em partes do payload | sessão AEAD com chaves de uso único | manter sessão HERUS |
| Retry | fila/timeout no BLE e retries de aplicação | novo planner limitado, sem repetir bytes | adaptar com dedup no enlace |
| Deduplicação | hashes/cache de mesh | sessão/replay e `link_dedup` autenticado | manter dedup pós-AEAD |
| Autoridade | comando do app pode alterar operação do rádio | confirmação física e gates separados | não relaxar |
| Memória | histórico textual e áudio local | cartões seletivos e temporários | não copiar persistência |
| IA | modelo gera texto de chat | modelo gera proposta tipada | manter compilador |
| Identidade | IDs de dispositivo e nomes | binding, sessão, rotação e revogação | evoluir com IDs efêmeros |
| Diagnóstico | strings e telemetria ampla | métricas numéricas permitidas | limitar logs |

## 6. Incremento aplicado ao HERUS

A ideia mais segura e concreta do Atlas para ser absorvida imediatamente é o **estado de entrega com fila, deadline, retry finito, ACK, cancelamento e watchdog**. A implementação está em:

- `firmware/core/delivery_plan.h`;
- `firmware/core/delivery_plan.c`;
- `firmware/core/test_delivery_plan.c`;
- alvo explícito `make -C firmware delivery-plan`.

O planner tem seis limites essenciais:

1. começa somente quando o chamador fornece `authorized_handoff == 1`;
2. exige um `app_seq` não nulo, deadline futuro e no máximo três tentativas;
3. não permite sobrescrever uma entrega que ainda aguarda ACK;
4. emite apenas eventos `SEND`, `RETRY`, `EXPIRED`, `DELIVERED`, `CANCELLED` ou `FAILED`;
5. nunca cria HCP, chama rádio, acessa chave, escolhe peer ou repete bytes;
6. exige que o chamador crie um novo frame selado para cada retry, mantendo o mesmo `app_seq` e consumindo um novo contador de sessão.

O ACK só deve ser encaminhado ao planner depois de ser autenticado pelo enlace. O planner apenas verifica se a sequência corresponde ao plano ativo. Portanto, esta melhoria não transforma ACK de aplicação em prova criptográfica por si só.

## 7. Invariantes adicionadas

A suíte host prova que:

- uma tentativa sem autorização física é rejeitada e limpa;
- argumentos inválidos não deixam estado parcialmente armado;
- a primeira poll produz um único `SEND`;
- poll antecipado não duplica o evento;
- retries são finitos e limitados a três tentativas;
- ACK de outra sequência não encerra o plano;
- ACK antes do primeiro envio é rejeitado;
- sucesso é terminal;
- expiração e cancelamento impedem futuras tentativas;
- ACK tardio não reabre plano expirado;
- o planner não recebe nem manipula conteúdo, chave ou frame.

## 8. O que fica para uma próxima fase

O próximo incremento de transporte pode adaptar a fila BLE do Atlas ao HERUS Link, mas deverá ser feito sobre o contrato de sessão existente. O trabalho precisa incluir envelope de operação, deadline, reason code numérico, cancelamento, backoff medido e testes de reboot. Um futuro rádio adaptativo — BLE, Wi‑Fi local, HaLow, LoRa ou outro — deve chamar o mesmo planner; a escolha do meio não pode alterar a autoridade da ação.

Também é necessário criar fixtures reais de RF switch e BUSY, testar troca RX/TX sem antena ausente, medir energia e separar o perfil de bancada do perfil de produto. Nenhum resultado físico do Atlas foi transferido para o HERUS como se fosse medição própria.

## 9. Veredito

O Atlas_Node é útil como fonte de engenharia prática, especialmente pela ponte BLE↔rádio, pelos watchdogs, pela fila de comandos, pela UX de entrega e pela disciplina de RF switch. Não deve ser tratado como referência de soberania, privacidade ou validação científica: o uso de texto como protocolo, persistência de conteúdo bruto, chaves default, GPS/mesh amplos e controles operacionais expostos criam limites claros.

A contribuição correta do Atlas para o HERUS é **melhorar a robustez de transporte sem transferir a autoridade para o transporte**. O HERUS deve ficar mais confiável depois dessa análise, não mais permissivo.

## Referências

[1] [Atlas_Node — README original](https://github.com/DiyYari/Atlas_Node/blob/main/README.original.md)

[2] [Atlas_Node — Windows/core/ai.py](https://github.com/DiyYari/Atlas_Node/blob/main/Windows/core/ai.py)

[3] [Atlas_Node — Apple/BluetoothManager.swift](https://github.com/DiyYari/Atlas_Node/blob/main/Apple/ONYX%20MECH/BluetoothManager.swift)

[4] [Atlas_Node — ESP32S3/src/main.cpp](https://github.com/DiyYari/Atlas_Node/blob/main/ESP32S3/src/main.cpp)

[5] [Atlas_Node — GITHUB_SOURCE_NOTES.md](https://github.com/DiyYari/Atlas_Node/blob/main/GITHUB_SOURCE_NOTES.md)

[6] [Atlas_Node — Windows/core/encryption.py](https://github.com/DiyYari/Atlas_Node/blob/main/Windows/core/encryption.py)
