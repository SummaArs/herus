# HERUS — Taxonomia multi-hospedeiro e multi-domínio

## Núcleo comum

O HERUS não deve adaptar suas garantias para cada mercado. Ele deve adaptar o vocabulário, o orçamento, as interfaces, as Skills e o nível de efeito permitido. O núcleo comum trata quatro objetos: intenção, observação, Skill e efeito.

| Objeto | Pergunta invariável |
|---|---|
| Intenção | O que se pretende fazer? |
| Observação | O que foi realmente medido ou recebido? |
| Skill | Qual transformação verificável pode produzir uma proposta? |
| Efeito | O que mudará fora do núcleo e quem autorizou? |

Cada domínio fornece um `HostProfile`, um vocabulário finito, um conjunto de Skills, um orçamento e uma política de efeitos. O núcleo não aceita que um domínio transforme uma proposta em ação apenas porque seu risco é diferente.

## Taxonomia de hospedeiros

| Classe | Exemplos | Recursos adaptáveis | Primeiro efeito permitido |
|---|---|---|---|
| Pessoal | pulso, telefone, computador | bateria, memória, rádio, botão, haptic, tela | proposta e confirmação humana |
| Robótica | braço, rover, drone, dispositivo móvel | sensores, atuadores, cinemática, latência, zona segura | simulação e shadow; depois ação limitada |
| Financeiro | carteira, simulador, sistema de ordens | saldo de teste, mercado, latência, limites, janela | proposta em sandbox; nunca transferência implícita |
| Servidor | serviço, agente, pipeline, banco | CPU, RAM, rede, permissões, disponibilidade | dry-run, canary e rollback |
| Crítico | energia, transporte, indústria, saúde | sensores, redundância, intertravamento, watchdog | recomendação; execução sob controle externo |

## Níveis de efeito

O HERUS precisa de uma escada de promoção que seja comum aos domínios, ainda que cada domínio defina seus próprios critérios:

```text
OBSERVE
→ PROPOSE
→ SIMULATE
→ SHADOW
→ CANARY
→ HUMAN_BOUND
→ ACTIVE
```

`OBSERVE` não executa nada. `PROPOSE` produz uma intenção tipada. `SIMULATE` usa um ambiente sem efeito real. `SHADOW` compara o que seria feito com o que o operador fez. `CANARY` limita escopo, tempo, orçamento e reversibilidade. `HUMAN_BOUND` exige autorização explícita vinculada ao contexto. `ACTIVE` é uma promoção excepcional, específica do domínio e nunca concedida pela adaptação sozinha.

## Regras por domínio

No pulso, o primeiro objetivo é demonstrar comunicação, haptic e confirmação sem autoridade implícita. Na robótica, o primeiro objetivo é validar estado, zona, trajetória e parada segura em simulação. Em finanças, o primeiro objetivo é explicar uma proposta contra dados históricos ou conta de teste, sem ordens reais. Em servidores, o primeiro objetivo é produzir planos de mudança reversíveis em shadow. Em sistemas críticos, o primeiro objetivo é detectar e explicar condições de risco, não assumir controle autônomo.

A arquitetura será considerada multi-domínio quando o mesmo núcleo passar por pelo menos três classes de hospedeiro com contratos diferentes, mantendo as mesmas propriedades de recusa, proveniência, orçamento, autoridade e auditoria. Isso não exige que uma Skill seja universal; exige que o mecanismo de adaptação seja comum e que as incompatibilidades sejam explícitas.

## Critério de mundo real

Um domínio entra no mundo real somente depois de demonstrar uma cadeia completa de evidência: dados com proveniência, simulação, casos ocultos, falhas adversariais, shadow, canary reversível, revisão humana, rollback e registro auditável. Finanças e sistemas críticos exigem gates adicionais de conformidade, segregação de funções e controle externo; o HERUS não substitui esses controles.

> **O objetivo não é colocar uma inteligência irrestrita em qualquer lugar. É colocar um núcleo que sabe exatamente onde pode operar, onde precisa de autorização e quando deve permanecer inerte.**
