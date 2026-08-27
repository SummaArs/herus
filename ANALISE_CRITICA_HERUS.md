# Análise crítica profunda do HERUS

**Escopo:** repositório `SummaArs/herus`, estado observado em `09d6874329552a160b03e94c1c607a769cdb6c8f` (`main`, 18 de agosto de 2026).  
**Método:** leitura da arquitetura e da documentação normativa; execução do `prove.sh`; compilação dos alvos de firmware e do simulador; execução com AddressSanitizer/UndefinedBehaviorSanitizer; análise estática com GCC `-fanalyzer`; inspeção de CI, manifestos de evidência, driver de rádio, protocolo, memória e histórico Git.

## 1. Veredito executivo

O HERUS apresenta uma **arquitetura conceitualmente disciplinada e uma cultura de engenharia acima da média para um protótipo pré-hardware**, especialmente na separação entre sugestão algorítmica e autoridade humana, no uso sistemático de estados fail-closed, na explicitação de limites e na tentativa de transformar requisitos de privacidade em invariantes executáveis. A cadeia host de memória, sessão, recuperação e confiança está bem decomposta e possui testes direcionados.

Entretanto, o repositório **não deve ser tratado como release candidate de produto no sentido operacional**. O estado real é melhor descrito como **protótipo de especificação executável e laboratório host-only, com um simulador funcional, um port parcial para ESP32-S3/SX1262 e hardware ainda não validado**. A própria documentação admite que o sistema não foi executado em silício, não sofreu auditoria independente e ainda não possui resultados de campo [1].

O achado mais importante é um **defeito de segurança de memória no simulador que o pipeline oficial não detecta**: `sim/study.c` declara `int by[5]`, mas indexa o vetor com `r.verdict`; o enum possui seis valores, incluindo `LEX_V0_SUGGEST == 5`. Quando ocorre uma sugestão, há leitura/escrita fora dos limites do array. O AddressSanitizer reproduziu um `stack-buffer-overflow` em `sim/study.c:194`. Isso não demonstra, por si só, uma vulnerabilidade no firmware embarcado, mas invalida a afirmação implícita de que a simulação padrão é uma base limpa de evidência e expõe uma falha relevante no próprio mecanismo que sustenta alegações de aprendizagem e recuperação sem erro.

| Dimensão | Avaliação | Fundamentação |
|---|---:|---|
| Arquitetura e decomposição | **Forte** | Módulos pequenos, contratos explícitos, separação entre modelo, intenção, autoridade, memória e transporte. |
| Segurança conceitual | **Boa no host; incompleta no alvo** | AEAD, ratchet, replay, rate limit e fail-closed são exercitados; provisionamento, RNG, mídia, reboot e plataforma continuam pendentes. |
| Qualidade da implementação | **Mista** | A suíte nominal passa, mas sanitização encontra overflow real no simulador e há warnings não tratados. |
| Evidência experimental | **Insuficiente para produto** | Os números de RF, energia, latência e alcance são modelos/simulações; não há resultados de campo ou de silício. |
| Reprodutibilidade | **Moderada** | Há scripts e manifestos; faltam toolchain fixada, artefato alvo, digest independente e CI com sanitizadores. |
| Prontidão comercial | **Baixa** | UX, autonomia, ergonomia, voz, BLE, armazenamento protegido e desempenho local ainda não foram medidos. |

> **Conclusão:** o projeto é intelectualmente sério e tecnicamente promissor, mas a distância entre “contrato host verificado” e “produto seguro e utilizável” ainda é grande. O risco principal não é a falta de ideias; é a possibilidade de que a densidade documental e o número de invariantes produzam uma sensação de maturidade superior à evidência física disponível.

## 2. O que o projeto faz bem

A melhor decisão arquitetural é a **separação explícita de autoridade**. O código e a documentação repetem que o modelo pode sugerir, classificar ou resumir, mas não pode enviar, persistir memória sensível, abrir um cartão ou executar uma ação externa sem confirmação física. Essa propriedade aparece em múltiplas fronteiras — intenção, diálogo, memória, coleção, recuperação, trust e interação — em vez de existir como uma promessa isolada no README [2].

A decomposição da memória é particularmente cuidadosa. Captura transitória, extração tipada, revisão humana, cofre, consolidação, recuperação e apresentação são módulos diferentes. Essa estrutura reduz o risco de um componente “inteligente” receber implicitamente poder de persistir ou comunicar. Os testes também cobrem conflitos, ambiguidade, expiração, remoção, rollback lógico e ausência de confirmação.

O projeto é igualmente forte ao declarar limites. O README afirma que a versão é pré-hardware e que alcance, consumo, ergonomia, reconhecimento de fala, háptica, BLE, armazenamento protegido e desempenho de LLM local ainda dependem de medições reais [3]. O `SECURITY.md` é ainda mais direto: informa “pre-hardware”, ausência de auditoria independente e ausência de execução em silício [1]. Essa honestidade é um ativo importante e deve ser preservada.

| Prática positiva | Por que é relevante |
|---|---|
| Vetores criptográficos independentes e casos aleatórios | Reduz a probabilidade de um erro trivial na implementação customizada passar despercebido. |
| Testes de mutação F4 | Avaliam se determinadas remoções de controles são percebidas pela suíte, e não apenas se o caminho feliz passa. |
| Manifestos separados para prontidão de hardware e proveniência | Evitam misturar evidência host, evidência física e supply chain em um único “passou”. |
| Simulador com firmware de `core` e `net` não duplicado | Evita que o simulador teste uma implementação paralela diferente da que seria portada. |
| Explicitação de “não prova” | Evita, ao menos documentalmente, converter um resultado de simulação em garantia de RF, energia ou UX. |

## 3. Achados críticos e de alta prioridade

### 3.1. P1 — overflow de stack no simulador de estudo

**Evidência:** em `sim/study.c`, `by` é declarado como `int by[5]` na linha 179, e `by[r.verdict]++` ocorre na linha 194 [4]. O enum `lex_verdict_t` possui seis valores: `LEX_OK` de 0 a `LEX_V4_LOCKED` de 4 e `LEX_V0_SUGGEST` igual a 5 [5].

A execução normal de `./prove.sh --quiet` e `make -C sim all` passa porque o build padrão usa `-O2 -Wall -Wextra -std=c11`, sem sanitização, e o overflow pode sobrescrever uma área de stack que não altera o resultado visível. A execução instrumentada reproduziu:

```text
study.c:194:15: runtime error: index 5 out of bounds for type 'int [5]'
AddressSanitizer: stack-buffer-overflow
READ of size 4 ... in scenario_study /home/ubuntu/herus/sim/study.c:194
```

A classificação adequada é **P1 para a infraestrutura de validação**: o defeito está no simulador, não foi demonstrado no firmware embarcado, mas ocorre em uma trajetória default usada para sustentar as alegações de estudo, aprendizagem e ausência de resposta confiante errada. A correção mínima é dimensionar o vetor com seis posições ou, melhor, dimensioná-lo por uma constante derivada do enum e validar o intervalo antes de indexar. A correção adequada também exige adicionar a execução ASan/UBSan à CI e incluir uma asserção de cobertura para todos os veredictos.

### 3.2. P1 — a CI não executa o diagnóstico que encontra o defeito

O workflow `.github/workflows/prove.yml` executa `./prove.sh` em Ubuntu e macOS, mas não inclui ASan, UBSan, `-fanalyzer`, `-Werror`, fuzzing ou uma etapa independente de memória [6]. O documento de prova de fogo chama sanitizadores de “diagnóstico adicional” e os coloca fora do baseline portátil [7]. Isso é aceitável como limitação temporária, mas é inadequado para um repositório que apresenta a simulação como evidência de release candidate.

A consequência é concreta: a validação oficial fica verde enquanto a trajetória padrão contém comportamento indefinido. A ausência de `-Werror` também permite que warnings de inicialização incompleta e parâmetros não usados continuem sem bloquear a integração.

### 3.3. P1/P2 — números de RF e energia são apresentados com precisão maior que a evidência

O simulador reporta alcance, ocupação de banda, latência, perdas por colisão, consumo diário e uma “cliff” de 550–600 m. O próprio simulador informa que propagação, sensibilidade, captura, drift de cristal e corrente são modelos, e a documentação diz que a decisão real permanece na Fase 0 física [8]. Portanto, esses números são úteis como **consistência interna do modelo**, mas não como desempenho do produto.

O risco comunicacional é elevado porque valores como `100 %`, `88 %`, `4246 ms`, `579 m` e `127.71 mAh/day` parecem medições experimentais. Eles devem ser rotulados em toda saída como `model-estimated`, com os parâmetros de entrada, incertezas e análise de sensibilidade. Em particular, não há demonstração de que o modelo represente multipath, interferência real, duty cycle efetivo, variação de antena, tolerância do cristal, temperatura, corpo humano, orientação ou bateria sob carga.

### 3.4. P1 — segurança criptográfica demonstrada apenas para a implementação host e estados ideais

A implementação customizada de SHA-256, HMAC, HKDF, ChaCha20 e Poly1305 é comparada com uma implementação independente e passa os vetores disponíveis [1]. Isso é positivo, mas não equivale a uma auditoria criptográfica. A segurança efetiva depende também de:

| Dependência | Estado observado |
|---|---|
| Geração e provisionamento da raiz de confiança | Não demonstrados no hardware final. |
| Entropia e RNG no primeiro boot | Há chamadas/abstrações de plataforma, mas não há evidência de avaliação física da qualidade da fonte. |
| Persistência atômica de contador e ratchet | O host possui oráculos lógicos; mídia real, corte de energia e atomicidade continuam pendentes. |
| Proteção contra extração de chave | O software zera buffers temporários, mas não prova proteção de flash, debug, side-channel ou falha de zeroização no alvo. |
| Gestão de nonce após reboot | A lógica depende de estado durável e de recuperação; a integração com o armazenamento real ainda não foi demonstrada. |
| Análise independente | Explicitamente ausente no `SECURITY.md`. |

A formulação correta é: **“a implementação host passa testes de conformidade e propriedades selecionadas”**, não “a comunicação está segura contra um adversário capaz”. O próprio repositório faz essa ressalva; o problema é que a densidade das alegações positivas pode eclipsá-la.

### 3.5. P2 — warning logicamente morto no driver SX1262

Em `firmware/port/sx1262.c`, `sx1262_tx` recebe `len` como `uint8_t` e testa `if (len > 255)`. O compilador reporta que a comparação é sempre falsa por limitação do tipo [9]. Não é uma exploração demonstrada, mas é um sinal de higiene de tipos insuficiente em uma fronteira de hardware. O contrato deve usar `size_t` ou `uint16_t` antes de limitar a 255, e a CI deveria tratar esse warning como erro.

### 3.6. P2 — contagem e semântica do ledger exigem uma fonte única

Há números diferentes em pontos distintos da documentação. O documento de prova de fogo registra 39 suítes, 87 invariantes de prova e 74 invariantes simulados [7]. A execução observada passou por 86 checks declarados em `prove.sh`, além de mensagens internas e alvos separados. Essa diferença pode ser explicável por mudanças de contagem ou por categorias diferentes, mas hoje exige interpretação manual.

Recomenda-se gerar automaticamente o número de suítes, checks, cenários e invariantes a partir do pipeline, gravar um manifesto versionado e fazer a documentação consumir esse artefato. Um número de “invariantes” sem definição operacional única é vulnerável a inflação acidental e dificulta a comparação entre commits.

## 4. Auditoria da qualidade de testes

A suíte funcional é extensa: o pipeline executa módulos de álgebra, núcleo, voz, intenção, diálogo, modelo, memória, trust, link de controle, interação, protocolo, rádio e física. Na execução com compilador C real, os alvos oficiais passaram, incluindo `prove.sh --quiet`, `watch-memory-frontend`, `delivery-plan`, todos os alvos de firmware e a simulação nominal.

Isso demonstra **regressão funcional nos casos escolhidos**, não cobertura do espaço de estados. O vetor de testes é majoritariamente determinístico, com seeds e casos direcionados. Há campanhas de snapshots hostis para alguns oráculos, mas não há evidência de fuzzing contínuo, cobertura de branches, cobertura de mutação ampla ou análise de interleavings concorrentes.

O comando `python3 -m unittest discover` encontrou zero testes. Isso não significa que os testes Python sejam inexistentes — vários são executados explicitamente pelo `prove.sh` —, mas significa que a convenção padrão de descoberta não oferece uma segunda rede de segurança. É recomendável migrar ou adaptar os testes Python para uma estrutura descoberta automaticamente, ou declarar formalmente que o projeto não usa `unittest discover` e adicionar um agregador que falhe quando uma suíte esperada não for executada.

| Camada | Evidência atual | Lacuna principal |
|---|---|---|
| Unitários C | Muitos alvos independentes e contratos claros | Sem ASan/UBSan no baseline; cobertura não publicada. |
| Integração host | Grand finales, capstone e campanhas F1–F4 | Interleavings, concorrência e callbacks reais não modelados integralmente. |
| Simulador | 74 invariantes de sistema reportados | Possui overflow; mundo físico é um modelo. |
| Python | Scripts explícitos de auditoria e estudo | Descoberta padrão executa zero testes. |
| CI | Ubuntu e macOS | Sem sanitizadores, fuzzing, coverage, toolchain fixada ou artefato alvo. |
| Hardware-in-the-loop | Não existe no estado auditado | Toda a cadeia de RF, energia, reboot e armazenamento real. |

## 5. Segurança, privacidade e threat model

A separação entre evidência host, controles pendentes no alvo e escopo não suportado é uma das partes mais maduras do projeto. O manifesto de ameaça não transforma automaticamente uma declaração em mitigação; exige evidência canônica e mantém modelo, plataforma física e escopos não suportados fora de “sucesso”. Isso é uma boa prática de governança técnica.

O ponto crítico é que **“fail-closed” no C11 não garante fail-closed no sistema físico**. Para transformar o contrato em propriedade de produto, será necessário provar, com instrumentos e procedimentos reproduzíveis, que:

1. uma interrupção durante `PREPARED`, `COMMITTED`, remoção e compactação não deixa uma cópia antiga utilizável;
2. o contador durável não pode sofrer rollback por corrupção, desgaste ou restauração de backup;
3. a RAM realmente perde chaves e autorização após reset, brownout e watchdog;
4. o boot não reativa uma sessão por erro de inicialização, estado parcial ou callback tardio;
5. o botão físico e a indicação háptica correspondem ao evento que o software considera “confirmação”;
6. debug, bootloader, flash externa, logs e ferramentas de fábrica não expõem chaves, cartões ou transcrições;
7. a privacidade prometida permanece válida quando o rádio, a antena, o relógio e a alimentação entram em estados adversos.

O repositório reconhece praticamente todas essas lacunas. Portanto, a crítica não é que ele as tenha escondido; é que o estágio de produto não pode avançar com base apenas no sucesso dos oráculos host.

## 6. Arquitetura de produto e viabilidade

A proposta de valor é coerente: comunicação curta, privada, offline e com baixa carga visual. O problema é a amplitude do produto. O repositório simultaneamente aborda wearable, dock, memória pessoal, Paper-Core, LoRa, equipe de campo, diálogo local, LLM local, conhecimento privado, confiança, UX háptica e produto de massa. Cada eixo possui riscos próprios e pode consumir a capacidade de validação do projeto.

A recomendação estratégica é reduzir o primeiro experimento a uma **fatia vertical mínima**: dois dispositivos, um botão, uma confirmação, um tipo de mensagem sem áudio, uma sessão curta, um rádio específico e um protocolo de armazenamento simples. A memória seletiva, a LLM local, a linguagem privada e a coleção multi-cartão devem ser mantidas como linhas posteriores até que RF, energia, interação e recuperação física estejam medidos.

A afirmação “significado antes de mensagem” é uma boa hipótese de diferenciação, mas ainda não é uma vantagem comprovada. Para validá-la, será necessário comparar HERUS com alternativas reais — rádio de voz, mensageria offline, LoRa convencional e comunicação por estados — usando métricas de tempo para completar tarefa, taxa de erro, carga cognitiva, compreensão e confiança do usuário. Sem estudo humano, “menos fricção” e “coordenação clara” permanecem hipóteses de produto.

## 7. Reprodutibilidade e supply chain

O projeto tem scripts locais, manifestos JSON e digests declarados, o que é superior a um protótipo sem rastreabilidade. Ainda assim, o próprio documento de proveniência não se apresenta como SBOM completo, atestação assinada, SLSA, build reproduzível ou garantia de supply chain [10]. Não há artefato final de firmware, digest reproduzido em ambientes independentes, versão fixada do ESP-IDF/toolchain ou comparação de binários de alvo.

A CI usa `ubuntu-latest` e `macos-latest`, tags móveis que mudam com o tempo. Isso é adequado para detectar portabilidade geral, mas não para uma cadeia de release auditável. O build depende do compilador disponível no ambiente; no sandbox inicial nem `cc` estava instalado, embora isso seja uma limitação do ambiente local e não uma falha do GitHub Actions. Ainda assim, a ausência de pinagem de toolchain reduz a reprodutibilidade histórica.

Prioridades para release engineering:

| Prioridade | Entrega recomendada |
|---:|---|
| 1 | Fixar versões de compilador, Python, ESP-IDF, SDK e dependências; publicar recipe de build. |
| 2 | Gerar SBOM do artefato e dos componentes transitivos. |
| 3 | Produzir firmware assinado ou ao menos um artefato imutável com digest e metadados. |
| 4 | Reproduzir o digest em dois ambientes independentes. |
| 5 | Separar evidência de “fonte”, “host build”, “target build” e “hardware test”. |
| 6 | Fazer a CI executar sanitizadores e falhar com warnings críticos. |

## 8. Plano de correção priorizado

### Imediato — antes de aceitar qualquer novo claim

Corrigir o overflow de `sim/study.c`, adicionar uma constante `LEX_VERDICT_COUNT` ou uma checagem de intervalo, e fazer o cenário `study` rodar sob ASan e UBSan em cada pull request. Corrigir o tipo de `sx1262_tx`, eliminar inicializadores incompletos de `herus_link` e resolver todos os warnings com `-Werror` nos alvos host.

Adicionar um job de CI que compile pelo menos o simulador, `core`, `net` e todas as suítes com `-fsanitize=address,undefined -fno-omit-frame-pointer`. O job deve executar os mesmos cenários default, não apenas testes unitários menores. Adicionar também GCC `-fanalyzer` e, se possível, Clang `-Weverything` filtrado por regras justificadas.

### Curto prazo — fechar a evidência host

Publicar cobertura de linhas e branches; adicionar fuzzing limitado e determinístico para decodificação HCP, abertura de frames, snapshots de recuperação, parsing de manifestos e transições de sessão. Fazer cada fuzzer salvar uma seed mínima reproduzível. Transformar a contagem de checks em artefato gerado automaticamente.

Separar claramente, em toda saída, `PASS: contrato host`, `MODEL: estimativa`, `PENDING: evidência alvo` e `BLOCKED: depende de hardware`. Eliminar números de alcance, energia e latência sem parâmetros ou incerteza explícitos.

### Médio prazo — bancada física

Construir a menor bancada possível com dois devkits, rádio SX1262, fonte monitorada, analisador ou instrumento RF apropriado, medição de corrente, botão e atuador háptico. Registrar temperatura, orientação, antena, potência, SF, largura de banda, taxa de código, canal, distância, perdas, retransmissões e tempo de recuperação. Pré-registrar critérios de parada e não atualizar a documentação de produto antes de o protocolo de coleta estar congelado.

Testar reset, brownout, watchdog, corte de energia em cada ponto de persistência, desgaste de mídia, boot interrompido, extração via debug e limpeza de RAM. Repetir com chaves e mensagens sintéticas; não usar dados pessoais nos harnesses.

### Longo prazo — validação de produto e segurança

Submeter o protocolo criptográfico e a implementação a revisão independente. Realizar estudo humano comparativo com tarefas reais. Só depois de medições de energia, RF, UX, robustez e segurança de plataforma deve o produto ser descrito como release candidate operacional.

## 9. Conclusão final

O HERUS merece crédito por tentar construir **limites executáveis**, e não apenas uma narrativa de inteligência privada. O desenho de autoridade humana, a separação de memória, os estados de recuperação e a documentação dos limites são pontos fortes reais.

Mas o repositório também mostra por que “muitos testes verdes” não bastam: um único vetor fora dos limites, escondido no simulador nominal e ausente da CI, foi suficiente para demonstrar que a infraestrutura de evidência ainda não é totalmente confiável. A prioridade correta não é acrescentar mais uma camada conceitual ou mais um “Grand Finale”; é **reduzir a superfície, corrigir a infraestrutura, instrumentar o alvo, medir o mundo físico e tornar cada claim proporcional à evidência**.

**Classificação global:**

> **Arquitetura: forte. Implementação host: promissora, mas com defeito de memória confirmado no simulador. Segurança: parcialmente demonstrada. Produto: pré-hardware. Prontidão para uso real: não comprovada.**

## Referências

[1]: https://github.com/SummaArs/herus/blob/main/SECURITY.md "HERUS — Security"
[2]: https://github.com/SummaArs/herus/blob/main/README.md "HERUS — README"
[3]: https://github.com/SummaArs/herus/blob/main/README.md#estado-atual "HERUS — Estado atual"
[4]: https://github.com/SummaArs/herus/blob/main/sim/study.c "HERUS — sim/study.c"
[5]: https://github.com/SummaArs/herus/blob/main/firmware/core/lexicon.h "HERUS — firmware/core/lexicon.h"
[6]: https://github.com/SummaArs/herus/blob/main/.github/workflows/prove.yml "HERUS — GitHub Actions prove workflow"
[7]: https://github.com/SummaArs/herus/blob/main/docs/35-PROVA-DE-FOGO-HOST.md "HERUS — Prova de fogo host"
[8]: https://github.com/SummaArs/herus/blob/main/sim/README.md "HERUS — Simulador"
[9]: https://github.com/SummaArs/herus/blob/main/firmware/port/sx1262.c "HERUS — firmware/port/sx1262.c"
[10]: https://github.com/SummaArs/herus/blob/main/docs/29-PROVENIENCIA-LOCAL-BUILD.md "HERUS — Proveniência local de build"
