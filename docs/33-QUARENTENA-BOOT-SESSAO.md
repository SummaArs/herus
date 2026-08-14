# Quarentena de boot da sessão física

**Status:** composição C11 portátil, verificada em host. O módulo inicializa uma nova estrutura em RAM a partir de um snapshot de reserva já classificado; ele importa somente um piso de ID e **nunca restaura sessão ativa**.

## Pergunta tratada

O Passo 8 estabeleceu que `memory_physical_session_recovery.[ch]` classifica registros `PREPARED` e `COMMITTED` autenticados contra um piso durável declarado. Ainda restava um risco de costura: uma porta futura poderia executar o oráculo corretamente e, por erro, também desserializar de memória persistente um nonce, propósito, prazo, orçamento de usos ou estado `ACTIVE` antigo. Esse caminho não pertence ao oráculo de reserva, mas poderia transformar um marcador recuperado em autorização indevida.

O Passo 9 adiciona `memory_physical_session_bootstrap.[ch]`. A função compõe o oráculo de reserva com `memory_physical_session_init()`, recria o gate transitório em RAM limpa e importa **exclusivamente** o `session_floor` coerente. O resultado é deliberadamente menor que uma sessão: nenhum campo de resultado ou de gate representa pessoa, evento, nonce, prazo, uso restante, propósito, cartão ou autorização ativa.

> Depois do bootstrap bem-sucedido, o gate está em `IDLE`. Um uso posterior só pode começar com `memory_physical_session_begin()` usando ID estritamente maior que o piso recuperado e uma **nova** afirmação canônica de evento físico vinda de adaptador futuro.

A NIST SP 800-193 descreve resiliência de firmware e dados como proteção contra alteração não autorizada, detecção e recuperação rápida e segura [1]. A aplicação modesta desse princípio aqui é iniciar de uma topologia coerente ou bloquear; ela não cria nenhuma alegação de boot seguro, integridade de firmware ou mídia. A NIST SP 800-63B-4 também trata gerenciamento, expiração e reautenticação de sessões em autenticação digital remota [2]. O HERUS usa isso como referência de desenho para não carregar autoridade automaticamente, mas a publicação explicitamente não autentica pessoa para acesso físico; portanto não há AAL, conformidade, gesto, biometria ou intenção humana provados.

## Contrato implementado

A função pública recebe quatro referências: gate de sessão a reconstruir, configuração local, snapshot de reserva e resultado. Ela não recebe I/O, dispositivo, armazenamento, chave, assinatura, evento, pessoa, nonce, relógio, cartão, consulta, texto, áudio, transcrição, embedding, localização, modelo, callback, rádio ou rede.

| Elemento | Regra host verificável | Resultado seguro |
|---|---|---|
| Configuração | É revalidada por `memory_physical_session_init()`. | Configuração inválida zera e bloqueia o gate. |
| Snapshot | É avaliado pelo oráculo do Passo 8 antes de importar qualquer piso. | Snapshot contraditório bloqueia o gate. |
| Ação `EMPTY` | Importa piso `0`. | Gate `IDLE`, sem sessão. |
| Ação `USE_COMMITTED` ou `FINALIZE_PREPARED` | Importa o ID committed autenticado. | O mesmo ID não pode iniciar sessão nova. |
| Ação `PROMOTE_PREPARED` | Importa o ID prepared já queimado. | A promoção é apenas piso; propósito e usos não são transportados. |
| Ação `DISCARD_PREPARED` | Retém o piso durável declarado anterior. | A preparação descartada não cria estado ativo. |
| `BLOCKED`, entrada ausente ou ação impossível | Zera resultado e gate, depois marca `BLOCKED`. | Nenhuma decisão permissiva sobrevive. |

Em êxito, `memory_physical_session_bootstrap_result_t` expõe somente `recovered_session_floor`, a ação de recuperação e `active_evidence_scrubbed == 1`. O gate precisa estar em `IDLE`; `active_session_id`, `active_event_nonce`, `active_purpose`, tempos, usos e métricas são zero. Essa é uma propriedade de composição sobre `struct` em host, não uma operação de sanitização de hardware.

## Matriz de bootstrap

| Ação que o Passo 8 já classificou | Piso importado | Estado do gate ao retornar | Sessão pré-reboot |
|---|---:|---|---|
| `EMPTY` | `0` | `IDLE` com campos ativos zerados | Não existe. |
| `USE_COMMITTED` | ID committed | `IDLE` com campos ativos zerados | Não é restaurada; o ID no piso é recusado em `begin()`. |
| `PROMOTE_PREPARED` | ID prepared | `IDLE` com campos ativos zerados | Não é restaurada; o ID promovido é recusado em `begin()`. |
| `DISCARD_PREPARED` | Piso durável anterior | `IDLE` com campos ativos zerados | Não é restaurada; uma tentativa futura ainda exige evento novo. |
| `FINALIZE_PREPARED` | ID committed | `IDLE` com campos ativos zerados | Não é restaurada; finalizar limpeza não cria capacidade. |
| `BLOCKED`, erro de snapshot, configuração inválida ou argumento ausente | Nenhum | `BLOCKED` e campos ativos zerados quando há gate | Não pode ser validada, consumida ou iniciada. |

A matriz não decide se um ID de `PREPARED` descartado deve ser considerado anti-replay durável em uma plataforma específica. A ação `DISCARD_PREPARED` mantém somente o piso que o oráculo recebeu; se uma nova sessão com ID acima desse piso for aceita, isso ainda depende da origem de evento, nonce e protocolo de persistência demonstrados pelo adaptador. O bootstrap apenas impede que a estrutura transitória antiga seja usada como atalho.

## Contraprovas reproduzíveis

T16 constrói gates antes do bootstrap para simular a presença, em RAM de teste, de ID, nonce, propósito, prazo e usos antigos. O bootstrap deve apagar todo esse conjunto antes de importar o piso. A suíte também tenta validar/consumir os IDs antigos, reutilizar o ID do piso, iniciar sucessor sem confirmação e atravessar falha de configuração ou recuperação.

| Contraprova | Veredito exigido |
|---|---|
| Gate anteriormente `ACTIVE` seguido de `USE_COMMITTED` | Gate vira `IDLE`, campos ativos e métricas são zero; o ID antigo e o ID no piso não concedem uso. |
| `PROMOTE_PREPARED` | O ID promovido torna-se piso, mas propósito/uso prepared não chegam ao novo gate. |
| `DISCARD_PREPARED` | Nenhuma sessão é criada; só uma nova afirmação canônica pode começar ID permitido pelo piso retido. |
| `FINALIZE_PREPARED` | Limpeza idempotente termina em `IDLE`, não em sessão correspondente ativa. |
| Snapshot sem autenticação ou contraditório | Resultado `BLOCKED`; gate antigo é apagado e bloqueado. |
| Configuração inválida, gate/snapshot/resultado ausente | Erro fail-closed; nenhum resultado ativo é emitido. |
| Remoção da evidência de quarentena em TM-04 | `MITIGATED_HOST` deixa de ser emitido. |

```sh
cd firmware
make memory-physical-session-bootstrap
cd ..
./prove.sh --quiet
```

Depois deste passo, o ledger esperado é de **34 suítes**, **77 invariantes de prova** e **74 invariantes de sistema simulado**. Eles testam contratos C11, não reboot físico, duração de escrita, energia, latência, qualidade de hardware, fala, LLM ou comportamento de pessoas.

## Porta de adaptador e evidência futura

O módulo não escolhe ESP32-S3, NVS, eFuse, secure element, TPM, FRAM, flash, dispositivo acompanhante, botão, touch, IMU, biometria, RTC ou contador de fornecedor. Um adaptador futuro pode usar qualquer arquitetura que demonstre o contrato do alvo e a matriz de interrupção correspondente.

| Evidência pendente no alvo | Por que este passo não a prova |
|---|---|
| Autenticidade do snapshot e do piso | `*_authenticated` é uma declaração validada pelo oráculo, não assinatura verificada pelo bootstrap. |
| Ordem e durabilidade de `PREPARED`/piso/`COMMITTED` | O C11 não lê ou escreve mídia. |
| Reset, brownout e concorrência reais | A troca de `struct` em host não mede corte de energia ou scheduler. |
| Novo evento depois do boot | O gate recebe uma flag futura; não observa botão, gesto, pessoa ou contexto. |
| Nonce e relógio confiáveis | Não há RNG, timer ou política de reset no contrato. |
| Cadeia de boot, isolamento de RAM e resistência a adulteração | Essas propriedades requerem plataforma selecionada e ensaios no alvo. |
| Resistência real a replay pós-reboot | Requer piso durável protegido, origem de evento e campanha adversarial do adaptador. |

## Limites honestos

Este passo não prova pessoa, evento, botão, gesto, intenção psicológica, consentimento, biometria, liveness, identidade, posse exclusiva, nonce imprevisível, CSPRNG, relógio real, prazo real, reboot físico, reset, brownout, persistência, atomicidade, autenticação criptográfica material dos marcadores, secure boot, flash cifrada, RAM protegida, secure element, contador monotônico físico, anti-rollback físico, resistência a side-channel, resistência real a replay pós-reboot, rede, rádio, ASR, LLM, WER, precisão, energia, latência, acessibilidade ou privacidade de padrão de acesso. O que existe é uma ponte host que torna explícito o único dado que pode atravessar para o gate em RAM: um piso de reserva coerente.

## Referências

[1] [NIST SP 800-193 — *Platform Firmware Resiliency Guidelines*](https://csrc.nist.gov/pubs/sp/800/193/final), DOI: [10.6028/NIST.SP.800-193](https://doi.org/10.6028/NIST.SP.800-193).

[2] [NIST SP 800-63B-4 — *Digital Identity Guidelines: Authentication and Authenticator Management*](https://pages.nist.gov/800-63-4/sp800-63b.html).
