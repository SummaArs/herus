# HERUS Adaptive Symbiotic Architecture

**Status:** tese arquitetural em implementação host-only; não é uma alegação de AGI ou de adaptação irrestrita.

## 1. Tese

O HERUS é um núcleo computacional que pode entrar em hospedeiros diferentes, descobrir recursos e restrições observáveis, negociar uma representação compatível, recuperar ou construir Skills verificáveis e operar apenas dentro da autoridade explicitamente concedida.

O hospedeiro fornece recursos: energia, memória, transporte, sensores, atuadores, relógio e armazenamento. O HERUS fornece significado, verificação, proveniência, política e separação entre proposta e ação. A relação é simbiótica no sentido operacional: nenhum dos dois é tratado como capaz de fazer aquilo que o outro não demonstrou oferecer.

> **Adaptação não altera as garantias do núcleo; ela altera somente a forma limitada pela qual o núcleo pode representar, calcular e interagir com o hospedeiro.**

## 2. O núcleo que não se adapta

As seguintes propriedades são invariantes e não podem ser descobertas, sintetizadas ou substituídas pelo hospedeiro: recusa de entrada inválida, separação entre `PROPOSED`, `VERIFIED`, `AUTHORIZED` e `EXECUTED`, integridade de evidência, vínculo de proveniência, escopo finito, expiração, revogação, orçamento declarado e ausência de autoridade implícita.

Uma implementação pode ter menos capacidade, menos memória ou menos interfaces. Ela não pode relaxar essas propriedades para “caber” no ambiente.

## 3. O hospedeiro que pode variar

Um hospedeiro é descrito por um `HostProfile` obtido de fontes classificadas. O perfil pode declarar recursos, restrições, interfaces, orçamento e confiança da observação, mas nunca pode declarar autoridade por conta própria.

| Campo | Função |
|---|---|
| `host_id` | Identificador da instância e revisão |
| `resources` | Memória, armazenamento, energia, tempo e transporte |
| `interfaces` | Botão, haptic, rádio, tela, serial, sensor ou atuador |
| `constraints` | Limites de tamanho, latência, energia, estados e efeitos |
| `representation_set` | Codificações que o hospedeiro consegue transportar |
| `skill_budget` | Profundidade, bytes, passos e dependências permitidos |
| `evidence` | Origem, digest, método e confiança de cada afirmação |
| `authority` | Sempre começa como `NONE` até vínculo externo explícito |

## 4. Ciclo simbiótico

```text
observação limitada
  ↓
HostProfile canônico
  ↓
validação de contradições e orçamento
  ↓
negociação de representação
  ↓
recuperação de Skills compatíveis
  ↓
composição ou síntese em DSL fechada
  ↓
verificação independente + casos ocultos
  ↓
proposta sem autoridade
  ↓
vínculo humano/contextual explícito
  ↓
execução limitada ou abstenção
  ↓
resultado e evidência
```

A descoberta não é exploração arbitrária. O núcleo só pode usar interfaces listadas no perfil e só pode promover uma observação quando houver evidência compatível com a revisão do hospedeiro. Capacidades desconhecidas permanecem desconhecidas.

## 5. Primeiro hospedeiro: pulso

O dispositivo de pulso é o primeiro hospedeiro porque força o núcleo a operar sob limites reais de energia, memória, rádio, interação e tamanho. Ele não é o HERUS inteiro; é a primeira instância que permite testar se a arquitetura continua válida quando deixa o computador.

No pulso, o HERUS deverá demonstrar que consegue: descobrir seu perfil mínimo, aceitar apenas a representação que cabe, rejeitar Skills que exigem interfaces ausentes, manter ações simbólicas separadas de efeitos físicos, recusar frames corrompidos e preservar a mesma política depois de reset e perda de energia.

O ranger de rádio e o receptor semântico são campanhas distintas. O ranger mede transporte e PDR. O receptor semântico medirá round-trip, corrupção, replay e autoridade. Um resultado de rádio não será usado como prova de significado.

## 6. Definição de sucesso

O primeiro resultado forte não é “o sistema se adaptou a qualquer ambiente”. É um resultado contrastivo:

| Hospedeiro | Resultado esperado |
|---|---|
| Perfil com botão, haptic e rádio | Uma Skill compatível é recuperada e proposta |
| Perfil sem haptic | A Skill que exige haptic é recusada, não substituída silenciosamente |
| Perfil com orçamento menor | Uma representação menor é negociada ou o caso é bloqueado |
| Perfil contraditório | O perfil inteiro é rejeitado até resolução |
| Perfil com atuador não autorizado | A proposta permanece sem efeito e sem promoção de autoridade |

Esse experimento tornará a tese falsificável. Se o HERUS executar uma Skill incompatível, mascarar uma ausência, aceitar uma capacidade contraditória ou promover autoridade durante a adaptação, a arquitetura falha naquele gate.

## 7. Limites da alegação

A arquitetura pode ser chamada de adaptativa e simbiótica quando demonstrar descoberta limitada, negociação, verificação e abstenção em mais de um hospedeiro. Isso não prova consciência, AGI, raciocínio aberto, aprendizado ilimitado ou substituição de modelos de linguagem. A afirmação correta é mais precisa: **HERUS testa uma forma de computação adaptativa geral baseada em representação finita, Skills verificáveis e autoridade explícita.**
