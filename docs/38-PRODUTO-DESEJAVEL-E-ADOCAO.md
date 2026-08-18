# HERUS — Produto desejável e estratégia de adoção

**Status:** proposta de produto para validação; não é previsão de vendas nem aprovação de hardware.  
**Revisão:** product-01  
**Data:** 17 de agosto de 2026

> **Tese:** o HERUS deve ser um objeto pessoal discreto que ajuda a pessoa a **guardar o essencial, encontrar o que guardou e se conectar quando a rede falha**, sem depender de conta, nuvem ou atenção constante.

## 1. A pergunta certa

Não podemos fabricar uma “febre” por decisão de engenharia. Podemos, porém, projetar um produto com maior probabilidade de ser desejado: valor individual antes do efeito de rede, ritual simples, estética aceitável, utilidade frequente, privacidade demonstrável e uma razão natural para convidar outra pessoa.

A categoria mostra que existe espaço comercial para wearables e assistentes pessoais, mas não garante que o HERUS será adotado. A EssilorLuxottica informou mais de 7 milhões de óculos com IA vendidos em 2025 [1]. A IDC projetou 13,6 milhões de smart glasses em 2026, com crescimento anual de 41,4%, enquanto previu pressão de curto prazo sobre smartwatches [2]. A Oura divulgou uma base próxima de cinco milhões de membros pagantes [3] e mantém uma assinatura de US$5,99 por mês nos Estados Unidos [4]. A PLAUD vende um dispositivo de captura e sumarização por US$159 [5].

Esses sinais sustentam apenas uma conclusão limitada: pessoas pagam por **conveniência, interpretação pessoal, saúde, notas e contexto**. O HERUS ainda precisará provar que a memória seletiva, o consentimento físico e a operação off-grid produzem valor superior à simplicidade do celular.

## 2. Reposicionamento

A apresentação anterior começava por comunicação LoRa e depois chegava à memória pessoal. Para adoção ampla, a ordem deve ser invertida. A comunicação offline, a linguagem privada e o Núcleo continuam sendo diferenciais técnicos, mas o motivo inicial de compra deve ser individual.

A frase do produto passa a ser:

> **“O HERUS ajuda você a guardar somente o que realmente importa e a encontrar isso depois — com controle físico e sem depender da nuvem.”**

A comunicação sem infraestrutura é o segundo motivo:

> **“Quando a rede falha, o HERUS ainda permite enviar intenções essenciais, de forma curta, privada e confirmada.”**

O HERUS não deve ser apresentado como uma LLM vestível, um gravador de vida ou um rastreador. Esses enquadramentos aumentam expectativa, medo e risco regulatório. O produto é uma **memória complementar seletiva com comunicação essencial**.

## 3. O produto de entrada: HERUS One

O primeiro produto destinado a adoção ampla deve ser mais simples do que o protótipo técnico de bancada. Ele deve ser um token de pulso discreto, confortável e sem tela obrigatória. O dispositivo não precisa demonstrar toda a inteligência no corpo; precisa oferecer um gesto confiável, uma resposta háptica clara e um estado de privacidade compreensível.

| Elemento | Decisão de produto |
|---|---|
| **Botão físico** | Pressão curta consulta ou confirma conforme o estado; pressão longa inicia uma sessão delimitada; gesto separado cancela. |
| **Háptica** | Padrões distintos para pronto, rascunho, confirmado, cancelado, ambíguo e bloqueado. |
| **Indicação** | Estado de captura, mute, pareamento e erro deve ser observável. A animação não substitui uma barreira física. |
| **Tela** | Opcional no telefone ou no Dock; não obrigatória no pulso, reduzindo distração, massa e consumo. |
| **Microfone** | Desligado por padrão, com mute físico e janela transitória. Áudio bruto não entra no ledger de produto. |
| **Rádio** | Envia somente significado estruturado e autenticado; nunca transmite sem confirmação física. |
| **Identidade** | Nenhuma identidade, localização ou identificador estável deve ser transmitido por padrão. |
| **Fallback** | Sem telefone, internet ou Dock, o One falha fechado e mantém apenas controles locais seguros. |

O objeto deve parecer uma peça pessoal ou um acessório de roupa, não uma câmera, um mini-telefone ou um equipamento médico. A estética pode variar por materiais, pulseiras, clips e cores, mas nunca deve esconder o estado de captura ou transformar o mute em uma promessa apenas visual.

## 4. O ritual que precisa ser desejado

O primeiro valor deve surgir em poucos segundos. Uma pessoa coloca o One, pressiona, recebe o padrão de “pronto” e entende que pode guardar algo sem abrir uma tela. Quando decide preservar uma ideia, decisão ou compromisso, mantém o gesto físico por uma janela limitada, fala ou fornece uma entrada curta e solta. O sistema produz um candidato efêmero. A pessoa confirma fisicamente; somente depois disso o cartão de memória é persistido.

Para recuperar, a pessoa pergunta pelo canal local disponível. O sistema devolve uma resposta curta no telefone ou Dock e um padrão de conclusão no pulso. Se houver uma correspondência inequívoca, apresenta-a. Se houver ausência ou ambiguidade, apresenta ausência ou ambiguidade. O modelo não pode abrir a memória “mais provável” por conta própria.

A experiência desejada tem três verbos:

| Verbo | Promessa percebida | Contrato técnico |
|---|---|---|
| **Guardar** | “Não vou perder isto.” | Sessão física, candidato transitório, confirmação e persistência autorizada. |
| **Encontrar** | “Consigo recuperar isto sem procurar em tudo.” | Recuperação tipada, match/ausência/ambiguidade e nenhuma abertura automática. |
| **Conectar** | “Consigo avisar sem depender da rede.” | Mensagem semântica compacta, vínculo revogável e uma confirmação por envio. |

## 5. O papel do HERUS Dock

O Dock é o puck circular já previsto, mas deixa de ser uma barreira de entrada. Ele é a expansão que entrega bateria maior, antena, armazenamento controlado e computação local. O HERUS One deve oferecer uma primeira experiência útil com o telefone ou um modo local mínimo; o Dock torna o sistema mais independente, mais privado e mais capaz.

O Dock deve ter display simples, botão de confirmação, cancelamento, USB-C, tampa de serviço e uma antena cuja posição possa ser medida. Ele pode executar um modelo local pequeno e o índice de memória, mas não deve prometer capacidade de nuvem. Um workload de modelo só será considerado adequado depois de medir pico de RAM, p95 de latência, energia, recusas e estabilidade.

A ausência do telefone, da internet ou do Dock nunca pode converter o sistema em autônomo. A degradação correta é bloquear, guardar nada e explicar o estado por háptica/indicador.

## 6. Portfólio e caminho de crescimento

| SKU | Público inicial | Primeiro valor | Função de crescimento |
|---|---|---|---|
| **HERUS One** | profissional de conhecimento, empreendedor e usuário privacy-first | memória seletiva e háptica individual | entrada sem cold start |
| **HERUS Duo** | casal, família, dupla de trabalho ou amigos | comunicação semântica e linguagem privada | pareamento natural e presenteabilidade |
| **HERUS Dock** | usuário avançado e doméstico | bateria, LoRa, storage e inteligência local | independência do telefone |
| **HERUS Anchor** | propriedade, clube, equipe, evento ou bairro | cobertura local ciphertext-only | expansão de rede sem conta central |
| **HERUS Field** | agricultura, construção, apoio, segurança operacional e equipes remotas | coordenação e vocabulário de domínio | receita B2B e validação de alto valor |

O One resolve o problema individual. O Duo cria o efeito social. O Dock aprofunda a experiência. O Anchor amplia a cobertura. O Field financia e testa o sistema em ambientes em que esquecer uma instrução ou perder comunicação tem custo alto.

## 7. Segmentos prioritários

O primeiro público não deve ser “todo mundo”. A ordem recomendada é começar por quem tem dor frequente e consequência real de esquecimento.

| Segmento | Hipótese de valor | Objeção que precisa ser vencida |
|---|---|---|
| **Profissional de campo** | Comunicação curta, estado operacional, checklist e operação fora de cobertura. | Robustez, bateria, alcance e suporte precisam ser reais. |
| **Profissional de conhecimento** | Recuperar decisões, ideias e contexto sem construir um arquivo de vigilância. | Privacidade, integração e taxa de erro. |
| **Empresário/executivo** | Não perder decisões, compromissos e pontos críticos de reuniões. | Já possui alternativas de notas; o HERUS deve ser mais rápido e mais confiável. |
| **Estudante** | Preservar ideias, perguntas e revisão sem gravar aulas automaticamente. | Preço, regras institucionais, consentimento e simplicidade. |
| **Consumidor geral** | Objeto pessoal de memória e comunicação. | Pouca urgência e comparação direta com o celular. |

O consumidor geral pode ser o destino, mas não deve ser o primeiro laboratório. A adoção de massa será consequência de um caso de uso inicialmente estreito que funciona muito bem.

## 8. Loop de adoção legítimo

O ciclo começa com um benefício solo: a pessoa guarda ou recupera algo importante sem abrir menus. Em seguida, ela percebe que o sistema não arquiva tudo e que pode apagar ou revogar. O terceiro passo é parear outro usuário para compartilhar comunicação essencial e desenvolver um vocabulário privado. O quarto é adicionar um Dock ou Anchor em uma casa, propriedade, equipe ou comunidade. O quinto é formar um grupo com um léxico útil.

A cunhagem semântica já documentada pode se tornar um artefato social: duas pessoas desenvolvem atalhos que representam seus usos repetidos. Entretanto, o dicionário deve ser exportável, revogável e degradar para a forma longa quando houver dessincronização. O efeito de rede deve ser criado pelo valor que o uso produz, não por bloqueio ou dependência artificial.

Não usar escassez falsa, reviews comprados, notificações manipulativas, rastreamento oculto ou gamificação que premie captura contínua. Um produto de privacidade que manipula confiança destrói sua própria razão de existir.

## 9. Modelo comercial inicial

A meta de acessibilidade deve ser tratada como requisito de projeto, não como preço prometido. Como hipótese de teste, um One simples poderia ser avaliado em uma faixa de entrada próxima a **US$99–149**, sem assinatura obrigatória; o Dock seria uma expansão premium; e o Field seria vendido por unidade, suporte e lexicon de domínio. A faixa só poderá ser confirmada após BOM, certificação, suporte e margem serem conhecidos.

O caminho de validação deve usar pré-reserva ou depósito reembolsável. “Eu compraria” é um sinal fraco; pagar, usar por duas semanas e voltar voluntariamente é um sinal mais forte. O preço final deve ser testado em pelo menos três faixas e por segmento, sempre distinguindo hardware, serviço opcional e custo de reposição.

## 10. Requisitos de aceitação para o produto desejável

| Requisito | Evidência desejada |
|---|---|
| O usuário novo entende o primeiro valor sem tutorial longo. | Teste observado de primeira utilização e tempo até a primeira ação útil. |
| O modo sem microfone continua útil. | Piloto com microfone fisicamente desabilitado e uso de memória/semântica local. |
| Confirmação e cancelamento são igualmente claros. | Padrões hápticos distintos, teste com usuários e contrato host. |
| A captura não parece vigilância. | Mute físico, estado visível, janela limitada e protocolo de consentimento. |
| O usuário pode apagar, exportar e revogar. | Exercícios de ciclo de vida e recuperação sem serviço externo obrigatório. |
| O Dock melhora o sistema sem ser obrigatório. | Comparação pareada de One sozinho, One + telefone e One + Dock. |
| O sistema não inventa memória. | Testes de ausência, ambiguidade, conflito e modelo bloqueante. |
| O sistema não envia sozinho. | Invariante de confirmação física e teste de tentativa sem botão. |

## 11. Não negociáveis

Nenhuma mudança de marketing, design ou preço pode remover a confirmação física para persistir memória, enviar comunicação ou criar HCP. Áudio, transcrição, embedding, identidade, localização, chaves, prompts, respostas e conteúdo continuam proibidos no log de produto. A LLM pode sugerir e organizar, mas não ganha autoridade. Provas host continuam separadas de medições físicas. A experiência pode ficar mais simples; a autoridade não pode ficar mais ampla.

## 12. Próximo incremento de engenharia

O próximo incremento implementado no núcleo é uma linguagem háptica explícita: o padrão de “rascunho aguardando confirmação” não será igual ao padrão de “confirmação física aceita”. Isso melhora a clareza do produto de massa e é verificável em host, sem criar qualquer caminho novo de transmissão. Em seguida, a documentação de produto e o README devem passar a apresentar o One como entrada, o Dock como expansão e o Field como caminho de validação comercial.

## Referências de contexto

[1] [EssilorLuxottica — Q4/Full Year 2025 Results](https://www.essilorluxottica.com/en/newsroom/press-releases/q4-full-year-2025-results/).

[2] [IDC — Wearable Devices Market Insights](https://www.idc.com/promo/wearablevendor/).

[3] [ŌURA — Newsroom](https://ouraring.com/newsroom).

[4] [ŌURA — Membership](https://support.ouraring.com/hc/en-us/articles/4409086524819-Oura-Membership).

[5] [PLAUD — Plaud Note AI Voice Recorder](https://www.plaud.ai/products/plaud-note-ai-voice-recorder).

[6] [HERUS — Product README](../README.md).
