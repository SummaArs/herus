# HERUS indispensável: relógio, memória e inteligência própria

**Status:** revisão estratégica e técnica para adoção cotidiana; esta PR adiciona contratos host-only e não é anúncio de produto pronto.
**Objetivo:** transformar o HERUS em uma presença diária útil, desejável e soberana, sem depender de LLMs hospedadas de terceiros.

## 1. Tese do produto

O HERUS não deve tentar ser apenas outro gravador, relógio esportivo, aplicativo de notas ou rádio de emergência. A oportunidade está na combinação de três coisas que normalmente ficam separadas:

> **Um relógio que conversa com você, lembra apenas o que você escolheu preservar e recorre ao seu Paper-Core quando precisa de mais contexto — sem entregar a sua vida inteira a uma nuvem.**

A promessa de “segundo cérebro” precisa ser operacionalizada. O HERUS não absorve a vida inteira. Ele constrói uma coleção seletiva de **cartões de contexto**: decisões, ideias, compromissos, relações entre conceitos, instruções e estados que a pessoa quer reencontrar. O áudio, a transcrição, o embedding e a observação bruta são transitórios e não são o produto final.

O produto deve ter valor mesmo quando a pessoa está em casa, na universidade, no trabalho ou em deslocamento. Comunicação remota, rede por proximidade e modos off-grid são extensões poderosas; não podem ser a única razão para comprar o relógio.

## 2. A brecha de mercado

Wearables já são uma categoria cotidiana, mas a maior parte deles se concentra em saúde, notificações, fitness e captura ampla. A pesquisa publicada em *npj Digital Medicine* observou que a adoção de wearables cresce junto com riscos recorrentes de transparência, minimização, privacidade por padrão, retenção e eliminação em políticas de fabricantes [1]. Isso cria espaço para um produto cuja diferença seja **controle verificável**, não apenas um novo sensor.

O reMarkable Paper Pro demonstra uma proposta clara de produto: escrita, leitura, organização, busca, conversão de manuscrito, aplicativos e foco; a página do fabricante declara até 14 dias de uso regular, 2 GB de RAM e 64 GB de armazenamento [2]. O aprendizado para o HERUS é que o Paper-Core precisa ser útil por si só.

A PLAUD demonstra que existe demanda adjacente por captura e organização assistida por IA, oferecendo gravação, transcrição, resumos, busca e respostas referenciadas [3]. O HERUS não deve vencê-la capturando mais. Deve diferenciar-se por **selecionar antes de persistir**, por operar localmente e por manter a autoridade física separada da inteligência.

Sinais de adoção de smart glasses, wearables e dispositivos de IA mostram que a categoria de computação pessoal vestível está em expansão [4] [5] [6]. Nenhum desses sinais prova que o HERUS será uma febre. Eles apenas tornam plausível testar uma proposta nova. O resultado comercial dependerá de uso repetido, conforto, preço, privacidade percebida e de uma experiência que seja útil no primeiro dia.

## 3. O produto completo

| Produto | Função principal | Experiência do usuário |
|---|---|---|
| **HERUS Watch** | Conversa curta, memória rápida, háptica, confirmação, wake word e comunicação | Sempre disponível no pulso, sem exigir tela para cada ação |
| **HERUS Paper-Core** | Caderno digital, leitura, estudo, LLM local maior, cofre, atualização e recarga | Uma estação única: papel digital + inteligência + memória |
| **HERUS Core Mini** | Cache de memória, comandos, filtro de voz e diálogo curto no Watch | O relógio continua útil longe do Paper |
| **Aplicativo local** | Acessibilidade, configuração, exportação, importação e diagnóstico | Complementa; não é a autoridade final |
| **Rede HERUS** | Convite bilateral por proximidade e comunicação sem infraestrutura quando disponível | Compartilha cartões e estados, não expõe a vida inteira |

O Paper-Core deve ser uma coisa só na experiência, mas pode usar um módulo circular removível na traseira. O módulo concentra computação, armazenamento protegido, rádio e bateria auxiliar; o Paper oferece escrita, leitura e revisão. Essa separação melhora reparabilidade e permite que o Watch seja vendido e usado sozinho.

## 4. O que torna o relógio indispensável

O HERUS precisa entrar na rotina por três loops simples:

### Lembrar

A pessoa pressiona o Watch, fala ou escreve uma intenção curta. A inteligência apresenta um candidato: “parece uma decisão”, “há um compromisso”, “isso se relaciona a um cartão anterior”. A pessoa revisa e confirma. Depois, reencontra o cartão por uma consulta tipada.

### Conversar

O usuário pergunta ao Watch. O Core Mini responde com comandos e contexto pequeno. Para uma pergunta mais profunda, o Watch consulta o Paper-Core. A resposta mostra se veio de conhecimento de fábrica, memória pessoal, uma anotação do Paper ou uma inferência. A LLM não pode transformar uma resposta em memória sem confirmação.

### Conectar

O usuário seleciona um cartão ou estado. Dois Watches iniciam um convite por proximidade. Cada lado vê o destinatário, o conteúdo, o prazo e o escopo. Ambos confirmam. O toque inicia a negociação; não concede confiança permanente. O vínculo pode ser revogado e expirar.

## 5. Arquitetura própria de inteligência

A exigência de não usar LLMs de terceiros deve ser tratada com honestidade. A PR não afirma que o HERUS já treinou uma LLM fundacional própria. Isso exigiria grandes dados, infraestrutura, avaliação e uma política de licenciamento que ainda não existem. Em vez de prometer uma impossibilidade, a arquitetura define uma **tecnologia própria do HERUS** em torno de modelos locais:

| Componente | Pode ser proprietário do HERUS desde cedo? | Função |
|---|---|---|
| Gramática semântica e HCP | sim | significados, cartões e estados compactos |
| Política de memória | sim | relevância, expiração, sensibilidade e revisão |
| Compilador de proposta | sim | modelo não confiável → saída tipada |
| Front-end Watch | sim | sessão, VAD/wake/locutor, candidato e confirmação |
| Knowledge pack | sim | vocabulário, procedimentos e fatos de fábrica versionados |
| Modelos pequenos | treináveis/quantizáveis pelo projeto | wake word, VAD, comandos, relevância e filtro de voz |
| Modelo conversacional maior | futuro, local no Paper-Core | respostas, síntese e consulta de contexto |
| API de LLM hospedada | **não usar** | removida da arquitetura do produto |

O HERUS pode inspirar-se em state of the art, mas a contribuição própria deve estar no sistema: representação semântica, privacidade, política de memória, interação, proveniência, atualização e segurança. Um modelo local aberto eventualmente usado para bootstrap deve ser declarado, auditado e substituído ou licenciado de forma explícita; ele não deve ser apresentado como uma LLM proprietária do HERUS.

A documentação Espressif mostra um caminho realista para inteligência no ESP32-S3: WakeNet para wake word e MultiNet para comandos offline leves, com customização e saída tipada de comando [7] [8]. LiteRT para microcontroladores reforça a estratégia de modelos pequenos convertidos para arrays locais, com operações limitadas e sem treinamento no dispositivo [9].

## 6. Conhecimento de fábrica e memória pessoal

O conhecimento deve ser separado em três pacotes:

| Pacote | Exemplo | Segurança e ciclo de vida |
|---|---|---|
| **Modelo base** | classificador ou modelo conversacional local | digest, versão, orçamento, regressão e rollback |
| **Conhecimento de fábrica** | vocabulário, FAQs, procedimentos, glossários e índices | pacote autenticado, substituível e sem autoridade sobre gates |
| **Memória pessoal** | decisões, ideias e compromissos autorizados | cofre local, origem, expiração, revisão e revogação |

Quando o Watch não souber algo, ele consulta o Paper-Core. “Alimentar o conhecimento” significa instalar um pacote validado ou criar um cartão pessoal confirmado; não significa alterar pesos silenciosamente com dados da vida da pessoa.

O gate `model_lab` existente exige medição de alvo, digest, localidade, orçamento de memória, latência, energia, casos funcionais, casos adversariais, zero tentativas de rede e zero tentativas de autoridade. A nova direção usa esse contrato como barreira para qualquer modelo futuro.

## 7. Voz pessoal sem vigilância

O Watch pode permanecer em um modo de baixo consumo que observa apenas VAD e wake word. Um filtro de locutor estima se a voz parece pertencer ao usuário. A pesquisa da Apple mostra que verificação local de locutor pode ser aprimorada com aprendizagem federada e privacidade, mas o ganho relatado pertence àquele experimento e não é uma métrica do HERUS [10].

A voz deve ser tratada como filtro de conveniência, não como identidade perfeita. A cadeia correta é:

```text
VAD → wake word → score de locutor → janela transitória →
proposta de relevância → revisão → confirmação física → cartão
```

O Watch precisa de mute físico, indicador claro de escuta, modo sem voz, exclusão do perfil acústico e apagamento transitório. Se não houver wake word, score suficiente, relevância ou confirmação, nada vira memória. Uma voz clonada, ruído, pessoa próxima ou erro de modelo não pode desbloquear ação sensível.

## 8. Caso de uso cotidiano principal

Uma estudante lê no Paper e percebe que confunde dois conceitos. Ela pressiona o Watch e diz: “guardar para revisar antes da prova: diferença entre autenticação e autorização”. O Watch não salva a aula nem a fala inteira. Ele cria uma proposta tipada. O Paper mostra o cartão, a estudante corrige e confirma. Na véspera da prova, pergunta “o que preciso revisar?” e recebe o cartão original, uma ausência ou uma sugestão marcada como sugestão.

A mesma experiência serve para um profissional preservar uma decisão de reunião, para uma família compartilhar “cheguei” com expiração ou para uma pessoa criativa guardar uma ideia. O produto é o mesmo; mudam os cartões e o vocabulário.

## 9. Segurança soberana

Soberania no HERUS não deve significar segredo social ou blockchain obrigatório. Significa que a pessoa controla chaves, memória, atualizações, exportação, exclusão e vínculos. A rede por toque é privada e bilateral, não clandestina. O blockchain pode ser estudado para interoperabilidade, mas não deve receber conteúdo privado nem tornar a memória irreversível.

Os não negociáveis permanecem:

- nenhum modelo cria HCP, persiste ou transmite;
- nenhuma memória pessoal é criada sem confirmação física;
- áudio, transcrição, embedding, identidade, localização e chaves não entram nos logs de produto;
- um pacote de modelo ou conhecimento não modifica gates de autoridade;
- o Watch degrada sem o Paper-Core e falha fechado sem sessão, vínculo ou verificação;
- o aplicativo não possui autoridade maior que o dispositivo físico;
- cada atualização tem digest, versão, compatibilidade e recuperação.

## 10. Roadmap para tentar criar desejo de massa

| Fase | Entrega | Prova necessária |
|---|---|---|
| 0 | front-end Watch host-only, como esta PR | abstention, expiração, mute e confirmação |
| 1 | Paper/Core protótipo de bancada + Watch simulado | fluxo de cartão e conversa local |
| 2 | dois Watches físicos e Core Mini | vínculo, háptica, bateria e sincronização |
| 3 | modelo pequeno próprio para wake/comandos/relevância | benchmark, digest e zero autoridade |
| 4 | Paper-Core físico | escrita, busca, cofre, atualização e recuperação |
| 5 | piloto diário com estudantes/profissionais/famílias | uso repetido, retenção útil, falsos positivos e confiança |
| 6 | produto premium e rede consentida | conforto, fabricação, segurança, suporte e interoperabilidade |

A palavra “indispensável” só poderá ser usada após uso longitudinal. Precisaremos medir se a pessoa volta ao produto, se reencontra cartões importantes, se entende o consentimento, se usa o Paper sem o Watch e se mantém o Watch no pulso sem desconforto. Uma boa demonstração ou um modelo que responde uma pergunta não prova adoção.

## 11. O incremento desta PR

Esta PR implementa `watch_memory_frontend.{h,c}` e uma suíte C11 host-only. O front-end recebe somente uma observação tipada de um futuro adapter local; não recebe áudio, texto, embedding ou prompt.

Ele exige sessão de captura não nula, rejeita sessão estrangeira, valores não canônicos, enums inválidos e scores fora de faixa, abstém para mute, ausência de wake, locutor baixo, locutor ambíguo e baixa relevância, mantém candidato transitório, expira sem retenção, permite rejeição e só emite `memory_signal_t` com `explicit_remember == 1` depois de confirmação.

O módulo não implementa ASR, VAD ou LLM real. Isso é deliberado: o contrato pode ser testado agora e receber modelos próprios depois. O alvo é `make -C firmware watch-memory-frontend`, fora de `all`.

## Veredito

A melhor chance de o HERUS se tornar desejado não é prometer uma LLM mágica dentro de um relógio. É entregar uma experiência que as pessoas sintam todos os dias: **perguntar, guardar o essencial, reencontrar no momento certo e compartilhar apenas o necessário**.

A bomba da LLM aconteceu porque o software resolveu tarefas imediatamente compreensíveis. O HERUS precisa fazer o mesmo com uma frase simples:

> **“O HERUS lembra o que importa para você, conversa com você no pulso e nunca decide sozinho o que pode fazer com a sua vida.”**

Isso é uma tese de produto. O mercado, a bateria, o conforto, o alcance, a voz e a qualidade da IA ainda precisam provar se ela pode virar um produto real.

## Referências

[1] [Doherty et al., Privacy in consumer wearable technologies](https://pmc.ncbi.nlm.nih.gov/articles/PMC12167361/)

[2] [reMarkable Paper Pro — Features and specifications](https://remarkable.com/products/remarkable-paper/pro/details/features)

[3] [PLAUD Note — AI voice recorder](https://www.plaud.ai/products/plaud-note-ai-voice-recorder)

[4] [EssilorLuxottica — Q4/Full Year 2025 Results](https://www.essilorluxottica.com/en/newsroom/press-releases/q4-full-year-2025-results/)

[5] [IDC — Wearable Devices Market Insights](https://www.idc.com/promo/wearablevendor/)

[6] [Rock Health — 2025 Consumer Adoption Survey](https://rockhealth.com/insights/whats-your-score-insights-on-wearables-and-connected-devices-from-rock-healths-2025-consumer-adoption-survey/)

[7] [Espressif ESP-SR — WakeNet Wake Word Model](https://docs.espressif.com/projects/esp-sr/en/latest/esp32s3/wake_word_engine/README.html)

[8] [Espressif ESP-SR — MultiNet Command Word Recognition](https://docs.espressif.com/projects/esp-sr/en/latest/esp32s3/speech_command_recognition/README.html)

[9] [Google AI Edge — LiteRT for Microcontrollers](https://www.tensorflow.org/lite/microcontrollers)

[10] [Apple Machine Learning Research — Improving On-Device Speaker Verification Using Federated Learning With Privacy](https://machinelearning.apple.com/research/improving-on-device-speaker)
