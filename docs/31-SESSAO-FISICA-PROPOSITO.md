# 31 — Sessão física vinculada a propósito

**Status:** contrato C11 portátil, verificado em host; não é evidência de gesto, pessoa, biometria, raiz protegida ou hardware físico.

## Pergunta tratada

Os passos anteriores exigiam confirmação física para operações de memória, mas uma estrutura mínima `{physical_session_id, physical_confirmed}` não conseguia distinguir uma autorização nova de uma repetida, não vinculava a autorização à operação pretendida e não delimitava sua validade no tempo. O Passo 7 introduz `memory_physical_session.[ch]` para que as operações da **coleção multi-cartão** dependam de uma sessão transitória de finalidade fechada, limitada e consumível.

> Uma sessão válida neste passo significa apenas que o adaptador futuro apresentou campos canônicos para o gate C11 e que o gate aceitou sua topologia em RAM. Ela **não significa** que o HERUS verificou a identidade, a presença, a intenção psicológica, a biometria ou a vivacidade de uma pessoa.

O desenho usa como referências conceituais a resistência a replay, a intenção de autenticação e a gestão de sessão discutidas pelo NIST, sem importar os níveis de garantia digital para o HERUS. A própria SP 800-63B-4 afirma que seu escopo é autenticação digital em rede e não autenticação de pessoa para acesso físico; por isso ela não é uma certificação nem uma classificação AAL do projeto [1]. O WebAuthn é uma segunda referência de separação de papéis: desafio com escopo, consentimento mediado e operação limitada ao autenticador, mas usa credenciais de chave pública, origem web e attestation que **não** foram implementadas aqui [2].

## Contrato implementado

A tabela apresenta a superfície deliberadamente mínima. Nenhuma função recebe cartão, consulta, chave, texto, áudio, transcrição, embedding, identidade, localização, saída de modelo, callback, rádio ou rede.

| Elemento | Regra host verificável | Dados que não recebe nem registra |
|---|---|---|
| `memory_physical_session_begin()` | Aceita somente propósito fechado, ID estritamente acima do piso RAM, nonce não nulo, confirmação exatamente `1`, janela sem overflow e orçamento canônico. | Evento bruto, gesto, biometria, identidade, cartão, consulta e conteúdo. |
| `memory_physical_session_validate()` | Confere estado ativo, propósito, ID e tempo sem gastar um uso; sessão expirada é limpa. | Não emite, renova ou prova autorização real. |
| `memory_physical_session_consume()` | Gasta exatamente um uso; o último uso move para `CONSUMED` e zera ID, nonce, propósito e tempos ativos. | Não persiste nonce, ID, histórico de evento ou telemetria de produto. |
| `memory_physical_session_cancel()` | Revoga apenas uma sessão ativa e limpa sua evidência transitória. | Não cria fallback, retry automático ou sessão substituta. |
| Métricas | Mantêm apenas contadores agregados de início, consumo, cancelamento, expiração e recusas. | Não mantêm sessão, nonce, propósito, pessoa, cartão ou resultado de consulta. |

Os propósitos são fechados: `COLLECTION_INSERT`, `COLLECTION_OPEN`, `COLLECTION_REMOVE`, `COLLECTION_COMPACT` e `COLLECTION_QUERY`. Apenas `QUERY` pode usar mais de uma operação, entre 1 e 8; o padrão é 3. Mutação e abertura são estritamente de uso único. O piso de ID é propositalmente **somente RAM**: ele recusa repetição dentro da vida do processo, mas reinicialização o apaga. Portanto não há alegação de anti-replay entre boots.

## Integração com a memória complementar

A coleção agora recebe `memory_collection_access_t`, formado apenas por referência ao gate, ID opaco de sessão e tempo monotônico observado. Antes de tocar o armazenamento, as operações `insert`, `open`, `remove` e `compact` consomem, respectivamente, os propósitos `INSERT`, `OPEN`, `REMOVE` e `COMPACT`.

A consulta privada tem duas barreiras deliberadamente distintas. O índice primeiro chama `validate(QUERY)` antes de incrementar seu próprio orçamento de sondagens; em seguida, a costura privada da coleção consome um uso `QUERY` somente quando copia os cartões autenticados para RAM. Assim, uma finalidade errada, uma sessão expirada, um ID divergente ou um replay não gastam orçamento do índice, não copiam cartões e não abrem cartão automaticamente. A decisão de matching continua retornando somente `MATCH`, `NO_MATCH` ou `AMBIGUOUS`; a coleção não enumera e o índice não cria uma rota de abertura.

O Grand Finale da coleção exige agora `collection_physical_session_bound == 1`. A ausência dessa evidência bloqueia a composição e TM-04 perde `MITIGATED_HOST` se `memory_physical_session_bound` deixar de ser canônico. Essa ligação é um requisito de prova; o auditor M14 permanece puramente diagnóstico e não cria autorização nova.

| Caminho | Regra após o Passo 7 | Resultado se a sessão falhar |
|---|---|---|
| Inserir cartão já autorizado | Consome `INSERT` depois de validar cartão e recibo humano. | `MEMORY_COLLECTION_E_ACCESS`; nenhum commit é iniciado. |
| Abrir ID opaco conhecido | Consome `OPEN` antes de carregar a coleção. | Saída continua zerada; nenhuma enumeração. |
| Remover ou compactar | Consome `REMOVE` ou `COMPACT`. | Não muda geração, cartão ou ordem. |
| Consulta tipada | Valida e depois consome `QUERY`; índice mantém orçamento próprio. | Sem cópia de cartões, sem orçamento gasto e sem resultado. |
| Apresentação one-shot | Continua sob o contrato anterior de apresentação. | Não herda uma sessão de mutação/consulta. |

## Contraprovas reproduzíveis

A suíte T14 contém a matriz direta do gate. T10 e T11 exercitam a integração real com coleção e índice; M14 exige a evidência na composição; T9 exige-a para a classificação host de TM-04.

| Contraprova | Veredito exigido |
|---|---|
| ID repetido na mesma vida RAM, nonce nulo, confirmação diferente de `1` ou propósito desconhecido | Recusa antes de iniciar sessão. |
| Finalidade diferente, ID errado, uso após consumo ou tentativa de iniciar outra sessão enquanto uma está ativa | Nenhuma autoridade é entregue. |
| Mais de um uso em mutação/abertura ou orçamento `QUERY` acima do máximo | Recusa canônica. |
| Tempo anterior ao início, expiração ou overflow no cálculo de validade | Recusa; expiração limpa a evidência ativa. |
| Cancelamento | Revoga e impede consumo posterior. |
| Consulta malformada, sessão `QUERY` inválida ou propósito não-`QUERY` | Índice recusa antes de avançar seu orçamento ou copiar cartões. |
| Remoção isolada da evidência de sessão | M14 bloqueia e TM-04 perde mitigação host. |

A prova global é executada com:

```sh
./prove.sh --quiet
```

Após este passo, o ledger esperado é de **32 suítes**, **73 invariantes de prova** e **74 invariantes do simulador**. Esses números descrevem testes host e simulação existentes; não medem confiabilidade de botão, WER, precisão de modelo, energia, latência, segurança de componente ou comportamento de bancada.

## Porta de plataforma aberta e gates pendentes

O módulo não escolhe ESP32-S3, NVS, eFuse, TPM, secure element, FRAM, flash, telefone, relógio externo, botão, touch, IMU ou biometria. Um adaptador de plataforma só poderá elevar a evidência depois de demonstrar, para o alvo escolhido, a origem do evento, nonce imprevisível, contador/piso durável ou estratégia equivalente contra reboot, relógio monotônico com política de reset, cancelamento em falha, limpeza de RAM, proteção da cadeia de boot e cortes controlados durante início, consumo e persistência.

A escolha pode ser um microcontrolador com periférico simples, uma raiz discreta, um ambiente seguro de sistema operacional, um dispositivo acompanhante ou outra plataforma futura. A comparação deve ser baseada em evidência de adaptador e bancada, não em marketing de fornecedor. O contrato portable serve justamente para impedir que uma migração de plataforma transforme uma flag local em aprovação tácita de autenticação humana.

## Limites honestos

Este passo não implementa ou demonstra presença física real, botão, gesto, PIN, biometria, identificação, liveness, posse exclusiva, credencial criptográfica, assinatura, attestation, anti-phishing, origem de evento, CSPRNG, relógio real, persistência de contador, resistência a reset, anti-replay entre reboots, secure boot, isolamento de RAM, secure element, resistência a firmware malicioso, resistência a side-channel, privacidade de padrão de acesso, PIR, ORAM, rede, rádio, LLM, ASR, escala, energia ou latência. A apresentação one-shot e os contratos unitários antigos continuam como fronteiras separadas; eles não são prova de sessão física alvo apenas porque a coleção agora possui um gate mais estrito.

## Referências

[1] [NIST SP 800-63B-4 — *Digital Identity Guidelines: Authentication and Authenticator Management*](https://pages.nist.gov/800-63-4/sp800-63b.html). Publicação final registrada em julho de 2025 em [CSRC](https://csrc.nist.gov/pubs/sp/800/63/b/4/final).

[2] [W3C Web Authentication: An API for accessing Public Key Credentials — Level 3](https://www.w3.org/TR/webauthn-3/), Candidate Recommendation Snapshot, 2026.
