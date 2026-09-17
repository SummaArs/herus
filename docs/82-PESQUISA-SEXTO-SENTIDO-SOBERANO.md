# HERUS como sexto sentido soberano

## Conclusão executiva

A visão do HERUS como uma interface corporal pela qual uma pessoa controla sistemas tecnológicos é tecnicamente plausível em partes, mas não existe hoje como uma infraestrutura geral pronta. Já existem componentes separados: wearables hápticos, padrões de interoperabilidade, protocolos de robótica, segurança zero trust e assistentes personalizados. O que falta é a composição soberana entre esses componentes.

A contribuição potencial do HERUS não é inventar um novo rádio ou substituir todos os protocolos. É colocar uma camada local, auditável e fail-closed entre a intenção humana, a capacidade descoberta de um sistema e o efeito autorizado. O pulso seria o canal corporal da pessoa; o HERUS seria o núcleo que traduz, verifica, aprende limites operacionais e conserva a autoridade humana.

A alegação correta neste estágio é:

> **HERUS é uma proposta de infraestrutura de coordenação pessoal que pode usar o pulso como interface corporal e adaptar-se a hospedeiros tecnológicos por meio de descrições de capacidade, contratos de risco, experiências locais e autorização explícita.**

Ainda não é correto afirmar que o pulso controla qualquer tecnologia. Isso precisa ser demonstrado por classes de hospedeiro e por testes de segurança e fatores humanos.

## O que já existe

A indústria já resolveu fragmentos do problema. Matter oferece interoperabilidade IP, conectividade local, comissionamento por Bluetooth LE e certificação para dispositivos compatíveis. Seu objetivo é permitir que dispositivos de várias marcas funcionem juntos com conectividade confiável e segura [1]. Isso é útil para o HERUS, mas Matter é um ecossistema específico, principalmente residencial; ele não define sozinho a semântica de intenção, a autoridade pessoal ou a prova de que uma ação é segura.

A arquitetura Web of Things do W3C é ainda mais diretamente relevante. Ela define descrições legíveis por máquina, affordances e intermediários que podem representar Things físicas ou compor Things virtuais. A arquitetura pode ser usada no dispositivo, em um gateway ou na nuvem [2]. O HERUS pode consumir Thing Descriptions para descobrir capacidades, mas deve convertê-las para o HIR finito e submetê-las ao seu próprio verificador. Uma affordance descreve como algo pode ser usado; ela não autoriza que uma pessoa específica use aquilo em qualquer contexto.

Na robótica, ROS 2 já fornece autenticação, criptografia, integridade e controle de acesso por meio de DDS-Security. Seus enclaves possuem identidade, chaves e permissões [3]. Porém, a própria documentação informa que a segurança depende de configuração e pode permanecer desligada por padrão. O adaptador HERUS deverá exigir configuração segura e falhar quando a política estiver ausente. Segurança de transporte não pode ser confundida com autorização semântica.

No campo de segurança, o NIST define zero trust como ausência de confiança implícita baseada na posição de rede ou na propriedade do dispositivo. Autenticação e autorização são funções distintas [4]. Essa separação coincide com o núcleo constitucional do HERUS: descobrir capacidade não concede permissão; identificar um dispositivo não autoriza um efeito; uma proposta não é execução.

Na interface corporal, revisões de háptica mostram que vibração e outros estímulos vestíveis conseguem representar propriedades de objetos e estados de interação, mas a tecnologia exige testes com usuários e, em alguns casos, treinamento para interpretação [5]. Portanto, um código LRA não pode ser inventado apenas por conveniência de implementação. O HERUS terá de medir confusão, tempo de reconhecimento, fadiga e erro sob distração.

## O que os usuários tendem a esperar

A adoção de wearables não depende apenas de capacidade técnica. Uma revisão sobre aceitação de dispositivos vestíveis IoT encontrou relação entre intenção de uso, risco percebido de privacidade, consciência de saúde, confiança e normas sociais [6]. Isso significa que o HERUS não será desejável por ser complexo. Ele deverá produzir benefício imediato e reduzir a ansiedade de controle.

Usuários provavelmente esperam cinco propriedades. Primeiro, querem saber o que o dispositivo está fazendo. Segundo, querem poder interrompê-lo. Terceiro, esperam que os dados íntimos permaneçam sob seu controle. Quarto, esperam que os alertas sejam raros e importantes. Quinto, esperam que o sistema continue útil quando a Internet falhar.

O pulso não pode transformar cada evento em vibração. Estudos sobre notificações de smartphones observaram respostas mais lentas quando sons associados a notificações eram apresentados, além de efeitos em componentes ligados à atenção e ao controle cognitivo [7]. A interface corporal deve proteger a atenção, não criar uma nova fonte de interrupção permanente.

Pesquisa recente sobre assistentes personalizados de privacidade mostra uma tensão central. Pessoas podem aceitar delegar decisões repetitivas, mas também temem perder controle e sofrer interferência de um agente que decide em seu nome. O estudo identificou fatores de design, regulação, educação e estrutura de mercado, incluindo preocupação com monopólio [8]. Para o HERUS, soberania não deve significar apenas “dados locais”; deve significar possibilidade real de inspeção, exportação, revogação e substituição do núcleo.

## O significado de “sexto sentido” para o HERUS

O termo deve ser usado de forma operacional, não mística. O sexto sentido do HERUS seria uma camada que percebe estados tecnológicos relevantes e os traduz em sinais humanos curtos, verificáveis e acionáveis.

O ciclo seria:

```text
sistema externo
  → estado observado
  → capacidade e risco
  → intenção humana
  → representação canônica
  → verificação
  → feedback no pulso
  → autorização explícita
  → ação limitada
  → registro auditável
```

O pulso não deve ser o lugar onde toda a inteligência reside. Ele é a interface corporal e a âncora de interação da pessoa. O núcleo pode existir em camadas: um runtime local no pulso, um hospedeiro auxiliar no telefone ou computador e adaptadores para sistemas externos. O pulso continua soberano no sentido de que nenhuma ação sensível deve ocorrer apenas porque um sistema remoto a propôs.

## Arquitetura recomendada

A arquitetura deve ter seis planos separados.

| Plano | Responsabilidade | Não pode fazer sozinho |
|---|---|---|
| Corpo | LRA, botão, tela mínima, voz local e confirmação | conceder autoridade implícita |
| Núcleo | identidade, HIR, memória seletiva e políticas | relaxar verificadores |
| Descoberta | capacidade, versão, latência, recursos e limites | transformar descrição em permissão |
| Interoperabilidade | WoT, Matter, BLE, ROS 2, USB, APIs e bridges | esconder o protocolo real |
| Assurance | risco, pré-condições, expiração, revogação e logs | aceitar conflito como verdade |
| Aprendizado | custo, utilidade, deriva, energia e escolha de Skill | aprender autorização ou identidade |

A camada de interoperabilidade deve ser plural. Matter é adequado para um subconjunto de casa conectada; WoT é adequado para descrições e affordances; ROS 2 é adequado para robótica; BLE, USB e APIs são transportes. Nenhum deles deve ser o núcleo semântico do HERUS.

## Requisitos de soberania

O HERUS só merece ser chamado de soberano se cumprir requisitos observáveis:

1. **Local-first:** o caminho básico funciona sem conta obrigatória, nuvem obrigatória ou conexão contínua.
2. **Memória exportável:** a pessoa consegue exportar eventos, Skills, políticas e proveniência em formato aberto.
3. **Revogação física:** existe uma ação física que coloca o sistema em estado seguro e impede efeitos.
4. **Logs legíveis:** toda proposta informa origem, hospedeiro, Skill, pré-condições, confiança e motivo da abstenção.
5. **Separação de planos:** dados de sensor, experiência, identidade e autoridade não são um mesmo objeto.
6. **Sem atualização silenciosa:** mudanças de Skill e firmware têm versão, digest e rollback.
7. **Interoperabilidade substituível:** a pessoa pode trocar o adaptador, o computador auxiliar ou o serviço sem perder sua identidade e seus registros.
8. **Falha positiva:** bateria baixa, perda de rádio, conflito ou sensor incerto geram abstenção, não improviso.

O estudo do NIST sobre dispositivos móveis e vestíveis para primeiros respondedores reforça que wearables ampliam operações, mas também introduzem novas vulnerabilidades no ambiente de trabalho [9]. Um “sexto sentido” em contexto crítico deve ser tratado como superfície de segurança, não apenas como UX.

## O que o HERUS deve aprender

O aprendizado deve ser limitado a propriedades operacionais:

```text
latência por hospedeiro
consumo de energia
pico de memória
taxa de falha
confiabilidade de interface
utilidade observada
deriva de capacidade
confusão do código háptico
```

O HERUS não deve aprender por estatística que:

```text
uma pessoa autorizou algo
um dispositivo é confiável por estar próximo
uma assinatura pode ser ignorada
um conflito é aceitável
um efeito físico é seguro
uma resposta da Internet é verdadeira
```

Essas propriedades precisam de verificadores, vínculos humanos e contratos externos.

## O que deve ser incorporado ao HERUS

| Decisão | Item | Motivo |
|---|---|---|
| Incorporar | Thing Descriptions e affordances do W3C WoT | descoberta de capacidades e composição de Things |
| Incorporar | modelo zero trust do NIST | eliminar confiança implícita por rede ou proximidade |
| Incorporar | enclaves e políticas de ROS 2 | adaptador seguro para robótica |
| Incorporar | Matter como adaptador | interoperabilidade local em casas compatíveis |
| Incorporar | LRA com vocabulário finito | feedback corporal de baixa largura de banda |
| Testar | códigos hápticos de 3 a 5 estados | medir reconhecimento e confusão antes de ampliar repertório |
| Testar | controle pelo pulso em shadow mode | provar descoberta e proposta sem efeito físico |
| Testar | consentimento por risco | reservar confirmação forte para efeitos importantes |
| Observar | agentes de linguagem locais | somente depois de medir memória, energia e erro no hospedeiro |
| Rejeitar | nuvem como autoridade | viola soberania e cria ponto único de dependência |
| Rejeitar | “qualquer dispositivo” sem adaptador verificável | promessa não falsificável |
| Rejeitar | treinamento irrestrito com dados pessoais | aumenta superfície de vazamento e não prova simbiose |
| Rejeitar | vibração para cada evento | ameaça atenção e habituação |

## Programa experimental

O próximo programa deve ser orientado por cenários, não por acumulação de módulos.

### Cenário A — casa local

O HERUS descobre uma lâmpada Matter e uma tomada. Ele mostra as capacidades, propõe uma ação e exige confirmação física. O teste mede tempo, erro, replay, perda de rede e revogação.

### Cenário B — computador e filesystem

O HERUS controla apenas um workspace de teste. Ele precisa sobreviver a crash, rollback, mudança de versão e conflito de política. Nenhuma ação deve alcançar arquivos fora da sandbox.

### Cenário C — robô simulado e ROS 2

O HERUS recebe um grafo com sensores e atuadores. Ele deve distinguir observação de comando, detectar permissões ausentes e bloquear o atuador quando o enclave estiver inseguro.

### Cenário D — pulso

O usuário aprende um vocabulário LRA pequeno. O experimento mede reconhecimento, falsos alarmes, tempo de resposta e fadiga em tarefas com distração. A ação de alto risco exige uma confirmação diferente de uma informação passiva.

### Cenário E — migração

A mesma identidade passa de computador para pulso e retorna. Memória operacional compatível pode acompanhar o handoff; autoridade, contexto privado e leases antigos não podem.

## Critério para uma alegação forte de simbiose

O HERUS poderá fazer uma alegação forte somente quando o mesmo núcleo demonstrar, em pelo menos três classes independentes de hospedeiro, que:

1. descobre capacidades sem receber o perfil verdadeiro;
2. escolhe uma representação que cabe no orçamento;
3. troca experiências úteis sem transportar contexto privado;
4. detecta deriva e expira conhecimento velho;
5. bloqueia conflitos e permissões ausentes;
6. continua seguro quando um hospedeiro falha;
7. oferece feedback compreensível à pessoa;
8. mantém a mesma identidade sem tornar-se autoridade global;
9. melhora uma métrica não vista sem aumentar falsos aceites;
10. produz logs reproduzíveis para auditoria independente.

Esse critério é mais forte que “o pulso controla um dispositivo”. Ele testa se o corpo, a tecnologia e a pessoa formam um sistema coordenado sem que o núcleo capture a autoridade humana.

## Veredito

A ideia é relevante e diferenciada, mas o diferencial não é simplesmente colocar uma IA em um relógio. Já existem smartwatches, assistentes, padrões de IoT, plataformas de robótica e sistemas de segurança. O espaço pouco resolvido é a combinação entre **interface corporal, descoberta multi-hospedeiro, aprendizado operacional bounded, soberania local e assurance verificável**.

O maior risco de produto é o HERUS virar apenas uma camada técnica que ninguém entende. O maior risco científico é chamar de simbiose uma sequência de integrações sem medir transferência, falha, conflito e fatores humanos. O maior risco de segurança é o pulso tornar-se uma chave universal capaz de produzir efeitos sem confirmação contextual.

A direção correta é construir um sistema que seja simples para a pessoa e rigoroso por baixo: o pulso mostra poucos sinais; o núcleo explica a origem; o adaptador conhece o protocolo; o verificador decide se a ação pode sequer ser proposta; e a pessoa mantém a última autoridade.

## Referências

[1]: https://csa-iot.org/all-solutions/matter/ "Matter Smart Home Standard — Connectivity Standards Alliance"
[2]: https://www.w3.org/TR/wot-architecture11/ "Web of Things (WoT) Architecture 1.1 — W3C Recommendation"
[3]: https://docs.ros.org/en/humble/Concepts/Intermediate/About-Security.html "ROS 2 Security — ROS 2 Documentation"
[4]: https://csrc.nist.gov/pubs/sp/800/207/final "NIST SP 800-207 Zero Trust Architecture"
[5]: https://pmc.ncbi.nlm.nih.gov/articles/PMC9919508/ "An Overview of Wearable Haptic Technologies and Their Performance in Virtual Object Exploration"
[6]: https://www.tandfonline.com/doi/full/10.1080/23311916.2022.2087456 "Determinants of User Acceptance of Wearable IoT Devices"
[7]: https://pmc.ncbi.nlm.nih.gov/articles/PMC9671478/ "The Hidden Cost of a Smartphone: Effects of Notifications on Cognitive Control"
[8]: https://arxiv.org/pdf/2509.08554 "Acceptability of AI Assistants for Privacy: Perceptions of Experts and Users"
[9]: https://www.nist.gov/publications/security-guidance-first-responder-mobile-and-wearable-devices "Security Guidance for First Responder Mobile and Wearable Devices"
[10]: https://pmc.ncbi.nlm.nih.gov/articles/PMC12167361/ "Privacy in Consumer Wearable Technologies: A Living Systematic Analysis"
