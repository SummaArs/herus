# Recuperação durável de reserva de sessão

**Status:** contrato C11 portátil, verificado em host. Ele classifica evidência declarada por um adaptador futuro; **não reativa uma sessão ativa após reinicialização** e não constitui prova de evento físico, pessoa ou plataforma segura.

## Pergunta tratada

O gate do Passo 7 limita uma sessão de coleção por propósito, ID, prazo, nonce e orçamento de usos durante uma vida de RAM. Seu piso de ID, porém, desaparece em uma reinicialização. O Passo 8 introduz `memory_physical_session_recovery.[ch]`: um oráculo puro que permite a um adaptador futuro classificar o estado de uma **reserva de ID** autenticada contra um piso durável que o próprio adaptador declara ter recuperado.

> O resultado da recuperação é apenas uma decisão sobre marcadores de replay. Ele nunca contém uma sessão ativa, nonce, prazo, callback, token, autorização de coleção, evento humano ou identidade. Toda operação posterior continua exigindo uma **nova** confirmação física e uma nova sessão do gate do Passo 7. O Passo 9 torna essa continuidade executável ao reconstruir o gate em `IDLE` e importar somente o piso em [Quarentena de boot da sessão](33-QUARENTENA-BOOT-SESSAO.md).

A resistência a replay é uma referência de desenho legítima: a NIST requer mecanismos resistentes a replay em seus contextos de autenticação remota de maior garantia [1]. Essa publicação também delimita seu próprio escopo: autenticação digital em rede, não autenticação de pessoa para acesso físico. Por isso o HERUS não a invoca para alegar AAL, intenção humana, biometria, presença ou conformidade. A NIST também enquadra resiliência de firmware e dados em proteção contra alteração não autorizada, detecção e recuperação segura [2]. Aqui a consequência deliberadamente modesta é falhar fechada quando a topologia não leva a um estado coerente; não é uma alegação de resiliência da plataforma.

## Contrato e superfície mínima

A entrada é `memory_physical_session_recovery_snapshot_t`. O contrato recebe somente presença e autenticação canônicas de dois registros, `COMMITTED` e `PREPARED`, seus IDs de reserva, propósito fechado, orçamento de uso, uma base para `PREPARED`, o indicador de igualdade autenticada entre registros de mesmo ID e o `durable_reservation_floor` declarado pelo adaptador. A API é livre de I/O, alocação, armazenamento, relógio, cartão, rede, callback, chave, pessoa, biometria, texto, áudio, transcrição, embedding, localização e saída de modelo.

| Elemento | Regra host verificável | Consequência de falha |
|---|---|---|
| Booleanos | Toda presença, autenticação e igualdade deve ser exatamente `0` ou `1`. | `BLOCKED` e `E_INVALID`. |
| Registro ausente | Não pode carregar nenhum campo material diferente de zero. | `BLOCKED` e `E_INVALID`. |
| Registro presente | Precisa ser autenticado, ter ID não nulo, propósito fechado e orçamento canônico. | `BLOCKED` e `E_INVALID`. |
| Propósito e usos | `QUERY` permite 1–8 usos; `INSERT`, `OPEN`, `REMOVE` e `COMPACT` permitem exatamente 1. | `BLOCKED` e `E_INVALID`. |
| Relação de IDs | `PREPARED` só pode ser o sucessor imediato de sua base; `COMMITTED` precisa coincidir com o piso quando está sozinho. | `BLOCKED` e `E_INVALID`. |
| Saída | Em sucesso, emite só uma ação de recuperação; não emite capacidade ativa. | Nenhuma autorização nasce do oráculo. |

O campo `prepared_matches_committed` só é aceito como `1` quando os campos materiais de `PREPARED` e `COMMITTED` são idênticos. Um ID igual, por si só, é insuficiente. Assim, uma limpeza interrompida pode ser terminada, mas uma contradição não é reinterpretada como sucesso.

## Matriz determinística de recuperação

| Topologia autenticada e canônica | Condição de piso declarada | Ação | Interpretação estrita |
|---|---|---|---|
| Nenhum registro | Piso `0` | `EMPTY` | Não existe reserva a recuperar. |
| Apenas `COMMITTED` | ID do committed = piso | `USE_COMMITTED` | O adaptador pode usar o ID apenas como piso anti-replay. |
| Apenas `PREPARED`, primeira reserva | Base `0`, ID `1`, piso `1` | `PROMOTE_PREPARED` | O marcador já queimado pode tornar-se piso; nenhuma sessão é restaurada. |
| Apenas `PREPARED`, sucessor | Base = piso anterior, ID = base + 1, piso = ID | `PROMOTE_PREPARED` | O piso avançou antes da limpeza; promove-se somente o marcador consumido. |
| Apenas `PREPARED` | Piso ainda é a base | `DISCARD_PREPARED` | A preparação antecede a confirmação durável do piso e é descartada. |
| `COMMITTED` e `PREPARED` idênticos | IDs iguais ao piso e igualdade autenticada completa | `FINALIZE_PREPARED` | Apenas a limpeza de uma escrita já confirmada é concluída. |
| Qualquer ausência de autenticação, salto, piso divergente, ID igual sem igualdade completa ou formato inválido | Qualquer | `BLOCKED` | O adaptador deve permanecer sem decisão permissiva e solicitar nova confirmação em caminho posterior. |

`PROMOTE_PREPARED` não “promove uma autorização”. Ele instrui somente a consolidação de um ID que já deve ser considerado queimado, reduzindo a chance de esse mesmo ID voltar a ser aceito como novo após reboot. A ação não chama `memory_physical_session_begin()`, não fabrica nonce, não repõe usos e não altera o estado transitório do gate. A ponte seguinte de bootstrap só pode importar esse piso em gate `IDLE`; ela não transforma a ação em capacidade.

## Contraprovas host reproduzíveis

A suíte T15 é compilada em C11 estrito com `make memory-physical-session-recovery` e é executada pelo ledger global. Ela percorre estado vazio, committed coerente, preparação descartada, promoção ancorada no piso, limpeza idempotente e bloqueio por contradição.

| Contraprova | Veredito exigido |
|---|---|
| Primeira reserva preparada antes de o piso avançar | `DISCARD_PREPARED`; não há reativação. |
| Sucessor preparado e piso já avançado | `PROMOTE_PREPARED` somente como ID queimado. |
| `PREPARED` e `COMMITTED` com mesmo ID, mas sem igualdade autenticada completa | `BLOCKED`. |
| Primeiro ID pulado, sucessor não imediato ou piso incompatível | `BLOCKED`. |
| Registro não autenticado, propósito `NONE`, uso `QUERY` zero, uso excessivo ou mutação com mais de um uso | `BLOCKED`. |
| Booleano não canônico ou campo material em registro ausente | `BLOCKED`. |
| Entrada ou saída nula | `E_ARG`; nenhuma ação permissiva é emitida. |

```sh
./prove.sh --quiet
```

Após o Passo 9, o ledger esperado é de **34 suítes**, **77 invariantes de prova** e **74 invariantes do simulador**. Os dois novos checks verificam que só um sucessor autenticado e imediatamente ancorado no piso pode ser promovido e que toda contradição bloqueia sem reviver autoridade. Esses resultados são de host e simulação; não são métricas de botão, energia, latência, qualidade de fala, LLM ou bancada.

## Adaptador futuro, plataforma aberta e fronteira de confiança

O contrato não seleciona ESP32-S3, NVS, eFuse, TPM, secure element, FRAM, flash, telefone, relógio externo, botão, touch, IMU ou biometria. Um adaptador pode usar qualquer combinação de armazenamento protegido, contador monotônico, log transacional, raiz discreta ou sistema acompanhante, desde que evidencie para o alvo escolhido a autenticação dos registros, a persistência do piso, a atomicidade ou procedimento seguro de recuperação e o comportamento perante corte de energia.

| Evidência que o adaptador ainda deve demonstrar | Por que o C11 não a entrega |
|---|---|
| Persistência e ordenação reais de `PREPARED`, piso e `COMMITTED` | O oráculo recebe uma fotografia já declarada; ele não escreve nem lê mídia. |
| Autenticidade material dos marcadores | A flag `*_authenticated` é entrada de adaptador, não verificação criptográfica interna. |
| Resistência a replay após reboot | Depende de o piso durável sobreviver e não poder ser revertido pelo atacante relevante. |
| Novo evento físico depois de reboot | O módulo não observa hardware, pessoa, gesto, botão ou contexto. |
| Boot confiável, isolamento de RAM e resistência a adulteração | Exigem propriedades e ensaios da plataforma alvo. |
| Brownout, reset, concorrência e desgaste de mídia | Exigem adaptador concreto, instrumentação e campanha de falhas. |

A SP 800-193 é útil como orientação de que recuperação segura não pode ignorar integridade de plataforma [2]. Ela não permite que o HERUS substitua validação de firmware, cadeia de boot ou testes de recuperação por uma enumeração C. Toda alegação dessa categoria permanece `PENDING_TARGET` no modelo de ameaças.

## Limites honestos

Este passo **não prova** evento, gesto, pessoa, intenção psicológica, biometria, liveness, posse exclusiva, identidade, nonce imprevisível, relógio real, prazo real, reboot físico, persistência real, corte de energia, secure boot, flash criptografada, RAM protegida, secure element, attestation, anti-rollback físico, resistência a side-channel, resistência real a replay pós-reboot, rádio, rede, ASR, LLM, precisão, WER, energia, latência ou privacidade de padrão de acesso. Em particular, um registro declarado autenticado não é uma assinatura verificada pelo oráculo; uma sessão posterior continua inválida até que um adaptador execute a confirmação física e inicie uma sessão nova pelo contrato anterior.

## Referências

[1] [NIST SP 800-63B-4 — *Digital Identity Guidelines: Authentication and Authenticator Management*](https://pages.nist.gov/800-63-4/sp800-63b.html). A página oficial registra o escopo de autenticação digital em rede e requisitos de resistência a replay para seus níveis de garantia.

[2] [NIST SP 800-193 — *Platform Firmware Resiliency Guidelines*](https://csrc.nist.gov/pubs/sp/800/193/final). Publicação da NIST sobre proteção, detecção e recuperação segura de firmware e dados de plataforma, DOI: [10.6028/NIST.SP.800-193](https://doi.org/10.6028/NIST.SP.800-193).
