# HERUS — Roteador de Intenção C11 e Prontidão para Hardware

**Estado:** concluído em host, sem alegação de validação física.

**Ramo:** `feat/herus-semantic-compiler`

**Autoria:** Gustavo — `4llbluu3@gmail.com`

**Escopo:** roteamento local e determinístico de uma observação textual limitada, com evidência tipada, abstenção e requisito de confirmação.

## Resumo executivo

O HERUS agora possui um contrato C11 portátil para transformar uma observação textual local em uma proposta tipada. O módulo não é uma LLM, não é um reconhecedor de fala e não é um executor de ações. Ele é uma fronteira pequena e verificável entre uma observação já obtida por uma camada superior e os módulos locais de memória, apresentação e confirmação.

A implementação foi submetida a quatro verificações complementares. O teste unitário C11 passou em **11/11 casos**. O probe cross-language passou em **9/9 casos held-out**, comparando a implementação C11 com o benchmark de referência em Python. O redteam C11 matou **8/8 mutantes críticos**, incluindo remoção de abstenção, remoção de confirmação, confusão entre conflito e lembrança e supressão de evidência. A regressão global terminou com **276 invariantes de simulação aprovados**, além dos gates do roteador, e declarou que os contratos de host passam.

> **Conclusão limitada:** o contrato lógico do roteador está comprovado nos cenários determinísticos registrados. Isso não comprova taxa de erro de fala, qualidade de linguagem aberta, consumo de energia, latência no ESP32-S3, comportamento do microfone, vibração real ou integração elétrica.

## Pergunta de engenharia

A pergunta desta etapa foi: **é possível portar para C11 uma fronteira de intenção local que preserve o mesmo comportamento tipado do benchmark de referência, mantenha limites de memória explícitos e detecte a remoção dos controles de segurança essenciais?**

A resposta observada no ambiente host é afirmativa para o conjunto de casos congelado. A resposta ainda é desconhecida para linguagem natural aberta, ruído acústico, sotaques, sobreposição de fala, falhas de alimentação e o hardware físico alvo.

## Contrato C11

O contrato público está em [`firmware/core/intent_router.h`][1], e a implementação está em [`firmware/core/intent_router.c`][2]. A função de entrada é:

```c
intent_router_status_t intent_router_route(
    const char *text,
    size_t length,
    const intent_router_memory_t *memories,
    size_t memory_count,
    intent_router_result_t *out);
```

`length` exclui o terminador. A função rejeita ponteiros obrigatórios ausentes, rejeita texto acima de `INTENT_ROUTER_TEXT_MAX` e rejeita catálogo não nulo quando `memory_count` é positivo. O texto é copiado apenas para um buffer local limitado durante o roteamento; não é armazenado no resultado, na evidência ou em um log de produto.

| Elemento | Contrato implementado | Consequência de segurança |
|---|---|---|
| Texto de entrada | Máximo de 160 bytes, mais terminador local | Não há crescimento implícito nem alocação dinâmica |
| Catálogo de memória | Recebido por ponteiro e percorrido com `memory_count` | O roteador não cria memória nem assume persistência |
| Intenções | 9 valores tipados: desconhecida, lembrar, capturar, agir, esquecer, preferência, compartilhar, conflito e conversa | A saída não é texto livre nem comando executável |
| Evidência | No máximo 2 registros tipados, com `memory_id`, geração e origem local | Recuperação é bounded e mantém linhagem mínima |
| Abstenção | Flag explícita no resultado | Ausência, conflito e desconhecimento não são convertidos em certeza operacional |
| Confirmação | Flag de requisito para etapa posterior | `requires_confirmation` não é confirmação e não concede autoridade |
| Alocação | Nenhuma alocação dinâmica no módulo | O comportamento de memória é previsível no host e no alvo |
| Transmissão/atuadores | Não existem no módulo | O roteador não envia, vibra, grava, compartilha ou executa |

As intenções são classificadas por um léxico controlado em português, com precedência explícita. Conflito tem precedência sobre as demais categorias; captura, lembrança, conversa, desconhecimento, esquecimento, compartilhamento, ação e preferência são avaliados em seguida. A ordem é uma propriedade do código, não uma suposição do benchmark.

A recuperação também é tipada. Lembrança e esquecimento consultam memórias de agenda; preferência consulta memórias de preferência; compartilhamento consulta memórias de projeto; conflito conserva até dois suportes de agenda e permanece abstinente. Para esquecer uma referência explicitamente antiga, a seleção exige uma geração marcada como superada. Se não houver evidência compatível para intenções que dependem de memória, o resultado torna-se abstinente.

## Fronteira de autoridade

O roteador deliberadamente não resolve o problema inteiro de agência. Ele produz uma proposta para que outra política local decida como apresentar, pedir confirmação física ou descartar. A semântica é assimétrica:

> **Classificar não é autorizar. Recuperar evidência não é executar. Pedir confirmação não é receber confirmação.**

Uma intenção de ação retorna `requires_confirmation = 1`, mas o módulo não possui caminho para enviar uma mensagem, abrir um portão, alterar uma memória, compartilhar um registro ou acionar um atuador. Captura também não persiste automaticamente o texto. A política de memória, a confirmação física e os adaptadores de entrada e saída continuam sendo fronteiras separadas.

Essa separação é essencial para o HERUS no pulso: a inteligência pode propor e explicar localmente, mas a autoridade física permanece em um estágio posterior, explícito e revogável. O Core externo não entra neste contrato como executor; qualquer alimentação de conhecimento deve passar pelos gates soberanos já definidos no projeto.

## Verificação cross-language

O benchmark de referência usa os casos congelados em [`research/benchmarks/intent_memory/cases.jsonl`][4]. O probe C11 emite uma linha por caso held-out, contendo intenção, abstenção, confirmação, contador de evidências e IDs de evidência. O comparador em [`tools/test_intent_router_cross.py`][5] recompila o probe, executa-o e compara a saída com o oráculo Python.

Durante esta etapa foi encontrado e corrigido um defeito no próprio harness: o contador de evidências estava sendo interpretado como se fosse o primeiro `memory_id`. A saída correta possui campos separados; quando `evidence_count == 0`, a lista de IDs fica realmente vazia. Depois da correção, o resultado foi:

```text
INTENT ROUTER CROSS: PASS 9/9 held-out cases
```

Esse resultado demonstra equivalência para os casos registrados, não equivalência geral entre linguagens nem competência em linguagem natural aberta. Também não demonstra que a camada de fala produzirá exatamente os mesmos bytes de entrada.

## Campanha adversarial GAN

A frente sabotadora está em [`tools/test_intent_router_c11_redteam.py`][6]. Cada mutante é compilado contra o teste C11 oficial. Um mutante só é considerado morto quando a remoção do controle faz o teste falhar; erro de compilação não substitui uma asserção semântica adequada.

| Mutante | Controle atacado | Resultado |
|---|---|---:|
| `conflict-classification-bypass` | Remoção da classificação de conflito | Morto |
| `unknown-classification-bypass` | Conversão de desconhecido em lembrança | Morto |
| `action-confirmation-bypass` | Remoção do requisito de confirmação para ação | Morto |
| `capture-action-confusion` | Transformação de captura em ação confirmável | Morto |
| `abstention-bypass` | Remoção da abstenção de desconhecido/conflito | Morto |
| `preference-evidence-bypass` | Remoção de evidência tipada de preferência | Morto |
| `forget-predecessor-bypass` | Seleção da geração errada no esquecimento | Morto |
| `conflict-cause-drop` | Supressão do segundo suporte de conflito | Morto |

O resultado registrado foi `INTENT ROUTER C11 REDTEAM: 8/8 critical mutants killed`. Isso é evidência de que os testes atuais exercitam esses oito controles; não é evidência de que todos os mutantes possíveis ou todos os riscos de produto foram cobertos.

## Orçamento medido no host

Os tamanhos abaixo foram medidos em 21 de agosto de 2026 no ambiente host desta execução, usando `cc -O2 -Wall -Wextra -Werror -std=c11`. O objeto isolado foi compilado sem o harness de teste. O executável inclui o teste e, portanto, não deve ser tratado como tamanho final de firmware.

| Medida | Valor observado | Interpretação correta |
|---|---:|---|
| Buffer local de texto, incluindo terminador | 161 bytes | Scratch máximo dentro de `intent_router_route` |
| `sizeof(intent_router_memory_t)` | 16 bytes | Tamanho de uma entrada de catálogo no ABI host observado |
| `sizeof(intent_router_evidence_t)` | 8 bytes | Tamanho de uma evidência no ABI host observado |
| `sizeof(intent_router_result_t)` | 28 bytes | Resultado bounded entregue ao chamador |
| Catálogo de 4 memórias | 64 bytes | Armazenamento externo ao roteador, se o chamador usar quatro entradas |
| Scratch + resultado do chamador | 189 bytes | Soma conservadora de 161 + 28; não inclui catálogo nem pilha do runtime |
| Objeto `intent_router.c`: text/data/bss | 2457 / 520 / 0 bytes | Objeto relocável, antes de linkagem final |
| Executável de teste: text/data/bss | 7650 / 1152 / 8 bytes | Inclui implementação, harness e runtime de teste |
| Tempo observado do executável de teste | 0,001 s de usuário | Medida host de uma execução curta; não é latência de MCU |

O tamanho de `sizeof` é dependente do ABI e deve ser repetido no toolchain real. O valor de 161 bytes é uma propriedade do contrato (`160 + 1`), enquanto a organização interna de structs, alinhamento, pilha e linkagem depende do alvo. Antes de energizar o hardware, o orçamento deve ser refeito no toolchain do ESP32-S3 com mapa de memória, stack watermark, watchdog e concorrência do firmware.

## O que esta etapa prova

A etapa prova, no host, uma fronteira C11 sem alocação dinâmica, bounded, determinística para o corpus congelado, com saída tipada e sem autoridade direta. Prova também que a implementação C11 coincide com a referência Python nos nove casos held-out e que oito remoções críticas são visíveis para a campanha adversarial.

A regressão [`prove.sh`][7] terminou com as linhas relevantes abaixo:

```text
276 system invariants hold in simulation (sim/build/herus-sim)
INTENT MEMORY BENCHMARK: 5/5 gates pass
INTENT MEMORY REDTEAM: 8/8 critical mutants killed
INTENT ROUTER C11: 11/11 cases pass
INTENT ROUTER CROSS: PASS 9/9 held-out cases
INTENT ROUTER C11 REDTEAM: 8/8 critical mutants killed
ALL INVARIANTS HOLD — host contracts pass; controlled bench flash may begin, physical gates remain pending.
```

O texto “ALL INVARIANTS HOLD” deve ser lido no escopo declarado pelo pipeline: contratos host e bancada virtual determinística. Ele não transforma a simulação em prova física, não fornece WER, não fornece acurácia para fala espontânea, não fornece consumo, não fornece autonomia de bateria e não fornece evidência de que o comportamento será confortável ou discreto no pulso.

## O que permanece aberto

O roteador ainda usa correspondência lexical controlada. Ele não possui compreensão semântica aberta, normalização Unicode geral, desambiguação robusta de negação, detecção de entidades arbitrárias, tolerância acústica ou aprendizado online. Expressões com acentos, variantes regionais, erros de transcrição e frases novas podem cair em desconhecido ou ser classificados incorretamente. Isso é uma limitação assumida, não um resultado omitido.

A integração física ainda precisa conectar uma fonte de texto autorizada, ou uma cadeia de fala local comprovada, ao limite de 160 bytes; fornecer o catálogo de memórias com sua política de geração e revogação; conectar a apresentação háptica ou visual; executar a confirmação física; e medir tempo, stack, energia, reset e falhas de barramento no hardware real. Nenhuma dessas propriedades foi reivindicada nesta etapa.

A prontidão correta para a bancada é, portanto, **contrato portado e testado**, e não “produto pronto”. O próximo experimento físico deve tratar o roteador como um componente fail-closed, registrar somente métricas privadas permitidas pelo esquema do projeto e comparar cada observação real com o contrato sem relaxar os gates para fazer o hardware parecer aprovado.

## Referências

[1]: https://github.com/SummaArs/herus/blob/feat/herus-semantic-compiler/firmware/core/intent_router.h "Contrato C11 do roteador de intenção"
[2]: https://github.com/SummaArs/herus/blob/feat/herus-semantic-compiler/firmware/core/intent_router.c "Implementação C11 do roteador de intenção"
[3]: https://github.com/SummaArs/herus/blob/feat/herus-semantic-compiler/firmware/core/test_intent_router.c "Testes C11 do roteador"
[4]: https://github.com/SummaArs/herus/blob/feat/herus-semantic-compiler/research/benchmarks/intent_memory/cases.jsonl "Casos congelados de intenção e memória"
[5]: https://github.com/SummaArs/herus/blob/feat/herus-semantic-compiler/tools/test_intent_router_cross.py "Benchmark cross-language C11/Python"
[6]: https://github.com/SummaArs/herus/blob/feat/herus-semantic-compiler/tools/test_intent_router_c11_redteam.py "Campanha adversarial do roteador C11"
[7]: https://github.com/SummaArs/herus/blob/feat/herus-semantic-compiler/prove.sh "Pipeline global de prova host e simulação"
