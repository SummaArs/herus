# HERUS — Vivacidade Generativa e Canal Háptico

**Estado:** concluído em host; atuador físico ainda não validado.

**Ramo:** `feat/herus-semantic-compiler`

**Autoria:** Gustavo — `4llbluu3@gmail.com`

**Escopo:** fechar o primeiro ciclo de apresentação local entre geração híbrida e HAP-SEM.

## Resumo

O HERUS agora possui uma ponte explícita entre o resultado generativo e o canal háptico em [`firmware/core/generative_haptic_bridge.c`][1]. A ponte transforma candidatos locais em eventos numéricos HAP-SEM, que são codificados e decodificados pelo encoder/decoder existente. A camada não recebe texto, não executa comandos, não transmite, não persiste e não amplia autoridade.

O benchmark passou **8/8 casos** e o redteam matou **4/4 mutantes críticos**. A regressão global preservou as **276 invariantes de simulação** e terminou com `ALL INVARIANTS HOLD`. Esses resultados provam a vivacidade operacional do contrato no host: o sistema consegue representar estados de resposta, grounded memory, plano pendente, contrafactual, desconhecimento, conflito, bloqueio de política e esgotamento de orçamento em uma forma háptica round-trip. Eles não provam que uma pessoa perceberá a mesma distinção no pulso, que o DRV2605L reproduzirá um código distinguível ou que o ciclo terá a latência e a energia desejadas.

> **Definição operacional desta etapa:** o HERUS está “vivo” quando uma saída local percorre uma cadeia completa de estado, apresentação, validação e retorno sem desaparecer em uma string, sem virar ação silenciosa e sem perder a distinção entre sucesso, proposta, desconhecimento e bloqueio.

## Mapeamento semântico

O bridge recebe `gc_result_t` e produz `gh_signal_t`. A decisão é deliberadamente conservadora: todos os eventos têm `actionable = 0`. Um plano nunca se torna confirmado somente porque foi gerado; uma abstenção nunca se torna ACK; um bloqueio de política nunca se torna sucesso.

| Resultado generativo | Escopo HAP-SEM | Classe | Estado | Confirmação |
|---|---|---|---|---:|
| Resposta direta | `COM` | `ACK` | `CONFIRMED` | Não |
| Resposta derivada grounded | `MEM` | `ACK` | `CONFIRMED` | Não |
| Plano candidato | `PLAN` | `QUERY` | `PENDING` | Sim quando exigida |
| Contrafactual | `COM` | `QUERY` | `CONFIRMED` | Não |
| Sem evidência | `COM` | `ERROR` | `UNKNOWN` | Não |
| Conflito | `COM` | `ALERT` | `UNKNOWN` | Não |
| Política bloqueada | `SFTY` | `PRIVACY` | `DENIED` | Não |
| Orçamento esgotado | `COM` ou `MEM` | `ERROR` | `UNKNOWN` | Não |

A escolha de um evento háptico não é uma tradução de texto para Morse nem uma promessa de universalidade perceptual. O HAP-SEM preserva a camada semântica; o perfil do atuador fornece a associação local entre símbolo e efeito. Isso mantém separadas a intenção semântica e a particularidade física do ERM, LRA ou DRV2605L.

## Round-trip

Cada cenário do teste em [`firmware/core/test_generative_haptic_bridge.c`][2] percorre quatro passos: criar um resultado tipado; chamar `gh_from_result`; codificar o `hl_event_t` com um perfil HAP-SEM válido; e decodificar com o mesmo perfil. O teste compara os campos semânticos antes e depois.

Esse round-trip não mede percepção humana. Ele mede que a codificação não perde `scope`, `class`, `state`, `urgency`, versão e fragmentação dentro do contrato vigente. Uma falha de checksum, perfil, waveform ou formato deve impedir o sucesso, como já exigido pelos testes do HAP-SEM.

## Vivacidade sem autonomia

A ponte fecha uma lacuna importante entre “o núcleo gerou uma resposta” e “a pessoa recebe um sinal”. Entretanto, a etapa háptica continua sendo apresentação. Para evitar que “vivo” vire sinônimo de “autônomo”, o contrato mantém quatro separações:

| Separação | Garantia do código |
|---|---|
| Geração versus execução | `actionable` é sempre zero no bridge |
| Plano versus ação | `PENDING` e `confirmation_required` permanecem ativos |
| Desconhecimento versus sucesso | `UNKNOWN` e `abstained` são preservados |
| Política versus falha comum | `SFTY/PRIVACY/DENIED` permanece distinguível |

O redteam em [`tools/test_generative_haptic_redteam.py`][3] remove cada uma dessas barreiras em mutantes separados. Todos foram mortos pelo benchmark, produzindo:

```text
GEN HAPTIC: 8 pass, 0 fail
GENERATIVE HAPTIC REDTEAM: 4/4 critical mutants killed
```

## Limites de alegação

Foi provado que a apresentação generativa local é tipada, bounded, codificável, decodificável e não acionável no host. Foi provado que planos, abstenções, conflitos e bloqueios não colapsam em ACK dentro do caminho testado.

Ainda não foi provado o reconhecimento acústico, a distinção háptica por pessoas, a calibração do DRV2605L, a resposta mecânica no pulso, a interferência com movimento, o consumo do atuador, a latência ponta a ponta ou a compreensão subjetiva dos códigos. Também não foi provado que um novo usuário aprenderá o código sem treinamento. Essas perguntas pertencem à bancada física e ao estudo humano posterior.

## Próxima fronteira

O próximo avanço pré-hardware é ampliar a vivacidade para um **ciclo de diálogo multietapa**: intenção local, recuperação grounded, geração bounded, evento háptico, confirmação física simulada, timeout e esquecimento. O benchmark deverá provar que uma interrupção no meio do ciclo não deixa uma resposta pendente, plano, adaptação ou evidência em estado ativo. A parte física só deve entrar depois que esse protocolo estiver fechado no host.

## Referências

[1]: https://github.com/SummaArs/herus/blob/feat/herus-semantic-compiler/firmware/core/generative_haptic_bridge.c "Ponte C11 entre geração e HAP-SEM"
[2]: https://github.com/SummaArs/herus/blob/feat/herus-semantic-compiler/firmware/core/test_generative_haptic_bridge.c "Benchmark de vivacidade generativa e round-trip háptico"
[3]: https://github.com/SummaArs/herus/blob/feat/herus-semantic-compiler/tools/test_generative_haptic_redteam.py "Redteam do canal generativo-háptico"
