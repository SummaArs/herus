# Linha do tempo do HERUS

Esta linha do tempo organiza a evolução do HERUS por marcos técnicos e decisões de escopo. Ela distingue produto, arquitetura, pesquisa host-only e lacunas que só podem ser respondidas por hardware físico.

## 1. Fundamentos de produto e segurança

| Período | Marco | Resultado |
|---|---|---|
| Início do projeto | Memória pessoal, conversa local e comunicação essencial | O problema foi definido a partir de privacidade, baixa conectividade, atenção limitada e necessidade de significado curto. |
| Primeiros ciclos | Intent Compiler, Semantic IR, HCP, Loom, Babel e Aether | Intenções passaram a ter representação canônica, vocabulário controlado e fronteiras explícitas entre proposta e execução. |
| Primeiros gates | Fail-closed, autoridade humana, memória seletiva e cofre de cartões | O sistema deixou de tratar texto, modelo ou conectividade como autoridade implícita. |

## 2. Consolidação da memória pessoal

A segunda etapa concentrou-se no produto pessoal. Foram definidos captura consentida, extração conservadora de candidatos, cartões mínimos, consolidação humana, recuperação tipada, índice privado, persistência transacional, quarentena de boot e recuperação após interrupção. O objetivo não era gravar a vida inteira, mas permitir que a pessoa preservasse ideias, decisões e compromissos escolhidos.

A cadeia host-only encontrou e bloqueou casos de piso terminal, replay, contradição, expiração e recuperação indevida. Essa etapa estabeleceu a ideia de **memória pessoal soberana**: o núcleo pode ajudar a lembrar, mas não deve transformar toda experiência em registro nem inferir autorização a partir de uma sugestão.

## 3. Assurance e sistemas de risco crítico

Em seguida, o HERUS foi expandido para verificação e assurance. Foram criados inventários de sinks críticos, contratos de risco, certificados finitos, mapas de refinamento e testes adversariais para garantir que uma intenção não ultrapassasse seus limites no caminho até um efeito.

Essa etapa não transformou o HERUS em um sistema certificado de segurança crítica. Ela criou uma base host-only para registrar invariantes, bloquear divergências e indicar quais propriedades ainda dependem de placa, energia, sensores, cadeia de boot, persistência e operação real.

## 4. Camada do significado

Em setembro de 2026, a camada de significado foi consolidada com Loom, Babel e Aether. A direção deixou de ser apenas comunicação protegida e passou a tratar de representação canônica de intenções, contexto, permissões, compromissos e consequências.

Ao mesmo tempo, o escopo foi deliberadamente restringido. VSA/HDC ficou limitado ao vocabulário finito dos cartões de contexto. AMR, CCG, OWL2, ProbLog, ASP e TAG foram retirados do roadmap por resolverem linguagem aberta que não é necessária para a arquitetura atual. A memória seletiva permaneceu em regras e domínios finitos. Uma LLM local futura foi mantida apenas como camada linguística possível, condicionada a medições físicas de memória, energia e latência.

## 5. Skill Layer e composição verificável

A Skill Layer introduziu uma biblioteca append-only de habilidades, contratos tipados, composição sequencial, síntese enumerativa limitada e verificação independente. O laboratório passou a testar se uma capacidade proposta poderia ser encontrada dentro de um DSL finito, aprovada em casos ocultos e armazenada sem conceder autoridade.

O resultado demonstrou crescimento verificável em domínios fechados. Não demonstrou raciocínio aberto, compreensão geral, aprendizagem livre ou substituição de modelos de linguagem. Essa distinção passou a ser uma regra editorial do projeto.

## 6. Adaptive Symbiotic Architecture

A próxima mudança conceitual foi a definição do HERUS como **Adaptive Symbiotic Architecture**. O núcleo deixou de ser descrito apenas como um dispositivo pessoal e passou a ser visto como uma entidade computacional que pode habitar diferentes hospedeiros.

Foram implementados perfil finito de capacidades, negociação de orçamento e representação, seleção de Skills verificadas, observação de dados reais, contratos de risco por domínio, detecção de deriva, reconciliação de observações e gates de prontidão física. Robótica, servidores, finanças em sandbox e sistemas críticos passaram a ser ambientes de teste da mesma arquitetura, não produtos independentes.

## 7. Descoberta autônoma de hospedeiros

A campanha seguinte implementou sondagem ativa com orçamento, hipóteses incompletas, abstinência e detecção de mudança. O HERUS pode descobrir capacidades ocultas em hospedeiros virtuais finitos e recusar uma Skill quando a evidência é insuficiente.

Esse resultado fechou o **teto host-only**. A descoberta foi demonstrada em laboratório controlado, mas ainda não em uma placa com rádio, energia, memória, latência, ruído, falhas, reinicialização e deriva física. A próxima pergunta passou a exigir hardware real.

## 8. Pulso, defesa e canais

A interface LRA foi formalizada como canal físico de feedback, não como fonte de autoridade. Padrões finitos comunicam confirmação, atenção, espera, recusa, bloqueio e emergência proposta.

Depois foram definidos contratos para USB, BLE, LoRa, Wi‑Fi, UART, I2C, SPI, CAN e RS-485. O guardião defensivo passou a registrar identidades autorizadas, detectar deriva de capacidade, quarentenar identidades alteradas e preservar revogações. A rede Bluetooth foi delimitada como defensiva: descoberta autorizada, evidência, bloqueio e quarentena, nunca exploração de dispositivos de terceiros.

## 9. Sandbox multi-domínio

Cenários locais demonstraram a mesma fronteira em três ambientes: um robô legado limitado à simulação, um par Bluetooth cuja capacidade mudou e um observador financeiro restrito a sandbox. Em todos, a adaptação produz uma proposta, não uma execução automática.

Essa etapa validou transferência de política entre domínios sem afirmar que o HERUS controla um robô, opera uma conta financeira ou protege uma rede real.

## 10. Pesquisa externa e Internet como ambiente

A definição mais recente amplia o ASA para celular, notebook e Internet. Quando o núcleo não sabe algo, um hospedeiro auxiliar pode pesquisar e devolver evidências. O gateway exige HTTPS, proveniência, data, claims explícitos e digest verificável. A Internet é uma fonte externa de evidência candidata, nunca uma autoridade.

O ASA pode aprender no sentido operacional de atualizar hipóteses e propor Skills limitadas após verificação. Ele não deve executar código baixado, enviar segredos silenciosamente ou reprogramar-se sem orçamento e sandbox.

## 11. Próximo marco: primeiro hospedeiro físico

O próximo ciclo é o bring-up do primeiro hardware de pulso. A sequência será identificação visual, verificação de pinagem, alimentação, boot serial, memória, latência, comunicação, consumo, perdas, reinicialização e deriva. Somente após esses gates será integrado o LRA real e, depois, uma ponte controlada com celular ou notebook.

O resultado esperado não é uma demonstração teatral de “inteligência geral”. É uma medida reproduzível de que o núcleo consegue entrar em um corpo limitado, declarar o que descobriu, reconhecer o que não sabe, pedir auxílio a outro hospedeiro e permanecer dentro da autoridade humana.

## Estado atual

O HERUS é, ao mesmo tempo, um produto pessoal em definição e uma arquitetura experimental em validação. Seu produto é **memória pessoal soberana, conversa local e comunicação essencial**. Sua arquitetura é o ASA. Sua pesquisa tenta provar adaptação verificável sem esconder as lacunas físicas.

A linha de evolução continua obedecendo ao princípio:

> **Construir muito bem → estabilizar → observar → melhorar pontualmente → preservar o que funciona.**
