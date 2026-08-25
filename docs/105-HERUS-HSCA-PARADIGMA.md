# HERUS — HSCA: significado é pequeno o bastante para ser livre

**Estado:** contratos host-only em C11, provados por `./prove.sh`. Nenhuma alegação de campo, alcance físico, energia, ergonomia ou desempenho em silício.
**Comando de prova:** `make -C firmware hsca && python3 tools/test_hsca_corpus.py && python3 tools/test_hsca_redteam.py`

---

## 1. A frase

> Um significado do HERUS tem 34 bytes. Nessa escala o canal deixa de ser um requisito e vira uma escolha — e o alcance deixa de ser função da potência para virar função do tempo.

Todo comunicador off-grid responde "até onde?" com um número em metros, e esse número é limitado pela regulação muito antes de ser limitado pela física ([00-HERUS-MASTER §6](00-HERUS-MASTER.md): SF9 é o teto, não SF12). A HSCA responde a outra pergunta: **quais canais do mundo são largos o bastante?** A resposta medida é: todos.

| | cabe no canal? | |
|---|---:|---|
| um significado (34 B) | **11 de 11 degraus** | toque, glifo, som, BLE, BLE-Coded, ESP-NOW, Wi-Fi, LoRa, malha, satélite, custódia |
| 4 s de fala comprimida (178 B) | 8 de 11 degraus | glifo óptico, chirp acústico e custódia ficam de fora |

Esses dois números saem de `ldr_rungs_for()` e são asseridos sobre a tabela inteira em `test_hsca_ladder.c`, não escritos aqui. Se um degrau deixar de aceitar um significado completo, a suíte quebra.

## 2. O que muda em relação ao HERUS de hoje

O HERUS já transmitia significado. O que a HSCA acrescenta é a **consequência** disso, em cinco camadas que se compõem:

| Camada | Pergunta que responde | Arquivo |
|---|---|---|
| **Herald** | o que a pessoa quis dizer, exatamente e de forma canônica? | `firmware/core/herald.{h,c}` |
| **Ladder** | por qual dos onze canais isso pode ir, sem aumentar autorização? | `firmware/core/ladder.{h,c}` |
| **Drift** | e se não houver canal nenhum agora? | `firmware/core/drift.{h,c}` |
| **Aura** | quem está por perto, sem servidor e sem identidade? | `firmware/core/aura.{h,c}` |
| **Keel** | tudo isso cabe no pulso, sem o Core? | `firmware/core/keel.{h,c}` |

A nomenclatura segue a existente — Core, Band, Lexicon, HCP, Weave, Echo, Vault, Beat ([00-HERUS-MASTER §3](00-HERUS-MASTER.md)). Herald, Ladder, Drift, Aura e Keel entram nela, não ao lado dela.

## 3. Compatibilidade deliberada com o que já existe

A HSCA **não** substitui o HCP, o Weave, o Beat, o ratchet, o `transport_selector`, o `semantic_compiler`, o Resonator nem a memória temporal. A forma de fio do `hir_t` tem 24 bytes e é bit-compatível com o Tier 1 do HCP: 2 bytes de cabeçalho, nove slots papel/preenchedor empacotados 5:11, 4 bytes reservados. No ar são 2 + 24 + 8 = 34 bytes, exatamente o quadro de 246,8 ms em SF9 que o projeto já orçou.

Isso é uma decisão de engenharia, não um detalhe: **nenhum número de RF, energia ou airtime do repositório precisa ser refeito por causa da HSCA.**

| Camada existente | Relação |
|---|---|
| `hcp.{h,c}` | a HSCA produz o payload de 24 B que o HCP já sabia carregar |
| `transport_selector.{h,c}` | continua válido para os quatro rádios; a Ladder é a escada maior, com canais não-rádio e ordenação por queda |
| `semantic_compiler.{h,c}` | gramática exata para fatos/regras/consultas simbólicas; o Herald cobre a fala curta do dia a dia, com convergência de paráfrase |
| `symbol_registry.{h,c}` | o espaço fechado do Herald é o próximo candidato a migrar para handles collision-aware |
| `memory_semantic_evidence` | a memória temporal com supersessão **já existe** ([68](68-HERUS-MEMORIA-SEMANTICA-TEMPORAL-E-CONFLITOS.md)); a HSCA não a reescreve |

## 4. De onde veio a ideia central do Herald

O método é o do compilador de intenção da OonCore: **não deixe o modelo gerar o estado final; faça-o gerar uma representação intermediária pequena que um compilador determinístico transforma em estado executável, com um validador que reprova antes de qualquer efeito.** Lá isso produziu 391 de 399 folhas de uma UI real e 44 de 44 mutações inválidas barradas. Aqui o alvo é outro — a frase de uma pessoa, não um manifesto — mas a forma é a mesma:

```
fala  →  IR tipada pequena  →  compilador determinístico  →  portão  →  efeito
                                   ↑
                       o que não é representável não vira
                       aproximação: vira uma lacuna tipada
```

A lacuna é a parte que interessa. Quando o Herald não sabe representar um lexema, ele devolve `hir gap`: um índice e um hash de 4 bytes, nunca os bytes. Uma lacuna recorrente é o pedido de uma capacidade nova — o mesmo princípio da OonCore, onde uma reprovação recorrente vira construção nova da gramática em vez de gambiarra.

## 5. O que foi deliberadamente recusado

Uma leitura externa propôs levar o HERUS para AMR 3.0, CCG, OWL 2 sob `SROIQ(D)`, SPARQL sub-milissegundo, ProbLog, ASP e TAG. Nada disso entra, e as razões são específicas:

| Proposta | Por que fica de fora |
|---|---|
| AMR 3.0 + CCG | os parsers competitivos de AMR são redes neurais; um parser AMR simbólico devolveria exatamente a fragilidade que o texto queria resolver. Além disso a cobertura em português é escassa |
| OWL 2 / `SROIQ(D)` + SPARQL | `SROIQ(D)` é N2ExpTime-completo; um triplestore com SPARQL sub-milissegundo não roda em ESP32-S3, e exigir isso empurraria o produto de volta para compute externo — o oposto de [64](64-HERUS-SOBERANIA-ON-WRIST-E-FRONTEIRA-CORE.md) |
| ProbLog / ASP completos | domínio finito com abstenção resolve o que o produto precisa; o resto é peso sem usuário |
| TAG para geração | a crítica correta era que template soa robótico; TAG não escapa disso, e a renderização do HERUS é no receptor, a partir de símbolo (§8 do master) |
| AVX-512 | é instrução x86. O núcleo é Xtensa LX7 |

Uma nota de honestidade sobre um número que circulou junto: os **98,8%** citados em favor de VSA vêm de um experimento de matrizes progressivas de Raven, não de raciocínio linguístico, e o próprio artigo publicado reporta médias mais baixas em treino ponta a ponta. Emprestar essa estatística seria fazer exatamente o que se acusa. Nenhum número deste documento vem de outro domínio.

**O que se manteve** da leitura externa: a tese de não depender de LLM hospedada, e HDC/VSA como peça de uma arquitetura maior — que já é o que o repositório faz.

## 6. Resultado medido nesta revisão

Tudo abaixo é produzido pelo código deste repositório, em host.

| Medida | Valor | Onde |
|---|---:|---|
| famílias de paráfrase que convergem para um digest | 10 de 10 | `hsca-herald` |
| formas de superfície aceitas sem divergir | 34 de 34 | `hsca-herald` |
| colisões de digest entre famílias distintas | 0 | `hsca-herald` |
| casos adversariais recusados com a razão certa | 25 de 25 | `hsca-herald` |
| degraus que carregam um significado completo | 11 de 11 | `hsca-ladder` |
| alterações de um bit num pacote de custódia rejeitadas | 272 de 272 | `hsca-drift` |
| entrega de 34 B a 40 km, 8 portadores, 12 populações | 12 de 12, média 7.965 s | `hsca-drift` |
| balizas forjadas aceitas em 100.000 tentativas | 0 | `hsca-aura` |
| combinações admissíveis de pacote de conhecimento | 1 de 32 | `hsca-keel` |
| ações permitidas ao Core no produto cartesiano papel×ação | 4 de 24 | `hsca-keel` |
| RAM de estado de toda a via cognitiva | **1.028 B** | `hsca-keel` |
| invariantes HSCA | 206 | seis suítes |
| controles removidos deliberadamente e detectados | **26 de 26** | `tools/test_hsca_redteam.py` |

O 1.028 B é `sizeof` das estruturas que existem, somado em `keel.c` — 0,2% da SRAM interna do ESP32-S3. É estado de runtime, não tamanho de código.

## 7. Um achado que contraria a intuição

Na simulação de custódia, multiplicar por oito o número de portadores mudou a média de entrega em **9%**, enquanto as transmissões cresceram cerca de **dez vezes**. Replicação cega gasta orçamento sem comprar distância; quem entrega é o tempo. É por isso que o fanout é limitado por projeto e não por educação.

O caminho até esse resultado também produziu uma correção real: a primeira versão marcava como "vista" toda cópia aceita, o que envenenava um portador que recebesse primeiro uma réplica sem saltos restantes. A regra passou a ser *ficar com a melhor réplica*, e a memória permanente ficou só para o que foi consumido.

## 8. O que isto **não** é

- Não é medição de campo. Alcance, energia, ergonomia, RF, antena, bateria e latência real continuam pendentes da Fase 0 ([03-BUILD-GUIDE](03-BUILD-GUIDE.md)).
- Não é acurácia de linguagem natural aberta, nem comparação estatística com um modelo de linguagem, nem reconhecimento de fala.
- A simulação de custódia usa um modelo de mobilidade declarado, unidimensional e determinístico. Ela demonstra o mecanismo; não prevê uma cidade.
- A tabela de portadores é **declarada** a partir de datasheets e especificações. `ldr_unmeasured_count()` devolve 11 de 11 exatamente para impedir que isso seja esquecido.
- Aura é indistinguibilidade observável (balizas distintas, imprevisíveis sem a chave), não prova criptográfica de indistinguibilidade, e não diz nada sobre impressão digital de rádio.

## 9. Ordem correta de trabalho

A Fase 0 continua antes de tudo. A HSCA é software que espera hardware, não desculpa para adiá-lo.

1. **Fase 0** — dois devkits, bancada, RF, energia, o pulso como lugar de antena.
2. **Medir a Ladder** — trocar 11 números declarados por 11 medidos, um degrau por vez.
3. **Medir o Drift** — encontros reais, não modelo. O critério de morte: se a mediana de entrega a 5 km ficar acima de 6 h com dez portadores, a custódia é um recurso de nicho e deve ser apresentada como tal.
4. **Medir o Herald na fala** — WER sobre a gramática fechada. Critério de morte: abaixo de 90% de compilação exata em fala espontânea de usuários reais, a entrada primária volta a ser toque.
5. **Aura em silício** — 64 HMAC-SHA256 por baliza observada é caro; medir e, se preciso, encolher a janela.

## 10. Documentos desta camada

| Documento | Assunto |
|---|---|
| [106 — Herald](106-HERUS-HERALD-COMPILADOR-DE-INTENCAO.md) | IR canônica, convergência de paráfrase, cobertura total, lacunas tipadas |
| [107 — Ladder](107-HERUS-LADDER-ESCADA-DE-PORTADORES.md) | os onze degraus, elegibilidade, ordenação e a fronteira de autoridade |
| [108 — Drift](108-HERUS-DRIFT-ALCANCE-NO-TEMPO.md) | custódia sem leitura, limites, simulação e o achado sobre densidade |
| [109 — Aura](109-HERUS-AURA-PRESENCA-PRIVADA.md) | presença sem servidor, ratchet de época, revogação e limites honestos |
| [110 — Keel](110-HERUS-KEEL-SOBERANIA-MEDIDA.md) | orçamento medido no pulso e a matriz de papéis do Core |
