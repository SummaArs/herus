# 12 — Enlace de controle Core↔Núcleo

**Avanço 6 de 10 · HERUS-A6-001 · envelope local autenticado, associado à sessão e anti-replay**

> O BLE seguro protege uma ligação. O HERUS ainda precisa proteger uma **decisão**: qual Núcleo pareado falou, para qual pressão de botão, em qual ordem, dentro de qual prazo, e com que autoridade limitada.

Este contrato protege o plano de companheiro entre o Core vestível e o Núcleo. Ele não altera o relay LoRa, que permanece cego a significados. O envelope só transporta uma observação de comando local do Núcleo para o gateway do Core; não transporta áudio, transcrição, contexto, chave de malha, payload HCP, localização ou autorização para enviar.

## 1. Ameaça e premissas

| Ameaça | Defesa do envelope | Limite honesto |
|---|---|---|
| Captura e replay de uma resposta anterior | Sequência monotônica por vínculo, `pair_id`, tag AEAD e estado RX persistível | Reset de sequência requer novo vínculo/chave; persistência física é trabalho de alvo |
| Resultado atrasado da sessão anterior | `session_id` da pressão de botão autenticado e gateway que ignora sessão obsoleta | O Core ainda precisa ter relógio/contador de sessão confiável |
| Alteração de fonte, confiança ou comando | Cabeçalho e payload como dados autenticados; falha AEAD zera plaintext | Não substitui validação de adaptador ASR real |
| Núcleo perdido/comprometido sugere ação | Fonte restrita a Núcleo, expiração, gateway de confiança e confirmação física | Comprometimento da chave pareada requer revogação e novo pareamento |
| Pareamento de atacante próximo | LE Secure Connections com comparação numérica ou OOB, mais vínculo explícito do HERUS | Interface física de pareamento e secure element ainda dependem de hardware |
| Injeção de frame fora de ordem | Janela estrita: aceita apenas `seq > last_seq`, após tag válida | O desenho inicial favorece segurança sobre tolerância a reordenação; BLE confiável torna isso aceitável |

O Bluetooth LE Secure Connections usa ECDH e disponibiliza associação por comparação numérica; a pessoa compara os seis dígitos e aborta se divergirem. [1] Essa associação é condição de provisionamento do `pair_key`; `Just Works` não é aceito como fundamento suficiente do vínculo de companheiro HERUS. A Bluetooth SIG também descreve nonces aleatórios durante autenticação para proteção contra replay. [1] O NIST define nonce como valor não repetitivo incluído em troca de protocolo para detectar e proteger contra replay. [2]

## 2. Funções de chave e vínculo

O pareamento físico gera ou libera um segredo de vínculo de 32 bytes `pair_key`, que a implementação de produto deverá guardar em secure element/keystore, não em log ou telemetria. O envelope usa a AEAD ChaCha20-Poly1305 de tag completa, já verificada independentemente no repositório, e uma chave distinta por papel através de HKDF:

```text
control_key = HKDF(pair_key, salt = pair_id, info = "HERUS/CORE-NUCLEUS/CONTROL/v1")
```

`pair_id` é um identificador de 32 bits provisionado junto do vínculo; não é endereço BLE, chave de malha, identidade transmitida via LoRa nem identificador de equipe. Os alvos reais devem derivar/obter `pair_key` de um mecanismo protegido; o código host recebe a chave apenas para que o protocolo e suas provas sejam reproduzíveis.

## 3. Envelope fixo e nonce

O envelope contém um cabeçalho visível, mas autenticado, e cinco bytes de payload cifrado. Ele tem tamanho fixo de 43 bytes, inclusive tag AEAD de 16 bytes.

| Campo | Tamanho | Proteção | Regra |
|---|---:|---|---|
| `version` | 1 B | AAD autenticado | Deve ser 1 |
| `direction` | 1 B | AAD autenticado e nonce | Nesta entrega, somente Núcleo→Core |
| `pair_id` | 4 B | AAD autenticado | Deve coincidir com vínculo local |
| `seq` | 8 B | AAD autenticado e nonce | Estritamente crescente, nunca zero |
| `session_id` | 4 B | AAD autenticado | Deve corresponder à sessão PTT ativa no gateway |
| `expires_ms` | 4 B | AAD autenticado | Deve estar no futuro e no máximo 8 s adiante no relógio monotônico do Core |
| `source`, `command`, `minutes`, `confidence`, `runner_up` | 5 B | Cifrado e autenticado | Sem áudio/texto; validação adicional no gateway |
| `tag` | 16 B | AEAD | Deve verificar antes de expor qualquer campo cifrado |

O nonce de 12 bytes é `pair_id || direction || seq[56]`. A sequência é limitada a 56 bits e uma nova chave é obrigatória antes de esgotá-la. Não há reutilização de nonce para uma mesma chave/direção. Cabeçalho como AAD impede que atacante altere par, direção, sessão, sequência ou prazo sem invalidar a tag. O transmissor só pode selar prazo positivo de até **8 s**; isso limita a janela de controle à interação local em curso, em vez de criar uma credencial de longa duração.

## 4. Máquina de aceitação

1. O Core inicia `push-to-talk` e cria sessão não nula.
2. O Núcleo só produz uma observação local para essa sessão em seu adaptador ASR.
3. O transmissor sela a observação com próxima sequência, prazo curto e direção Núcleo→Core.
4. O Core verifica versão, tamanho, direção, `pair_id`, sequência e tag. Texto cifrado só é liberado depois da tag.
5. Uma mensagem autenticada vencida é descartada e sua sequência é consumida; replay posterior não tem segunda chance.
6. Uma observação autenticada é entregue ao gateway do Avanço 5.
7. O gateway ainda aplica confiança/margem/contexto; rascunho ainda requer confirmação física; handoff ainda é único.

A sequência estrita faz uma escolha explícita: o plano de controle prioriza não aceitar replay nem reordenação. BLE conectado é ordenado; caso uma implementação futura escolha transporte que pode reordenar, ela deverá introduzir janela de replay testada em nova revisão, não relaxar silenciosamente `last_seq`.

## 5. Critérios de prova

A suíte de controle prova que selo/abertura preservam a observação, que alteração de um único bit não expõe payload, que par/direção errados falham, que replay e sequência fora de ordem falham, que expiração consome a sequência e que uma mensagem autenticada ainda não pula o gateway ou confirmação física. Essa prova **não** prova segurança de rádio BLE, secure element, pareamento físico, relógio de hardware ou armazenamento persistente; ela torna seus contratos de software falsificáveis.

## Referências

[1] [Bluetooth SIG — LE Secure Connections: Numeric Comparison](https://www.bluetooth.com/blog/bluetooth-pairing-part-4/)
[2] [NIST CSRC — Nonce glossary](https://csrc.nist.gov/glossary/term/nonce)
[3] [HERUS — protocolo e primitivas criptográficas](02-PROTOCOL.md)
[4] [HERUS — gateway semântico de confiança](11-GATEWAY-CONFIANCA.md)
