# HERUS — Ciclo de Diálogo Multietapa e Continuidade Local

**Estado:** concluído em host; áudio, atuador e interação humana ainda não validados.

**Ramo:** `feat/herus-semantic-compiler`

**Autoria:** Gustavo — `4llbluu3@gmail.com`

**Escopo:** transformar um candidato generativo isolado em um ciclo local com sessão física, apresentação, confirmação, interrupção, timeout e limpeza.

## Resumo

O HERUS agora possui uma máquina de estados bounded em [`firmware/core/generative_dialogue_loop.c`][1]. Ela recebe um resultado já produzido pelo núcleo generativo, o apresenta por meio do bridge HAP-SEM e controla o ciclo de vida do candidato. O estado transitório permanece no vestível e é apagado em interrupção, timeout, negação ou esquecimento.

A suíte multietapa passou **22/22 casos** e o redteam matou **5/5 mutantes críticos**. O resultado global preservou as **276 invariantes de simulação**. Esta etapa prova continuidade operacional no host; não prova que o HERUS já escuta voz, entende fala livre ou mantém uma conversa humana natural.

> **Alegação permitida:** um candidato local pode atravessar um ciclo bounded de geração e apresentação, permanecer pending quando exige confirmação, expirar de forma determinística e não deixar um plano ou sessão armado após interrupção.

> **Alegação proibida:** o HERUS já possui consciência, autonomia geral, memória perfeita ou equivalência com uma LLM aberta.

## Máquina de estados

| Estado | Entrada principal | Saída permitida | Regra de segurança |
|---|---|---|---|
| `IDLE` | sessão física não nula | `GENERATING` | sessão deve vir de gesto validado externamente |
| `GENERATING` | candidato bounded válido | `PRESENTED` ou `CONFIRMATION_PENDING` | candidato inválido aborta e é apagado |
| `PRESENTED` | leitura, interrupção ou timeout | `ABORTED`, `TIMED_OUT` ou limpeza | resposta não vira ação |
| `CONFIRMATION_PENDING` | confirmação ou negação da mesma sessão | `CONFIRMED` ou `ABORTED` | outra sessão é rejeitada |
| `CONFIRMED` | esquecimento | `CLEARED` | confirmação é one-shot |
| `ABORTED` | novo início | `GENERATING` | payload anterior já foi limpo |
| `TIMED_OUT` | novo início | `GENERATING` | sessão e candidato são zerados |
| `CLEARED` | novo início | `GENERATING` | fronteira de privacidade explícita |

A máquina não chama inferência, rádio, armazenamento, HCP ou atuador físico. Ela recebe um `gc_result_t` e produz um `gh_signal_t`; a etapa de encode/decode HAP-SEM continua separada. Essa decomposição permite testar a continuidade sem confundir o estado interno com a realização física do gesto ou da vibração.

## Confirmação e não-autonomia

Um resultado do tipo plano que exige confirmação entra em `GDL_CONFIRMATION_PENDING`. A confirmação precisa carregar exatamente o `physical_session_id` que iniciou o turno. Uma sessão diferente retorna `GDL_E_PHYSICAL`. Depois da confirmação válida, o identificador é zerado e uma segunda confirmação retorna erro de estado.

O loop não possui uma operação de execução. `gdl_confirm` apenas registra a transição de apresentação pendente para apresentação confirmada; qualquer ação real deverá passar por outra camada de autoridade, confirmação e transmissão, que não faz parte deste módulo.

## Timeout, interrupção e esquecimento

O timeout de geração é contado a partir de `started_ms`; o timeout de confirmação é contado a partir de `presented_ms`. Esses relógios são independentes para que uma inferência longa não transforme automaticamente uma confirmação em válida ou inválida sem uma regra explícita.

`gdl_abort`, `gdl_tick` quando expira e `gdl_deny` chamam a limpeza bounded. `gdl_forget` também zera candidato, sinal háptico, sessão e timestamps, e passa o estado para `GDL_CLEARED`. O módulo só mantém contadores numéricos de métricas; não registra texto, áudio, embeddings, identidade, localização, chaves ou conteúdo de resposta fora do payload transitório.

## Evidência adversarial

O teste em [`firmware/core/test_generative_dialogue_loop.c`][2] cobre sessão física, apresentação direta, round-trip HAP-SEM, plano pendente, sessão incorreta, confirmação válida, replay, esquecimento, timeout de geração, timeout de confirmação, negação, abstenção, saída inválida e reinício após limpeza.

O redteam em [`tools/test_generative_dialogue_loop_redteam.py`][3] removeu cinco controles individualmente:

| Mutante | Controle removido | Resultado |
|---|---|---:|
| `session-bypass` | correspondência da sessão física | Morto |
| `confirmation-replay` | transição one-shot | Morto |
| `timeout-retain-payload` | limpeza na expiração | Morto |
| `abort-retain-payload` | limpeza na interrupção | Morto |
| `forget-retain-candidate` | apagamento de privacidade | Morto |

O resultado reproduzível foi:

```text
GEN DIALOGUE LOOP: 22 pass, 0 fail
GENERATIVE DIALOGUE LOOP REDTEAM: 5/5 critical mutants killed
```

## Limitações e próxima fronteira

O loop recebe candidatos tipados; ele ainda não resolve a compreensão de linguagem aberta. A entrada de voz, ASR local, detecção de interlocutor, ruído, latência e energia permanecem fora desta prova. O HAP-SEM round-trip garante integridade semântica do frame no host, mas não garante distinguibilidade perceptual no pulso.

A próxima fronteira é integrar o loop a uma sequência real de intenção→memória→geração→apresentação, mantendo a regra de que todo dado textual de entrada é transitório e que a aprendizagem pessoal só pode alterar apresentação sob consentimento. Antes do hardware, também será necessário um benchmark de diálogo composto com múltiplos turnos e perturbações de relógio, reboot e perda de adaptador.

## Referências

[1]: https://github.com/SummaArs/herus/blob/feat/herus-semantic-compiler/firmware/core/generative_dialogue_loop.c "Implementação C11 do ciclo de diálogo multietapa"
[2]: https://github.com/SummaArs/herus/blob/feat/herus-semantic-compiler/firmware/core/test_generative_dialogue_loop.c "Teste do ciclo de diálogo multietapa"
[3]: https://github.com/SummaArs/herus/blob/feat/herus-semantic-compiler/tools/test_generative_dialogue_loop_redteam.py "Redteam do ciclo de diálogo multietapa"
