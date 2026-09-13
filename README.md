# HERUS

**Memória pessoal soberana, conversa local e comunicação essencial — com significado antes de mensagem.**

O HERUS é um sistema pessoal para **perguntar, guardar, reencontrar e transmitir o essencial** em situações nas quais um telefone, uma rede móvel ou uma interface densa não são a melhor ferramenta. No relógio, ele oferece interação curta, confirmação física e memória seletiva. No Paper-Core, concentra escrita, estudo, conhecimento local e memória. Em vez de depender de uma LLM hospedada ou transmitir áudio e longas mensagens, trabalha com **significados essenciais, cartões de contexto, representações finitas e autoridade humana explícita**.

O produto nasce para o cotidiano, para trilhas, áreas rurais, deslocamentos, equipes de apoio e situações de baixa atenção visual. Seu propósito não é substituir o telefone nem prometer uma inteligência autônoma. É preservar comunicação, memória e coordenação humana quando conectividade, privacidade, energia ou atenção são limitadas.

> **O HERUS não confunde uma mensagem com seu significado, uma proposta com uma autorização ou uma conexão com confiança.**

## O que o HERUS é agora

O HERUS possui duas camadas inseparáveis.

A primeira é a **experiência pessoal**: um relógio ou outro dispositivo simples que ajuda a pessoa a perguntar, confirmar, lembrar e comunicar estados essenciais. O Paper-Core funciona como uma estação pessoal para escrita, estudo, conhecimento local e memória. A pessoa permanece no comando: uma sugestão pode ser gerada localmente, mas memória, transmissão ou ação externa exigem uma fronteira de confirmação definida pelo sistema.

A segunda é a **Adaptive Symbiotic Architecture (ASA)**. Ela é o núcleo computacional capaz de entrar em um hospedeiro, descobrir suas capacidades e limitações, negociar uma representação finita, aprender dentro de orçamento, propor Skills verificadas e operar somente dentro da autoridade comprovada. O hospedeiro pode ser o relógio, um celular, um notebook, um robô, um servidor, um sensor, uma rede ou um ambiente externo como a Internet.

Os cenários já implementados — memória pessoal, pulso, robótica, rede defensiva, servidores e finanças em sandbox — não são categorias fechadas do ASA. São ambientes de validação. A arquitetura não deve perguntar “qual modo pré-programado estou usando?”, mas sim:

> **Que ambiente encontrei? O que consigo observar? O que ainda não sei? Qual evidência falta? O que posso propor sem ultrapassar autoridade?**

## Princípios

| Princípio | Consequência prática |
|---|---|
| **Significado antes de mensagem** | O núcleo trabalha com intenções, estados e cartões de contexto essenciais, não com áudio bruto por padrão. |
| **Memória pessoal soberana** | A pessoa decide o que pode ser lembrado, recuperado, removido ou transmitido. |
| **Local e privado por padrão** | O funcionamento essencial não depende de conta, nuvem ou conectividade contínua. |
| **Pessoa no comando** | Inteligência pode sugerir, resumir e perguntar; não envia, compra, publica, altera ou aciona algo sozinha. |
| **Fail-closed** | Ambiguidade, digest inválido, deriva, replay, expiração ou autoridade ausente bloqueiam a progressão. |
| **Adaptação verificável** | O ASA pode aprender sobre um ambiente, mas precisa registrar evidência, orçamento, hipótese e motivo de abstinência. |
| **Conectar não significa confiar** | USB, BLE, LoRa, Wi‑Fi ou Internet transportam dados e propostas; nenhum canal concede autoridade automaticamente. |

## Como a adaptação funciona

O ciclo do ASA é comum a qualquer hospedeiro observável:

```text
observar o ambiente
        ↓
formular uma lacuna de conhecimento
        ↓
sondar ou pedir pesquisa a um hospedeiro auxiliar
        ↓
verificar proveniência, integridade e escopo
        ↓
negociar uma representação finita
        ↓
compor ou sintetizar uma Skill limitada
        ↓
verificar em casos visíveis, ocultos ou sandbox
        ↓
propor uma ação
        ↓
executar somente sob autoridade comprovada
```

Quando o ASA não sabe algo, ele pode usar um celular ou notebook como **hospedeiro auxiliar**. A Internet pode fornecer evidência externa, mas é tratada como ambiente não confiável. O gateway de conhecimento exige HTTPS, proveniência, data, identidade do pesquisador auxiliar, claims explícitos e digest verificável. Ele não executa código baixado, não transforma pesquisa em permissão e não envia segredos ou autoridade silenciosamente.

A adaptação não significa reprogramação arbitrária. Significa adquirir evidência, atualizar uma hipótese, sintetizar uma Skill dentro de orçamento e submetê-la a um verificador independente. Quando a prova falha, o resultado correto é pedir ajuda, permanecer em sandbox ou recusar.

## Produto e arquitetura

| Elemento | Papel atual |
|---|---|
| **Relógio / wearable** | Interface imediata de gesto, confirmação, estado de privacidade, memória e feedback háptico. |
| **Paper-Core** | Estação pessoal para escrita, estudo, conhecimento local e memória soberana. |
| **ASA Core** | Núcleo adaptativo que descobre hospedeiros, negocia representações, seleciona Skills e limita autoridade. |
| **Celular ou notebook** | Hospedeiro auxiliar para gateway, pesquisa, armazenamento ou computação delegada sob contrato. |
| **Robô ou dispositivo legado** | Primeiro exemplo de hospedeiro cuja capacidade deve ser descoberta antes de qualquer controle. |
| **Internet** | Ambiente externo de evidência e comunicação; nunca autoridade implícita. |

O feedback háptico é deliberadamente limitado. Um LRA pode comunicar confirmação, espera, recusa, bloqueio ou necessidade de atenção. Ele não concede autoridade nem executa uma intenção sozinho. O usuário deve conseguir distinguir “o HERUS detectou”, “o HERUS propôs”, “a ação foi recusada” e “uma confirmação física é necessária”.

## Evidência atual

A implementação host-only já contém contratos de memória, intenção, Skills, descoberta de hospedeiros, negociação de representação, deriva, quarentena, canais, guardião defensivo, feedback háptico, sandbox multi-domínio e gateway de conhecimento externo. No estado publicado mais recente, a suíte `make -C research test` passa com **248 testes**, com um skip preexistente.

Esses testes provam propriedades do software e cenários finitos. Eles não provam ainda alcance de rádio, consumo, latência física, qualidade de BLE, persistência após perda de energia, comportamento real do LRA, ergonomia, compreensão humana dos padrões, controle seguro de robôs ou desempenho de uma LLM local. Essas alegações dependem do hardware e dos gates de bancada.

## Próximo marco físico

O próximo passo é o primeiro hospedeiro físico, começando de forma deliberadamente restrita:

| Gate | Objetivo |
|---|---|
| **B1 — Identificação** | Confirmar placa, revisão, alimentação, pinagem e componentes sem inferir capacidades não verificadas. |
| **B2 — Boot** | Gravar o firmware mínimo, confirmar boot serial, reset, identidade e leitura local. |
| **B3–B5 — Descoberta** | Medir memória, latência, comunicação, perdas e capacidades realmente observáveis. |
| **B6–B8 — Haptics** | Integrar driver e LRA, testar padrões, consumo e reconhecimento humano. |
| **B9–B10 — Adaptação** | Repetir descoberta, deriva, renegociação e ponte com celular ou notebook. |

Motores, ações irreversíveis, contas financeiras e controle de sistemas críticos permanecem fora do primeiro gate. O sucesso não será apenas “conectar”; será recusar corretamente o que não pôde ser provado.

## Documentação principal

| Documento | Conteúdo |
|---|---|
| [Linha do tempo](docs/63-LINHA-DO-TEMPO.md) | Evolução do projeto, dos contratos locais à ASA e ao gate físico. |
| [Arquitetura ASA](docs/62-SYMBIOTIC-ARCHITECTURE.md) | Definição atual da Adaptive Symbiotic Architecture e seus limites. |
| [Especificação do sistema](docs/00-HERUS-MASTER.md) | Arquitetura geral, protocolo, segurança, energia e limites conhecidos. |
| [Visão do produto](docs/04-PRODUCT.md) | Propósito, proposta de valor e direção de produto. |
| [Memória seletiva](docs/17-MEMORIA-SELETIVA.md) | Política para lembrar ideias, decisões e contexto sem gravar a vida inteira. |
| [Arquitetura finita e linguagem](docs/48-ARQUITETURA-FINITA-E-LINGUAGEM.md) | Vocabulário finito, regras e papel futuro de uma LLM local. |
| [Skill Layer v1](research/evidence/skill_layer_v1/limits.md) | Síntese limitada, verificação independente e limites de autoridade. |
| [Descoberta de hospedeiros](docs/59-AUTONOMOUS-HOST-DISCOVERY.md) | Protocolo de sondagem sem perfil completo pré-fornecido. |
| [Teto host-only](docs/61-HOST-ONLY-CEILING.md) | O que foi demonstrado e o que exige hardware físico. |
| [Contrato háptico](docs/58-HAPTIC-CONTRACT.md) | Estados, padrões e limites da interface LRA. |
| [Entrada em hardware](research/evidence/hardware_entry_gate_v1.md) | Critérios normativos B1–B10. |
| [Lista de compra](research/evidence/hardware_purchase_list_v1.md) | Componentes do primeiro hospedeiro físico. |
| [Segurança](SECURITY.md) | O que os controles protegem hoje e o que depende da integração física. |

## Verificação local

```bash
./prove.sh --quiet
make -C firmware watch-memory-frontend
make -C research test
```

Os comandos acima exercitam contratos de software e pesquisa host-only. Um resultado positivo autoriza o início controlado da bancada; não constitui prova de desempenho físico, segurança de rádio, privacidade de sensores ou utilidade do produto em campo.

## O que o HERUS ainda não é

O HERUS ainda não é uma AGI, não resolve NLU/NLG aberto, não substitui modelos de linguagem atuais e não possui adaptação geral comprovada a qualquer dispositivo. VSA/HDC permanece restrito ao vocabulário finito dos cartões de contexto. A memória é governada por regras e estados tipados. Uma LLM local futura pode atuar como camada linguística, mas somente depois de orçamento medido em hardware real e sem autoridade sobre memória, rádio ou confirmação física.

A ambição é grande; as alegações permanecem proporcionais às evidências. O projeto avançará quando uma nova capacidade puder ser especificada, testada, auditada e, se necessário, recusada.

## Licença

Proprietary. Copyright © 2026 Gustavo Gonçalves. Todos os direitos reservados — veja [LICENSE](LICENSE).
