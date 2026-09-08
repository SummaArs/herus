# HERUS — Inteligência invisível e critério de excelência

**Estado:** especificação de produto e contrato host-side; não substitui contato humano nem validação em hardware.  
**Princípio:** o state of the art do HERUS não será “produzir muitas respostas”, mas **reduzir a necessidade de o usuário pensar no dispositivo** sem retirar sua autoridade.

> A tecnologia desaparece quando a pessoa age naturalmente, o HERUS percebe apenas sinais autorizados, permanece silencioso quando sua utilidade é incerta e aparece no instante certo como uma possibilidade pequena, compreensível e reversível.

## 1. O que deixa de ser critério suficiente

A contagem de invariantes continua útil como mapa de cobertura, mas não pode ser apresentada como prova do sistema inteiro. Uma suíte pode demonstrar que um contrato C11 específico fecha em host; ela não demonstra que a memória é relevante, que a intervenção é desejável, que o usuário entende a vibração, que a voz funciona, que a autonomia é confortável ou que o hardware é confiável.

A partir deste avanço, o ledger deve ser lido como **evidência particionada**, e não como um número único. Cada resultado precisa declarar qual propriedade foi observada, em qual ambiente, com quais entradas e qual parte permanece desconhecida.

| Camada de evidência | Pergunta que pode responder | O que ainda não prova |
|---|---|---|
| Contrato unitário C11 | A função fecha estados inválidos e não concede autoridade? | Utilidade ou conforto no mundo real |
| Cenário host-side | O fluxo mantém silêncio, abstention e limites em uma sequência controlada? | Que pessoas interpretarão o sinal corretamente |
| Integração no firmware | Os módulos compõem sem quebrar os gates? | Temporização, consumo e robustez elétrica |
| Bancada física | O dispositivo opera com sensores, rádio, haptics e energia reais? | Aceitação social ou indispensabilidade |
| Contato humano | Pessoas entendem, toleram e desejam o comportamento? | Generalização automática para todos os contextos |

## 2. Definição operacional de “invisível”

“Invisível” não significa escutar tudo, registrar tudo ou agir sem autorização. Significa que o sistema mantém um estado latente, mínimo e temporário enquanto a pessoa vive sua rotina. Esse estado só atravessa a fronteira de atenção quando quatro condições independentes se alinham: **relevância provável**, **baixo risco**, **janela de atenção autorizada** e **orçamento de intervenção disponível**.

Quando uma dessas condições falha, a saída correta é silêncio ou abstention. Ausência de saída não é falha: é uma decisão de segurança e de ergonomia. Uma intervenção repetida porque a primeira não recebeu resposta é classificada como intrusão, não como persistência inteligente.

## 3. Contrato da presença ambiente

O novo contrato deve tratar cada observação como uma oportunidade, nunca como uma ordem. O HERUS poderá manter apenas metadados tipados de curta duração, como classe de situação, relevância, risco, novidade, validade e motivo de descarte. Não poderá manter áudio, transcrição, embedding, identidade, localização ou conteúdo de terceiro.

Uma oportunidade pode assumir apenas um destes destinos:

| Destino | Significado |
|---|---|
| `QUIET` | Não há benefício provável ou a atenção não está disponível; nenhum sinal é emitido. |
| `HOLD` | Há benefício possível, mas o instante é ruim; o estado aguarda uma janela limitada e não cresce em autoridade. |
| `OFFER` | Uma única microintervenção local é permitida; ela não transmite, não grava e não executa ação. |
| `ABSTAIN` | A evidência é ambígua, sensível, conflitante ou insuficiente; o HERUS não escolhe por conveniência. |
| `EXPIRED` | A oportunidade perdeu validade e é apagada, sem reprocessamento automático. |
| `ACKNOWLEDGED` | O usuário teve contato físico ou gesto autorizado; isso registra apenas a recepção do sinal, não confirma memória, envio ou ação. |

A promoção para memória, transmissão ou atuação permanece nos contratos existentes e exige sua própria autoridade física. A camada de presença não pode usá-la como atalho.

## 4. Métricas que importam mais que a contagem bruta

Antes do hardware, as métricas são propriedades de cenários determinísticos, não alegações sobre pessoas. O objetivo é medir o comportamento do agente em relação à atenção humana:

| Métrica host-side | Critério de sucesso |
|---|---|
| Silêncio apropriado | Nenhum sinal em contexto sem janela, sem consentimento, sensível, ambíguo ou expirado. |
| Intervenção única | Uma oportunidade não confirmada não gera repetição automática. |
| Latência semântica | Uma oportunidade mantida em `HOLD` não pode virar `OFFER` apenas por envelhecer. |
| Reversibilidade | Toda saída de `OFFER` pode ser ignorada ou descartada sem criar autoridade. |
| Privacidade | O estado persistido pela camada contém apenas tipos permitidos e é zerado no fechamento/expiração. |
| Não substituição | A camada não escolhe entre conflitos nem transforma baixa confiança em certeza. |
| Densidade de valor | Em cenários com sinais suficientes, um único offer útil vence o silêncio; isso é uma propriedade de composição, não uma taxa de acerto humano. |
| Respeito a contato | O estado só muda para `ACKNOWLEDGED` após evento físico autorizado, sem inferir aprovação. |

## 5. Pergunta de produto

A pergunta correta deixa de ser “quantos testes passaram?” e passa a ser:

> **Em quais situações o HERUS conseguiu não incomodar, preservar a autoridade e ainda estar disponível no instante em que uma pequena ajuda era justificável?**

Essa pergunta poderá ser respondida progressivamente. No host, por rastros tipados e mutações. Na bancada, por temporização, energia, haptics e rádio. Com pessoas, por observação de entendimento, confiança, carga cognitiva e vontade de continuar usando. Nenhuma camada deve ser usada para fingir que a seguinte já foi validada.

## 6. Implementação host-side concluída

A política de **presença ambiente** foi implementada em `firmware/core/ambient_presence.{h,c}`. Ela não é uma nova LLM nem um executor. Consome somente observações tipadas já autorizadas, mantém uma oportunidade transitória, aplica expiração e cooldown, produz no máximo uma microoferta local e limpa seu estado. A confirmação física apenas reconhece o recebimento do sinal; não promove memória, transmissão ou ação.

A suíte de propriedades cobre silêncio sem atenção ou consentimento, abstention por baixa confiança, bloqueio de privacidade, janela inválida, retenção em `HOLD`, oferta única, ausência de confirmação, expiração, cooldown, monotonicidade e `forget`. A campanha GAN removeu seis gates críticos e matou **6/6 mutantes**. A regressão global passou com **78 suítes** e **111 invariantes de simulação**.

Isso permite avançar sem hardware sem fingir que o problema físico foi resolvido. Quando o dispositivo existir, o contrato poderá receber adaptadores reais para relógio monotônico, haptic e gesto; os testes de produto terão de ser recalibrados com pessoas. O núcleo lógico já demonstra uma propriedade importante, em host: **o HERUS não tenta ser notado o tempo todo**.
