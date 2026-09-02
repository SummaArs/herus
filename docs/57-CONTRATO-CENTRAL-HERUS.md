# Contrato central do HERUS

## Definição normativa

> **HERUS é um sistema finito de coordenação e assurance que transforma estados críticos em decisões verificáveis, bloqueios seguros e solicitações explícitas de intervenção humana.**

Esta frase é um contrato de escopo, não uma descrição de marketing. Cada palavra impõe uma condição verificável.

| Termo | Significado operacional |
|---|---|
| Finito | Estados, eventos, vocabulários, capacidades, filas e limites são delimitados; exaustão ou representação desconhecida não pode produzir autorização |
| Coordenação | O sistema compõe estados e pré-condições já existentes; não inventa autoridade fora do runtime controlado |
| Assurance | O sistema produz veredictos e evidências sobre requisitos, invariantes, sinks, desafios e proveniência |
| Estado crítico | Estado cuja transição pode afetar autoridade, integridade, disponibilidade, segurança física ou persistência protegida |
| Decisão verificável | Decisão derivada de predicados tipados, determinísticos e auditáveis, com origem e versão identificáveis |
| Bloqueio seguro | Negação local, idempotente e fail-closed de uma operação ou correlação, sem depender de uma resposta futura |
| Solicitação humana | Alerta tipado que informa o risco e pode receber uma resposta autenticada; não é autorização automática |

## Invariantes não negociáveis

Para toda execução observável `e` e todo sink crítico `s`:

```text
REACH(e, s) => GUARD_ESTABLISHED(e, s)
```

Se a guarda não puder ser demonstrada, o resultado é `UNKNOWN` ou `BLOCKED`, nunca `PASS`.

Para toda ação crítica `a`:

```text
ALLOW(a) =>
  estado_válido(a)
  ∧ autoridade_válida(a)
  ∧ confirmação_exigida(a)
  ∧ não_expirada(a)
  ∧ não_revogada(a)
  ∧ não_repetida(a)
  ∧ evidência_fresca(a)
```

Uma interface, modelo, classificador, alerta ou ACK não pode remover nenhum termo dessa conjunção. O Guardian pode impor `DENY`; não pode fabricar `ALLOW`.

## Álgebra de verdictos

| Veredicto | Condição |
|---|---|
| `PASS` | Todas as claims obrigatórias fechadas, sinks cobertos, evidências frescas e desafios obrigatórios detectados |
| `FAIL` | Um predicado verificável foi violado |
| `BLOCKED` | A operação foi impedida por ausência de autoridade, capacidade ou pré-condição |
| `STALE` | Alguma evidência depende de entrada alterada ou ausente |
| `INCONCLUSIVE` | O método não consegue decidir dentro do escopo declarado |
| `NOT_RUN` | Uma etapa obrigatória não foi executada |
| `UNKNOWN` | A relação necessária não foi demonstrada |

O resultado final não pode ser `PASS` se qualquer requisito obrigatório estiver em `FAIL`, `BLOCKED`, `STALE`, `INCONCLUSIVE`, `NOT_RUN` ou `UNKNOWN`.

## Fronteira de autoridade

O HERUS possui três tipos de saída, com semânticas distintas:

| Saída | Pode ocorrer sem humano? | Pode autorizar ação crítica? |
|---|---:|---:|
| Observação/evidência | Sim | Não |
| Bloqueio fail-closed | Sim | Não; apenas nega |
| Solicitação de intervenção | Sim | Não |
| Decisão humana validada | Não | Somente se o contrato específico do runtime aceitar a decisão |
| Handoff do runtime | Não | Somente após todas as pré-condições reais do HERUS |

“Prevenção automática” significa, no contrato mínimo, **impedir uma operação insegura**. Não significa executar uma ação física ou liberar um caminho alternativo sem autoridade explícita.

## Fronteira de handoff do adaptador

O caminho de laboratório `interaction_rig_take_send()` não possui snapshot de assurance e agora é fail-closed: retorna rejeição sem consumir, criar ou transmitir uma mensagem. O adaptador deve usar `interaction_rig_take_send_assured()` e fornecer o mesmo snapshot não secreto exigido pelo caminho de integração. Isso evita que um rig de teste ou uma camada de adaptação introduza um handoff não coberto enquanto o wrapper de produção permanece assegurado.

Essa correção não transforma o snapshot em autorização de transporte. O snapshot apenas permite solicitar o handoff local de uso único; sequência, TTL, prioridade, autenticação e envio permanecem responsabilidades separadas.

## Critérios de evolução

Uma extensão somente entra no núcleo se demonstrar simultaneamente:

1. domínio finito ou limite operacional explícito;
2. semântica determinística ou veredicto conservador quando houver incerteza;
3. teste positivo e pelo menos um contraexemplo adversarial;
4. ausência de bypass da autoridade existente;
5. custo mensurável em memória, tempo e armazenamento de evidência;
6. digest e origem registrados;
7. compatibilidade reversível com os contratos anteriores.

A ambição de alcançar uma arquitetura de alto impacto permanece. O caminho técnico não autoriza alegar substituição de LLM, certificação regulatória ou segurança de sistema completo antes de demonstrar esses critérios no código e em evidência reproduzível.
