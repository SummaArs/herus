# System One / Jev no HERUS

## Decisão

O Jev, da TypeSafe AI, é um modelo fechado de decisão estruturada. Ele não é um chatbot nem um gerador de texto. Recebe um estado e perguntas tipadas e retorna escolhas, scores ou probabilidades booleanas com confiança. A TypeSafe descreve uma arquitetura de amostragem paralela e treinamento chamado Reinforcement Learning for Calibrated Decisions (RLCD).

O HERUS não reproduz o modelo proprietário, seus pesos, seu treinamento ou suas alegações de latência. O que foi incorporado é o **contrato arquitetural útil**: decisões tipadas, distribuição explícita, execução paralela, confiança, abstention e nenhuma autoridade implícita.

## Correspondência

| Jev | HERUS |
|---|---|
| State | estado finito do hospedeiro e evidências canônicas |
| Noul | `QuestionKind.NOUL` |
| Choice | `QuestionKind.CHOICE` |
| Score | `QuestionKind.SCORE` |
| Probabilidades | distribuição inteira em 0–1000 |
| Confiança | confiança conservadora em 0–1000 |
| Parallel sampler | `ThreadPoolExecutor` para perguntas independentes |
| Harness | `SystemOneEngine` |
| Incerteza | `abstained=True` e motivo verificável |
| Autoridade | permanece `NONE`; o resultado nunca é executável |

## O que isso melhora

O SIM anterior produzia uma única decisão de classificação. A camada System One permite consultar várias propriedades do mesmo estado em uma chamada lógica: urgência, rota, risco, compatibilidade, necessidade de confirmação e outras perguntas finitas. Isso reduz acoplamento entre o modelo e o fluxo de controle e aproxima a integração de um sistema de software real.

A camada também separa três questões que não devem ser confundidas:

1. **O que o sistema estima?** — distribuição probabilística.
2. **Quão forte é a evidência?** — confiança e gate de abstention.
3. **O que pode ser executado?** — nada nesta camada; execução continua com o verificador de autoridade do HERUS.

## Limites

A implementação atual é um harness local e uma API de contrato. `FiniteStateOracle` não é uma rede neural nova; ele consome evidências finitas já fornecidas pelo estado. O HERUS ainda não possui os pesos de Jev nem uma implementação pública do RLCD. Portanto, não se deve declarar equivalência de inteligência, calibração, custo ou velocidade com Jev.

A implementação também não transforma probabilidade em autorização. Uma probabilidade de 99,9% pode permitir uma classificação de baixa consequência, mas nunca libera sozinha um atuador, envio, compra, alteração de política ou ação crítica.

## Próximo experimento

O próximo experimento correto é rodar o System One em shadow mode sobre os casos existentes do HERUS:

- comparar regras determinísticas, SIM e decisões tipadas;
- medir calibração por bins de confiança;
- medir falsos aceites e falsos bloqueios;
- medir latência serial e paralela;
- testar deriva, evidência ausente e conflito;
- automatizar apenas decisões de baixo risco.

A regra é a mesma recomendada para qualquer decisão probabilística: preservar o comportamento determinístico, coletar resultados ao lado, revisar erros e somente depois promover uma classe limitada de decisão.

## Fontes

- TypeSafe AI, [Introducing System One Models & Jev](https://typesafe.ai/blog/introducing-system-one-models-and-jev)
- LangChain, [Building a Harness with Jev](https://www.langchain.com/blog/building-a-harness-with-jev)
- Flavio Copes, [A deep dive into Jev](https://flaviocopes.com/jev/)
