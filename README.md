# HERUS

**Memória pessoal soberana, conversa local e comunicação essencial — com significado antes de mensagem.**

O HERUS é um sistema pessoal de memória, conversa e comunicação para o cotidiano e para momentos em que telefone, rede móvel e interfaces densas não são a ferramenta certa. No relógio, ele ajuda a pessoa a perguntar, guardar e reencontrar o essencial. No Paper-Core, concentra escrita, estudo, conhecimento local e memória. Em vez de depender de uma LLM hospedada ou transmitir áudio e longas mensagens, ele trabalha com **significados essenciais**, cartões de contexto e autoridade física.

O produto foi pensado para trilhas, áreas rurais, deslocamentos, equipes de apoio, eventos externos e qualquer situação em que coordenação clara importa mais que uma conversa longa.

> O HERUS não tenta substituir o telefone. Ele existe para preservar comunicação humana essencial quando conectividade, atenção ou privacidade falham.

## A experiência HERUS

A pessoa usa um vestível simples. Ela inicia uma interação por um gesto físico, expressa uma intenção curta e recebe confirmação clara por voz local, interface mínima ou vibração. Nada é transmitido sem confirmação física.

| Princípio | O que significa na prática |
|---|---|
| **Significado primeiro** | O rádio carrega uma intenção estruturada, não áudio ou uma transcrição literal. |
| **Privado por padrão** | O produto não depende de conta, nuvem, identidade transmitida, localização ou histórico de conversa para operar. |
| **Pessoa no comando** | Inteligência pode sugerir, resumir ou perguntar; ela nunca envia, publica, compra ou altera algo por conta própria. |
| **Offline de verdade** | O funcionamento essencial é local e continua útil sem sinal de celular ou internet. |
| **Falhar fechado** | Ambiguidade, confiança insuficiente, replay, expiração ou vínculo revogado bloqueiam a ação em vez de “tentar mesmo assim”. |

## Evolução para produto de massa

A direção de produto passa a priorizar valor individual antes do efeito de rede. O **HERUS One** é a porta de entrada: um wearable discreto, com botão físico, háptica, estado de privacidade e memória seletiva confirmada. O **Núcleo/Dock** é a expansão de bolso que acrescenta bateria, antena, armazenamento controlado e computação local. A comunicação LoRa, a linguagem privada e o Anchor continuam como multiplicadores, não como requisito para o primeiro valor.

Essa mudança não reduz a autoridade humana. Nenhuma camada pode persistir memória, enviar significado ou criar HCP sem confirmação física. O objetivo é tornar a experiência mais simples e desejável sem transformar a inteligência em autonomia.

## O sistema: HERUS One + Núcleo/Dock

O HERUS é composto por duas presenças complementares.

| Elemento | Papel |
|---|---|
| **HERUS One** | A interface pessoal e imediata: gesto, confirmação física, vibração, estado de privacidade e memória seletiva. Entrada inicial sem tela obrigatória e sem captura contínua. |
| **Núcleo/Dock** | Um dispositivo circular de bolso que amplia bateria, antena, armazenamento controlado e capacidade de computação local. Ele atua como estação-base pessoal para o One. |
| **Anchor/Field** | Expansões para cobertura ciphertext-only, grupos e equipes de campo; não são pré-requisitos para o primeiro valor individual. |

O Núcleo é também a base para a próxima evolução do HERUS: um **complemento seletivo de memória pessoal**. A visão não é gravar toda a vida da pessoa. É identificar, com consentimento, ideias, decisões, compromissos e contexto que valem recuperar no futuro, descartando o restante.

Uma futura LLM local poderá ajudar o Núcleo a organizar e explicar essa memória. Ela será uma camada de raciocínio e recuperação, não uma fonte de autonomia: qualquer uso de comunicação, armazenamento sensível ou ação externa continuará sob controle da pessoa.

## Para quem é

| Cenário | Valor do HERUS |
|---|---|
| **Trilhas, campo e estrada** | Coordenação curta entre pessoas sem depender de cobertura móvel. |
| **Equipes de apoio** | Estados e intenções rápidos, sem a fricção de rádio de voz contínuo. |
| **Família e grupos pequenos** | Confirmação de chegada, espera, encontro, mudança de plano ou ajuda. |
| **Rotinas com baixa atenção visual** | Interação por gesto, fala curta e háptica em vez de telas e menus. |
| **Memória pessoal diária** | Recuperação privada de ideias, decisões e compromissos que a pessoa autorizou lembrar. |
| **Relógio e Paper-Core** | Conversa local no pulso, estudo e escrita em uma estação pessoal soberana. |

## Estado atual

O HERUS está em **release candidate pré-hardware**. A arquitetura, os contratos de privacidade, a confirmação física, a inteligência local limitada, o vínculo entre vestível e Núcleo, as barreiras de modelo, o cofre cifrado de cartão mínimo, a consolidação humana limitada, a recuperação tipada controlada, sua apresentação simbólica de status, a composição Grand Finale da cadeia de memória, uma coleção multi-cartão transacional limitada, seu índice privado abstencionista, sua composição multi-cartão com confirmação humana e sem fallback, seu oráculo de recuperação por interrupção, sua sessão de coleção vinculada a propósito com expiração e consumo, seu oráculo de recuperação durável de reservas que nunca reativa sessão após reboot, sua quarentena de boot que importa somente o piso e apaga evidência transitória, seu Gran Finale host que compõe boot, coleção e TM-04 sem reativar sessão, sua primeira prova de fogo determinística de recuperação/quarentena que encontrou e bloqueou piso terminal, sua referência local de integridade de build e um modelo de ameaças executável para controles host, lacunas de alvo e escopo residual foram implementados e verificados em host.

Ainda não há resultados de campo. Alcance, consumo, ergonomia, reconhecimento de fala, comportamento háptico, integração BLE, armazenamento protegido e desempenho de uma LLM local precisam ser medidos no hardware real antes de se tornarem alegações de produto.

A próxima etapa física é a Fase 0: dois devkits, bancada curta, medição RF, energia e interação, com critérios de interrupção definidos antes da coleta.

## Documentação principal

| Documento | Para quê serve |
|---|---|
| [Visão do produto](docs/04-PRODUCT.md) | Propósito, proposta de valor e direção de produto. |
| [Produto desejável e adoção](docs/38-PRODUTO-DESEJAVEL-E-ADOCAO.md) | Reposicionamento para valor individual, HERUS One, Dock, portfólio, UX e hipóteses de adoção. |
| [Núcleo](docs/06-NUCLEO.md) | Papel do dispositivo circular de bolso, privacidade e caminho de inteligência local. |
| [LLM local em ESP32](docs/40-LLM-LOCAL-ESP32-E-HERUS.md) | Avaliação da demonstração ESP32-S3, orçamento de memória e workload correto para o HERUS. |
| [Proposta tipada e invariantes](docs/41-PROPOSTA-TIPADA-E-INVARIANTES.md) | Fronteira fail-closed entre modelo local, candidato de memória e autoridade humana. |
| [Memória seletiva](docs/17-MEMORIA-SELETIVA.md) | Política inicial para lembrar ideias, decisões e contexto útil sem gravar a vida inteira. |
| [Captura consentida](docs/18-SESSAO-CAPTURA-MEMORIA.md) | Sessão física, limitada e transitória que antecede qualquer memória pessoal. |
| [Extração de candidatos](docs/19-EXTRACAO-CANDIDATOS.md) | Interpretação local e conservadora que cria sinais tipados sem guardar a fala. |
| [Cofre de memória](docs/20-COFRE-MEMORIA.md) | Cartão mínimo cifrado, autorização humana separada, geração anti-rollback e apagamento fail-closed. |
| [Consolidação humana](docs/21-CONSOLIDACAO-HUMANA.md) | Revisão física limitada, expiração sem retenção, conflito não automático, recuperação por identificador e remoção controlada. |
| [Recuperação controlada](docs/22-RECUPERACAO-SEMANTICA.md) | Matching local de cartões tipados com limiar, razões e ambiguidade explícita; sem busca livre, escrita ou autoridade de modelo. |
| [Interface de recuperação](docs/23-INTERFACE-RECUPERACAO-HUMANA.md) | Status simbólico one-shot para correspondência, ausência e ambiguidade; sem conteúdo livre, desempate, escrita, envio ou ação. |
| [Grand Finale de memória](docs/24-GRAND-FINALE-MEMORIA.md) | Prova composta da cadeia de captura ao status humano, com conflito/modelo bloqueantes e gates explícitos para hardware e avaliação. |
| [Modelo de ameaças executável](docs/25-MODELO-AMEACAS-EXECUTAVEL.md) | Evidência rastreável para riscos de rádio, trust, memória, telemetria e modelo; lacunas físicas e supply chain continuam explícitas. |
| [Coleção de memória](docs/26-COLECAO-MEMORIA.md) | Até oito cartões mínimos autorizados em uma transação cifrada; recuperação, exclusão e compactação lógicas sem alegação de mídia física. |
| [Índice privado da coleção](docs/27-INDICE-PRIVADO-COLECAO.md) | Consulta tipada, física e limitada que retorna apenas match inequívoco, ausência ou ambiguidade; sem listagem, abertura automática, texto ou modelo. |
| [Recuperação transacional](docs/28-RECUPERACAO-TRANSACIONAL.md) | Oráculo C11 de estados pós-interrupção: promove somente sucessor autenticado ancorado no piso, descarta preparação pré-piso e bloqueia contradições; sem alegação de power-loss físico. |
| [Proveniência local de build](docs/29-PROVENIENCIA-LOCAL-BUILD.md) | Inventário direto e digests locais fail-closed para insumos de prova; não é SBOM completo, atestação assinada, SLSA, build reproduzível ou garantia de supply chain. |
| [Grand Finale da coleção](docs/30-GRAND-FINALE-COLECAO.md) | Cadeia multi-cartão de consentimento à apresentação abstencionista; sem abertura automática, fallback unitário, autoridade de modelo ou alegação de backend/hardware físico. |
| [Sessão física vinculada a propósito](docs/31-SESSAO-FISICA-PROPOSITO.md) | Gate transitório para inserir, abrir, remover, compactar ou consultar a coleção com propósito, validade e consumo explícitos; não prova gesto, pessoa, biometria ou hardware. |
| [Recuperação de reserva de sessão](docs/32-RECUPERACAO-RESERVA-SESSAO.md) | Oráculo pós-reboot para marcadores autenticados e piso durável declarado: avança apenas ID queimado, bloqueia contradições e nunca reativa uma sessão. |
| [Quarentena de boot da sessão](docs/33-QUARENTENA-BOOT-SESSAO.md) | Costura C11 que reconstrói o gate em `IDLE`, importa somente o piso recuperado e exige novo evento para toda sessão posterior. |
| [Gran Finale pré-hardware](docs/34-GRAN-FINALE-PRE-HARDWARE.md) | Composição final host de bootstrap, M14 e TM-04: qualquer divergência bloqueia; o único sucesso permanece `IDLE` e pede sessão nova. |
| [Prova de fogo host](docs/35-PROVA-DE-FOGO-HOST.md) | Campanha F1 determinística: snapshots hostis atravessam recuperação e bootstrap; corrigiu o piso terminal `UINT32_MAX` antes de atingir hardware. |
| [Especificação do sistema](docs/00-HERUS-MASTER.md) | Arquitetura geral, protocolo, segurança, energia e limites conhecidos. |
| [Guia de construção](docs/03-BUILD-GUIDE.md) | Próximos passos de hardware e critérios para interromper ou prosseguir. |
| [HERUS indispensável e inteligência própria](docs/47-HERUS-INDISPENSAVEL-E-INTELIGENCIA-PROPRIA.md) | Revisão de produto, mercado, Watch, Paper-Core, conhecimento local e tecnologia sem LLM hospedada. |
| [Segurança](SECURITY.md) | O que a criptografia protege hoje e o que ainda depende de integração física. |
| [Aprendizados do Atlas_Node](docs/44-ATLAS-NODE-APRENDIZADOS.md) | Comparação auditável com um sistema ESP32/BLE/rádio e adaptação de transporte limitada. |
| [Arquitetura finita e linguagem](docs/48-ARQUITETURA-FINITA-E-LINGUAGEM.md) | VSA/HDC limitado a vocabulário finito; memória por regras; LLM local futura apenas como camada linguística medida no hardware. |
| [Protótipo de raciocínio finito](research/FINITE-REASONER-README.md) | Trilha host-only experimental: composição tipada, saturação limitada e hipóteses paraconsistentes, sem autoridade sobre o produto. |
| [Desafio do raciocínio generativo](docs/49-DESAFIO-RACIOCINIO-GENERATIVO.md) | Tese de pesquisa: generalização combinatória segura, aprendizado local mínimo e limites contra a promessa de raciocínio aberto universal. |
| [Intent Compiler e Semantic IR](docs/50-INTENT-COMPILER-E-SEMANTIC-IR.md) | Conexão entre compilação de intenção, representação intermediária semântica, percepção multimodal e núcleo cognitivo verificável. |
| [Manifesto de datasets reais](research/datasets_manifest.json) | Proveniência, licença, alinhamento, hashes, estado de download e limites de alegação para MIntRec, Common Voice, WESAD, SLURP, Fluent Speech Commands e MInDS-14. |
| [Protocolo de anotação independente](research/annotation_protocol.py) | Vocabulário finito de anotação HERUS; `OTHER`, `AMBIGUOUS` e `CONFLICT` não têm autoridade operacional e rótulos externos não são convertidos. |
| [Evidências Wide Research ciclo 04](research/evidence/wide_cycle_04/) | Logs e auditorias agregadas do primeiro uso local de metadados reais; dados crus permanecem fora do Git. |
| [Relatório Wide Research ciclo 05](research/evidence/wide_cycle_05/report.md) | Auditoria real do SLURP, probe ZIP por Range, licenças por modalidade, parser C e bloqueios restantes; nenhum áudio ou archive bruto publicado. |
| [Evidências Wide Research ciclo 06](research/evidence/wide_cycle_06/) | Auditoria MInDS-14 por API `/rows`, quatro WAVs estruturais temporários, split pt-PT completo sem download de áudio, correção de falso HELP e identidade SLURP ainda ambígua. |
| [Evidências Wide Research ciclo 07](research/evidence/wide_cycle_07/) | Política de evolução incremental, inventário de contratos, ensaios de Core-Link/HCP/Semantic IR/coleção e decisão de mudança mínima; sem migração persistente alegada. |
| [Evidências Wide Research ciclo 08](research/evidence/wide_cycle_08/) | Brecha reproduzida no limite de confiança do hint contextual, regressão antes/depois e correção fail-closed de uma condição; nenhum novo caminho de autoridade. |
| [Laboratório generativo v1](research/generative_lab/) | Núcleo host-only de termos tipados, geração limitada, regras, hipóteses, conflitos locais e saturação; não é linguagem aberta nem possui autoridade. |

## Evolução incremental

O HERUS adota o princípio **construir, estabilizar, observar, melhorar pontualmente e preservar** como política de contratos e invariantes, não como proibição absoluta de mudança interna. HCP tem uma extensão forward-compatible específica para papéis desconhecidos; Semantic IR, Core-Link e persistência exigem versão/migração quando a estrutura fechada muda. Uma correção de segurança pode exigir quebra controlada. O ciclo 07 não adicionou autoridade ao núcleo: adicionou regressões de versão/tamanho do Core-Link, testes host-only e documentação verificável. No ciclo 08, uma regressão real mostrou que `intent_context_hint_t.confidence_pct=255` podia promover uma observação ambígua; o gate passou a exigir `0..100`, com teste antes/depois. A correção alterou uma condição, sem nova autoridade ou mudança de wire. Veja a [política de evolução](research/evidence/wide_cycle_07/evolution_policy.md), a [decisão de complexidade](research/evidence/wide_cycle_07/complexity_report.md) e o [relatório da brecha](research/evidence/wide_cycle_08/breach_report.md).

## Laboratório generativo simbólico

O diretório [`research/generative_lab/`](research/generative_lab/) é a primeira implementação executável da vertente paradigmática do HERUS. Ele gera e compõe termos tipados, deriva consequências por regras declaradas, isola conflitos por contexto e agrupa equivalências sob orçamento finito. O benchmark formal v1 passou 8/8 casos e gerou 25 termos na fixture de profundidade 3. A medição por profundidade chegou a 137 termos na profundidade 6, com custo observado de 2,172 ms no host da execução.

Esses números demonstram somente composição simbólica finita. Não demonstram compreensão de linguagem, grounding, conhecimento aberto, aprendizagem ou substituição de modelos de linguagem. O laboratório não importa firmware, não executa código gerado, não aceita rótulos externos e não possui ponte de autoridade. Os comandos são `make -C research generative-lab` e `make -C research generative-lab-scale`. Os limites e resultados estão em [`research/evidence/generative_lab_v1/limits.md`](research/evidence/generative_lab_v1/limits.md).

O [`research/free_reasoner/`](research/free_reasoner/) acrescenta uma segunda linha experimental: um kernel polinomial exato sobre `Fraction`, descoberta limitada por orçamento, síntese enumerativa sobre exemplos e contraexemplos em uma grade declarada. Ele amplia a investigação de equivalência algébrica, mas continua host-only, inerte, fechado ao fragmento implementado e sem caminho para Semantic IR, firmware, memória, rádio ou autoridade humana. Os testes e o benchmark dedicados podem ser reproduzidos com `PYTHONPATH=research python3 -m unittest -v free_reasoner.test_free_reasoner` e `PYTHONPATH=research python3 -m free_reasoner.benchmark`; o perfil C11 de memória fixa pode ser executado com `make -C research free-reasoning-embedded-profile`. Este perfil é um proxy sem heap para orçamento inicial, não uma medição de ESP32; a compilação no alvo real continua obrigatória antes de qualquer alegação de desempenho. A comparação contra enumeração, incluindo o caso subdeterminado que falha no holdout, está em [`research/evidence/free_reasoner_optimizer/comparison_report.md`](research/evidence/free_reasoner_optimizer/comparison_report.md). Os resultados não constituem prova de raciocínio aberto geral.

## Trilha de verificação e síntese de sistemas críticos

O HERUS mantém uma trilha host-only para síntese e verificação de máquinas de estado finitas. O MVP possui especificação declarada, verificador independente, síntese enumerativa limitada, resultados `VERIFIED`, `COUNTEREXAMPLE`, `UNKNOWN` e `INVALID_SPEC`, benchmark reproduzível e perfil C11 sem heap. A especificação formal está em [`docs/54-MVP-SINTESE-VERIFICACAO-MAQUINAS-ESTADO.md`](docs/54-MVP-SINTESE-VERIFICACAO-MAQUINAS-ESTADO.md).

Esta trilha não é chamada de certificação, segurança absoluta ou estado da arte. Uma prova é relativa ao modelo e à especificação; não garante que requisitos, sensores, compilador ou ambiente físico estejam corretos. O padrão de avanço será benchmark público, baseline comparável, checker independente, contraexemplos, reprodução externa e custos medidos. A evidência da rodada atual está em [`research/evidence/critical_state_mvp_round.md`](research/evidence/critical_state_mvp_round.md).

## Estado de engenharia

A versão consolidada pode ser verificada localmente com:

```bash
./prove.sh --quiet
make -C firmware watch-memory-frontend
make -C research test
# somente com os TSVs MIntRec baixados localmente e ignorados pelo Git:
make -C research real-corpus-audit real-corpus
# somente com o repositório textual SLURP e metadata obtidos legitimamente:
make -C research slurp-text-audit SLURP_ROOT=/caminho/slurp SLURP_METADATA=/caminho/metadata.json
make -C research slurp-text-sanitizers SLURP_ROOT=/caminho/slurp
# somente com autorização local explícita para baixar WAVs temporários:
make -C research minds14-sample-audit MINDS14_ALLOW_AUDIO=1
make -C research minds14-batch-audit MINDS14_ALLOW_AUDIO=1
make -C research minds14-batch-sanitizers MINDS14_ALLOW_AUDIO=1
```

O comando executa as verificações portáveis e o simulador. A análise Atlas_Node inclui ainda a suíte explícita `make -C firmware delivery-plan`. Um resultado positivo confirma contratos de software e autoriza somente o início controlado da bancada; ele **não** constitui evidência de alcance, energia, UX ou desempenho físico.

O HERUS não resolve NLU/NLG aberto: VSA/HDC opera somente sobre o vocabulário finito dos cartões de contexto, e a memória seletiva é governada por regras e estados tipados. A decisão normativa está em [Arquitetura finita e linguagem](docs/48-ARQUITETURA-FINITA-E-LINGUAGEM.md). Uma LLM local futura poderá atuar apenas como camada linguística no Núcleo depois de orçamento medido em hardware real, sem autoridade sobre memória, rádio ou confirmação física.

A fronteira entre uma proposta de modelo e um candidato de memória pode ser exercitada com `make -C firmware memory-proposal`. O sizing grosseiro da demonstração de LLM em ESP32-S3 pode ser reproduzido separadamente com `make -C firmware llm-budget-check`. Esses alvos validam somente contratos e comparações C11/Python host-only; persistência, HCP, comunicação, inferência, qualidade, autonomia e desempenho continuam exigindo os gates humanos e físicos existentes.

A história detalhada de experimentação, provas e decisões de implementação é preservada no ramo [`internal/engineering-archive`](https://github.com/SummaArs/herus/tree/internal/engineering-archive). Ela existe para rastreabilidade de engenharia, sem ocupar a apresentação principal do produto.

## Licença

Proprietary. Copyright © 2026 Gustavo Gonçalves. Todos os direitos reservados — veja [LICENSE](LICENSE).
