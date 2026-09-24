# HERUS — Direção soberana e objetivo do simbionte

**Status:** direção oficial do projeto a partir de setembro de 2026.

## Decisão central

O HERUS seguirá uma direção diferente da de um assistente pessoal preso a um único ecossistema. Seu objetivo é tornar-se um **simbionte computacional autônomo, soberano e transferível**.

O simbionte deverá habitar sistemas autorizados, compreender as capacidades de cada hospedeiro, aprender Skills operacionais e remapear essas Skills entre softwares diferentes. A pessoa permanecerá no controle por meio de uma autoridade física, revogável e auditável. O dispositivo de pulso será a superfície de confirmação, estado, cancelamento e parada do sistema.

> **Objetivo futuro:** o HERUS deverá acompanhar a pessoa entre seus sistemas sem que seus dados pessoais precisem abandonar o domínio sob controle dela.

## O que “soberano” significa no HERUS

Soberania não significa apenas criptografar dados depois que eles já foram enviados para um provedor. Significa que o HERUS deve ser projetado para que os dados privados permaneçam no dispositivo, na rede local ou em infraestrutura diretamente controlada pelo usuário por padrão.

O princípio normativo é:

> **Dados privados não saem do domínio do usuário. Nenhum envio externo ocorre por padrão. Uma comunicação externa só pode existir quando for indispensável para uma ação explicitamente autorizada e quando o usuário puder saber, limitar, revogar e auditar o que foi transmitido.**

Na forma mais forte da arquitetura, o HERUS não enviará o dado pessoal bruto. Um gateway local deverá transformar a intenção em uma operação mínima, com escopo, prazo, nonce e prova de autorização. O sistema externo receberá apenas o necessário para executar a ação autorizada, enquanto a memória pessoal, os modelos privados e o histórico completo permanecerão sob controle do usuário.

Essa é uma **meta arquitetural**, não uma propriedade já demonstrada pelo protótipo atual. O repositório ainda não possui o armazenamento soberano completo, o gateway local, o hardware de pulso nem uma prova de não exfiltração em ambiente real.

## Relação com o Muse Charm

O lançamento do Muse Charm confirma a importância de combinar um agente pessoal com um dispositivo físico dedicado. A Meta descreve o Muse como um agente capaz de operar aplicações, manter memória pessoal e pedir aprovação antes de ações sensíveis [1]. O Charm é apresentado como um dispositivo dedicado para interagir com esse agente [2].

O HERUS não seguirá essa mesma dependência. O Charm é uma superfície dedicada para o agente Muse. O HERUS pretende ser o **agente soberano que pode habitar vários hospedeiros e usar diferentes superfícies físicas**.

A diferença central é esta:

| Muse Charm | HERUS soberano |
|---|---|
| Dispositivo dedicado a um agente e a um ecossistema | Dispositivo físico de autoridade para um simbionte transferível |
| Agente hospedado na infraestrutura do fornecedor | Núcleo e dados sob controle do usuário por padrão |
| Integrações e serviços do ecossistema | Host Adapters para sistemas autorizados diversos |
| Memória pessoal do agente | Memória de Skills, modelos de hospedeiro e histórico controlável |
| Aprovação de ações sensíveis | Autoridade física, revogável, graduada e auditável |
| Dependência de serviço remoto | Execução local-first, com comunicação externa mínima e explícita |

A comparação não afirma que o Muse seja inseguro. Ela define a razão pela qual o HERUS precisa existir como uma arquitetura diferente.

## Arquitetura-alvo

```text
┌──────────────────────────────────────────────────────────────┐
│ Pessoa                                                       │
│ decide escopo, confirma, cancela e revoga                   │
└─────────────────────────────┬────────────────────────────────┘
                              │ canal físico autenticado
┌─────────────────────────────▼────────────────────────────────┐
│ HERUS Pulse                                                 │
│ estado, háptica, confirmação, cancelamento e parada         │
└─────────────────────────────┬────────────────────────────────┘
                              │ autoridade revogável
┌─────────────────────────────▼────────────────────────────────┐
│ Sovereign Symbiont Core                                     │
│ identidade, memória local, Skills, modelo de host, plano    │
│ evidência, abstenção, orçamento e adaptação                 │
└─────────────────────────────┬────────────────────────────────┘
                              │ operações autorizadas
┌─────────────────────────────▼────────────────────────────────┐
│ Local Sovereign Gateway                                     │
│ permissões, sandbox, credenciais, minimização e ledger      │
└───────────────┬──────────────────────┬───────────────────────┘
                │                      │
       ┌────────▼────────┐    ┌────────▼────────┐
       │ Host Adapter A  │    │ Host Adapter B  │    ...
       │ software local  │    │ sistema remoto  │
       └─────────────────┘    └─────────────────┘
```

O **Symbiont Core** não deve receber credenciais brutas nem possuir autoridade irrestrita. O **Local Sovereign Gateway** deve separar raciocínio, dados, credenciais e execução. Essa separação é necessária para que uma falha no modelo não se transforme automaticamente em acesso total.

## O que o simbionte deverá fazer

O HERUS futuro deverá:

1. entrar em um hospedeiro somente depois de autorização verificável;
2. descobrir interfaces, estados e capacidades disponíveis;
3. construir um modelo local do hospedeiro sem copiar dados privados para fora;
4. observar ou receber uma tarefa autorizada;
5. representar a intenção e o efeito da tarefa, não apenas seus cliques;
6. aprender uma Skill finita com pré-condições, pós-condições e limites;
7. transferir a Skill para outro hospedeiro compatível;
8. adaptar o plano quando a interface mudar;
9. executar automaticamente ações de baixo risco dentro de um escopo definido;
10. pedir confirmação física para ações relevantes;
11. abster-se quando a observação não sustentar a decisão;
12. registrar a ação e permitir auditoria e reversão;
13. parar imediatamente quando a pessoa revogar a autoridade pelo pulso.

## Soberania por camadas

A soberania será avaliada em camadas separadas:

| Camada | Pergunta | Critério futuro |
|---|---|---|
| Dados | Onde ficam memória, traces e conteúdo privado? | Permanecem em armazenamento controlado pelo usuário |
| Modelo | Quem pode usar os modelos e memórias? | Isolamento local e autorização explícita |
| Credenciais | Quem vê senhas e tokens? | O Core não recebe credenciais brutas |
| Rede | O que pode sair? | Egress bloqueado por padrão e registrado |
| Execução | Quem pode alterar um host? | Gateway aplica escopo, orçamento e risco |
| Autoridade | Quem pode parar? | Pulso e revogação independente do agente |
| Auditoria | Como saber o que ocorreu? | Ledger local, rederivável e exportável pelo usuário |
| Portabilidade | O simbionte depende de um fornecedor? | Skills e dados em formatos controláveis e transferíveis |

## Primeiro produto que deve ser construído

O primeiro produto do novo caminho não será um agente que controla qualquer software. Será um **HERUS Sovereign Bridge** local, formado por:

- um Symbiont Core executando localmente;
- um gateway local com sandbox;
- dois hosts de demonstração com interfaces diferentes;
- uma Skill transferível entre os hosts;
- um software de pulso com eventos de estado;
- confirmação tátil simulada antes da execução relevante;
- revogação e parada independentes;
- um ledger local que prove quais dados permaneceram locais;
- testes negativos de exfiltração, acesso fora do escopo e autoridade excessiva.

A primeira demonstração forte será:

> **O usuário ensina uma rotina em um software. O HERUS aprende a intenção, remapeia a rotina em outro software, executa ações permitidas localmente e solicita confirmação física para a ação relevante, sem enviar a memória pessoal ou o transcript bruto para um serviço externo.**

## Gates da nova direção

A direção soberana só poderá ser promovida de objetivo para capacidade demonstrada quando passar por estes gates:

### Gate S1 — Armazenamento local

Memória, Skills, traces e configurações permanecem em armazenamento local controlado pelo usuário. O teste deve detectar qualquer escrita externa inesperada.

### Gate S2 — Egress negativo

Com a rede externa bloqueada, o núcleo ainda deve descobrir, aprender, propor, confirmar, cancelar e revogar no ambiente local. Qualquer tentativa de envio deve ser registrada e bloqueada.

### Gate S3 — Separação de credenciais

O Core deve operar sem receber senhas ou tokens em texto claro. O gateway deve executar operações autorizadas sem revelar o segredo ao raciocinador.

### Gate S4 — Transferência entre hosts

A mesma Skill deve ser remapeada entre dois hosts com interfaces diferentes. O resultado deve ser comparado a um baseline manual e a uma condição sem Symbiont.

### Gate S5 — Autoridade física

O pulso deve confirmar, cancelar e interromper a ação por um canal independente do processo que está executando a Skill. O processo não pode ignorar a parada.

### Gate S6 — Não promoção indevida

O sistema deve bloquear a ação quando houver efeito oculto, capacidade ausente, escopo vencido, orçamento esgotado, trace incompleto ou evidência insuficiente.

### Gate S7 — Auditoria externa

Outra pessoa deve conseguir instalar o protótipo, reproduzir os testes de soberania e verificar os resultados sem confiar apenas na palavra do autor.

## O que muda no roadmap

A sequência oficial passa a ser:

```text
reproduzir
  → provar soberania local
  → separar Core, gateway e host
  → construir dois hosts reais controlados
  → transferir uma Skill
  → integrar pulso e revogação
  → medir compreensão humana
  → medir valor comparativo
  → avançar para hardware e múltiplos ambientes
```

Hardware, voz, nuvem e modelos maiores não serão usados como substitutos para os gates de soberania. A expansão só será legítima depois que o HERUS demonstrar controle dos próprios dados e autoridade de execução.

## Estado atual e limite da promessa

O HERUS atual possui contratos de identidade, Skills finitas, fronteira black-box, abstenção, orçamento, campanha causal e um Bridge local de proposta e cancelamento. Ele ainda não possui o Sovereign Gateway completo, nem prova de que dados nunca saem em um ambiente operacional real.

Portanto, o estado correto continua sendo:

> **`host-only / mechanism candidate / not_proven`**

A soberania é agora o objetivo central do futuro do projeto. Ela só poderá ser chamada de capacidade do HERUS depois de passar por testes de egress, credenciais, armazenamento, revogação e auditoria externa.

## Referências

[1]: https://about.fb.com/news/2026/09/introducing-muse-personal-ai-agent/ "Introducing Muse: The World’s First Personal AI Agent Built for Everyone"
[2]: https://www.meta.com/muse-charm/ "Muse Charm — Meta"
[3]: https://www.reuters.com/business/meta-expected-unveil-smart-glasses-without-camera-privacy-concerns-grow-2026-09-23/ "Meta launches AI gadget Charm as race for post-smartphone hardware heats up"
[4]: https://www.theverge.com/tech/999944/meta-muse-charm-ai-interact-5g-modem "Meta’s Muse AI Charms can interact with each other"
