# HERUS — Continuidade Semântica em Diálogo Composto

**Estado:** concluído em host; hardware, áudio e percepção háptica ainda não validados.

**Ramo:** `feat/herus-semantic-compiler`

**Autoria:** Gustavo — `4llbluu3@gmail.com`

**Escopo:** conectar roteamento, memória semântica, geração local, adaptação pessoal, HAP-SEM e reboot em um único fluxo bounded.

## Resumo

O HERUS agora possui um orquestrador composto em [`firmware/core/composed_dialogue.c`][1]. Uma observação textual emprestada é roteada para uma intenção tipada, abre uma sessão física, escolhe uma consulta ou plano, combina memória semântica corrente quando disponível, gera um candidato local, apresenta um evento HAP-SEM e mantém o estado até confirmação, negação, timeout, interrupção ou esquecimento.

A implementação não guarda a observação textual. O orquestrador armazena apenas o resultado tipado bounded, o request estruturado e os metadados necessários durante o turno. A memória grounded continua sendo fornecida por um índice reviewed e temporal. A adaptação pessoal pode alterar somente a apresentação. Nenhuma função do módulo executa, transmite, persiste ou concede autoridade.

O benchmark passou **31/31 casos** e o redteam composto matou **6/6 mutantes críticos**. A regressão global preservou **276 invariantes de simulação** e terminou com `ALL INVARIANTS HOLD`. O resultado é uma prova de composição no host, não uma prova de conversa humana aberta ou de equivalência com uma LLM.

> **Alegação permitida:** componentes locais já provados podem atravessar uma sequência de diálogo com grounding, abstenção, confirmação, limpeza e recuperação pós-reboot sem selecionar evidência expirada ou reativar uma sessão antiga.

> **Alegação proibida:** o HERUS já tem memória humana completa, consciência, autonomia geral, voz robusta ou uma LLM aberta embarcada.

## Fluxo composto

| Etapa | Entrada | Saída | Controle dominante |
|---|---|---|---|
| Roteamento | observação local emprestada | intenção + evidência bounded | limite de texto e tipo |
| Sessão | gesto físico validado | `physical_session_id` transitório | sessão não nula |
| Geração | intenção + consulta tipada | candidato `gc_result_t` | orçamento e abstenção |
| Grounding | índice reviewed corrente | card/review/generation | expiração e conflito |
| Adaptação | perfil local consentido | estilo de apresentação | margem, revogação, quarentena |
| HAP-SEM | resultado generativo | evento numérico | encode/decode e perfil |
| Confirmação | mesma sessão física | pending→confirmed | one-shot, sem execução |
| Encerramento | timeout, aborto ou forget | payload zerado | privacidade |
| Reboot | floor recuperado | quarentena + rearmamento | geração estritamente nova |

A máquina de alto nível não interpreta a forma verbal como autorização. Uma frase que pede ação pode virar um plano pending, mas o módulo não possui a operação seguinte. A confirmação apenas fecha uma transição de apresentação; a execução real continua em um limite externo, físico e explicitamente confirmado.

## Cenários comprovados

O benchmark em [`firmware/core/test_composed_dialogue.c`][2] utiliza fixtures reais dos módulos C11 existentes e cobre seis famílias de comportamento. O recall corrente é grounded no card revisado e retorna a provenance de card, review e geração. A adaptação técnica muda a apresentação sem mudar grounding ou autoridade. Ação e captura entram como proposta pending. Desconhecimento e conflito abstêm. Evidência expirada não é usada. Depois do reboot, o índice com floor divergente ou geração igual é rejeitado, enquanto uma geração estritamente posterior pode ser rearmada explicitamente.

| Cenário | Resultado observado |
|---|---|
| Recall corrente | grounded, `MEM`/`ACK`, não acionável |
| Recall com estilo pessoal | adaptação presente; prova e provenance preservadas |
| Ação | `PLAN`/`PENDING`, confirmação exigida |
| Sessão errada | `CDH_E_PHYSICAL`; plano permanece pending |
| Replay de confirmação | erro de estado |
| Desconhecido | `ABSTAINED`, sem resposta inventada |
| Conflito | `CONTRADICTED`, alerta háptico, nenhum lado escolhido |
| Card expirado | abstention; nenhum ghost memory |
| Reboot | rota/request/candidato/sessão zerados; quarentena ativa |
| Rearmamento igual ou floor divergente | rejeitado |
| Rearmamento novo | idle explícito; somente card novo pode groundear |

## Redteam

O redteam em [`tools/test_composed_dialogue_redteam.py`][3] removeu seis controles: bypass da quarentena, aceitação de geração igual ao floor, bypass de floor divergente, uso de memória expirada, conversão de conflito em seleção e retenção de payload no reboot. Todos os mutantes foram detectados pelo benchmark.

```text
COMPOSED DIALOGUE: 31 pass, 0 fail
COMPOSED DIALOGUE REDTEAM: 6/6 critical mutants killed
```

O resultado é significativo porque testa propriedades de composição, não apenas unidades isoladas: um módulo sabotado em uma fronteira central precisa produzir uma falha observável no cenário que atravessa várias camadas.

## Orçamento e privacidade

O orçamento host-side em [`tools/generative_core_budget.py`][4] mede objetos C11 individuais e aplica limites deliberados de text/data/bss. O resultado atual é **3779/0/0 bytes** para `generative_core`, **1267/0/0 bytes** para `personal_adapter` e **3164/0/0 bytes** para `composed_dialogue`. Esses valores são do compilador e ABI host usados na prova; não são consumo de RAM, flash, energia ou tempo no ESP32-S3.

A análise de superfície do orquestrador confirma que não existe buffer de texto em `cdh_t`. A observação é passada a `intent_router_route` e não é copiada para a estrutura. O resultado composto contém enums, IDs, geração, flags, provenance e resposta transitória do núcleo generativo. O `cdh_forget` limpa esse payload, o request, a rota e o estado de sessão.

## Limitações honestas

A entrada continua sendo uma string fornecida pelo harness. Não foi medido reconhecimento de fala, detecção de voz do proprietário, ruído, WER, latência ponta a ponta, energia, vibração, distinção humana de códigos ou comportamento de RF. O índice semântico é reviewed e bounded; o fluxo não representa ainda a vida inteira de uma pessoa nem faz aprendizagem aberta de conhecimento.

A continuidade demonstrada é continuidade de estado e provenance dentro de contratos tipados. Ela não deve ser confundida com consciência, intenção própria ou autonomia. O próximo trabalho antes do hardware é submeter o mesmo fluxo a mais casos de composição linguística e a um corpus de diálogos locais controlados, mantendo a regra de que a qualidade generativa nunca substitui evidência, consentimento ou confirmação.

## Referências

[1]: https://github.com/SummaArs/herus/blob/feat/herus-semantic-compiler/firmware/core/composed_dialogue.c "Orquestrador C11 de diálogo composto"
[2]: https://github.com/SummaArs/herus/blob/feat/herus-semantic-compiler/firmware/core/test_composed_dialogue.c "Benchmark composto multi-turno"
[3]: https://github.com/SummaArs/herus/blob/feat/herus-semantic-compiler/tools/test_composed_dialogue_redteam.py "Redteam contra ghost memory, replay e bypass de floor"
[4]: https://github.com/SummaArs/herus/blob/feat/herus-semantic-compiler/tools/generative_core_budget.py "Gate de orçamento host-side dos objetos generativos"
