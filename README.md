# HERUS

## Comunicador semântico pessoal; pesquisa em assurance crítico

> **HERUS é um comunicador pessoal privado, offline e orientado a significado, cuja arquitetura está sendo expandida para coordenação e assurance de sistemas críticos.**

O produto principal continua sendo o comunicador: wearable, Núcleo/Dock, interação push-to-talk, linguagem semântica finita, memória seletiva e autoridade física humana. A vertente de sistemas críticos é a expansão de pesquisa desenvolvida na branch de integração; ela não substitui a identidade original do HERUS.

O projeto **não é apresentado como uma LLM, agente autônomo, certificação regulatória ou sistema de segurança completo**. As alegações são limitadas ao que código, testes, modelos, hashes e evidências reproduzíveis sustentam.

## O sistema atual

A leitura correta separa **produto principal** de **vertente de pesquisa**. O comunicador, a documentação de produto, o Núcleo/Dock e o Paper-Core continuam sendo a identidade original do HERUS. A pesquisa de assurance procura estender essa arquitetura para sistemas em que estados, riscos e autoridade precisam ser demonstráveis.

| Camada | Responsabilidade | Estado |
|---|---|---|
| **Coordenação finita** | Intenção, sessão física, confirmação, confiança, revogação e handoff | Implementada em C11 e testada no host/simulador |
| **Protocolo e enlace** | Composição semântica, autenticação, replay, expiração e limites de transporte | Implementados com invariantes e testes adversariais |
| **Verificação crítica** | Verificador independente e síntese limitada de máquinas de estado | Implementada em Python, com contraexemplos e estados `UNKNOWN` |
| **Refinamento** | Relação entre modelos abstratos e concretos | Implementado para o domínio finito atual |
| **Assurance de sinks** | Auditoria de guardas em operações críticas reais | Implementada lexicalmente; análise AST/interprocedural ainda pendente |
| **Guardian de runtime** | Observação, risco, bloqueio, alerta e registro | Modelo host-only determinístico; ainda não é firmware de produção |
| **Interface humana** | Tela, voz ou pulso como canais de alerta e resposta tipada | Contrato arquitetural; codec e hardware ainda pendentes |

## Princípio de autoridade

O HERUS pode **observar, verificar, classificar, registrar, solicitar intervenção e bloquear**. Ele não pode converter um modelo, uma interface, um ACK, uma heurística ou uma evidência incompleta em autoridade automática.

Para qualquer ação crítica `a`:

```text
ALLOW(a) =>
  estado_válido(a)
  ∧ autoridade_válida(a)
  ∧ confirmação_exigida(a)
  ∧ não_expirada(a)
  ∧ não_revogada(a)
  ∧ não_repetida(a)
  ∧ evidência_fresca(a)
```

Se uma relação necessária não puder ser demonstrada, o resultado correto é `UNKNOWN`, `BLOCKED`, `STALE`, `INCONCLUSIVE` ou `FAIL`; nunca `PASS` por conveniência.

## Arquitetura

```text
estado observado
      ↓
modelo finito e invariantes
      ↓
verificação · síntese · refinamento
      ↓
auditoria de sinks · proveniência · mutação
      ↓
Guardian de runtime
      ↓
classificação determinística de risco
      ↓
bloqueio fail-closed
      ↓
alerta humano tipado: tela · voz · pulso
      ↓
decisão humana registrada
      ↓
revalidação pelo runtime HERUS
```

A camada de assurance observa o runtime real. Ela não cria uma autoridade paralela. A decisão operacional continua pertencendo aos contratos do núcleo, incluindo confirmação física, trust, revogação, persistência autorizada e handoff.

## Como o projeto evoluiu

### 1. Comunicação semântica privada

A primeira tese foi transmitir significado estruturado em vez de áudio ou texto aberto. HCP, HDC/VSA limitado, Echo, Weave, Beat, Vault e a escada de mensagens foram especificados com limites de airtime, energia, replay e compatibilidade. Essa linha continua documentada como produto e firmware, mas não é mais a única definição do projeto.

### 2. Produto e memória soberana

O HERUS One, o Núcleo/Dock, a memória seletiva e os cartões de contexto estabeleceram a regra de que inteligência pode sugerir, mas não pode persistir, transmitir ou executar ações sem autoridade humana. O produto permanece uma direção válida, porém os resultados físicos ainda dependem de hardware real.

### 3. Raciocínio simbólico finito

O laboratório generativo evoluiu de composição tipada para DAGs estruturais, saturação limitada, hipóteses locais, kernel polinomial exato, busca enumerativa, bandits, beam search e MCTS. O resultado demonstrado é sistematicidade dentro de vocabulário e domínio finitos. Isso **não** constitui raciocínio aberto geral nem substituição de LLM.

### 4. Sistemas críticos

O centro científico mudou para síntese e verificação de máquinas de estado. O HERUS passou a usar um verificador independente, síntese limitada guiada por esse verificador, contraexemplos, perfil C11 sem heap, monitoramento de surpresa causal e refinamento abstrato-concreto. Uma prova do modelo não é transferida à implementação sem uma relação de refinamento verificada.

### 5. Vertente de pesquisa: assurance operacional

A comparação SyGuS foi executada contra um corpus público com limites explícitos. O resultado foi pequeno e honesto: 9.719 arquivos examinados, 526 lexicalmente compatíveis, 8 `BOUNDED_VERIFIED` e 518 `UNKNOWN`. Em seguida, a V5 de Critical Assurance foi adaptada aos quatro sinks reais do firmware, com auditoria fail-closed e distinção explícita entre cobertura lexical e dominância interprocedural.

A camada Guardian acrescentou observação, classificação determinística, latch de bloqueio, alerta humano e snapshot de evidência. Um ACK não libera uma ação crítica. O contrato central consolidou essas fronteiras em um único modelo de pesquisa; isso ainda não transforma o comunicador em uma plataforma certificada de segurança.

## O que está comprovado e o que não está

| Categoria | Evidência atual | Limite da alegação |
|---|---|---|
| Núcleo C11 | Testes host, sanitizers e invariantes de integração | Não equivale a prova do hardware físico |
| Criptografia e enlace | Testes independentes de autenticação, replay, expiração e corrupção | Não cobre toda a cadeia de provisionamento físico |
| Máquinas de estado | Verificador independente, síntese limitada e contraexemplos | Resultado relativo ao modelo e à especificação |
| Refinamento | Inclusão finita de traços e rejeição de mapas incompatíveis | Não é refinamento geral de programas |
| Sinks críticos | Quatro sinks reais classificados `COVERED` lexicalmente | Regex não prova dominância interprocedural completa |
| Guardian | Modelo determinístico com 7 testes adversariais | Ainda não é componente embarcado integrado |
| SyGuS | Benchmark público reproduzível e limitado | Cobertura semântica baixa; sem alegação de superioridade |
| Hardware | Perfis C11 e simulador | Alcance, energia, latência, secure boot e ergonomia ainda precisam de bancada |

## Reproduzir

Na raiz do repositório:

```bash
./prove.sh --quiet
make -C research test
make -C research hcae-sink-audit
make -C research guardian-model
```

Para o benchmark externo SyGuS, é necessário possuir localmente o corpus oficial:

```bash
SYGUS_ROOT=/caminho/SyGuS-Org/benchmarks \
  make -C research sygus-subset-benchmark
```

O manifesto [`research/software_provenance_manifest.json`](research/software_provenance_manifest.json) registra hashes dos artefatos declarados. Trata-se de proveniência local e não assinada; não é atestação SLSA, certificação ou garantia completa de supply chain.

## Mapa do repositório

| Área | Conteúdo |
|---|---|
| [`firmware/`](firmware/) | Núcleo C11, trust, memória, interação, protocolo e perfis embarcados |
| [`sim/`](sim/) | Simulador e cenários de comunicação/adversários |
| [`research/`](research/) | Raciocínio simbólico, verificação, síntese, benchmarks e assurance |
| [`tools/`](tools/) | Gates, proveniência, orçamento, mutação e auditorias |
| [`docs/`](docs/) | Especificações, decisões, limites e evidências narrativas |
| [`CONTRIBUTING.md`](CONTRIBUTING.md) | Fluxo de mudança, comandos e disciplina de evidência |
| [`SECURITY.md`](SECURITY.md) | Modelo de segurança, limites e reporte |
| [`docs/INDEX.md`](docs/INDEX.md) | Índice editorial por tema e fase |

## Documentos essenciais

| Tema | Documento |
|---|---|
| Contrato central atual | [`docs/57-CONTRATO-CENTRAL-HERUS.md`](docs/57-CONTRATO-CENTRAL-HERUS.md) |
| Sistemas críticos | [`docs/53-PREVISAO-DEGRADACAO-E-SISTEMAS-CRITICOS.md`](docs/53-PREVISAO-DEGRADACAO-E-SISTEMAS-CRITICOS.md) |
| Síntese e verificação | [`docs/54-MVP-SINTESE-VERIFICACAO-MAQUINAS-ESTADO.md`](docs/54-MVP-SINTESE-VERIFICACAO-MAQUINAS-ESTADO.md) |
| Assurance V5 | [`docs/55-AUDITORIA-HCAE-V5.md`](docs/55-AUDITORIA-HCAE-V5.md) |
| Guardian e interface humana | [`docs/56-GUARDIAN-INTERFACE-HUMANA.md`](docs/56-GUARDIAN-INTERFACE-HUMANA.md) |
| Álgebra e protocolo | [`docs/01-ALGEBRA.md`](docs/01-ALGEBRA.md) · [`docs/02-PROTOCOL.md`](docs/02-PROTOCOL.md) |
| Firmware e construção | [`docs/05-FIRMWARE.md`](docs/05-FIRMWARE.md) · [`docs/03-BUILD-GUIDE.md`](docs/03-BUILD-GUIDE.md) |
| Produto | [`docs/38-PRODUTO-DESEJAVEL-E-ADOCAO.md`](docs/38-PRODUTO-DESEJAVEL-E-ADOCAO.md) |
| Arquitetura finita | [`docs/48-ARQUITETURA-FINITA-E-LINGUAGEM.md`](docs/48-ARQUITETURA-FINITA-E-LINGUAGEM.md) |
| Intent Compiler | [`docs/50-INTENT-COMPILER-E-SEMANTIC-IR.md`](docs/50-INTENT-COMPILER-E-SEMANTIC-IR.md) |

O índice completo separa documentos normativos, produto, firmware, pesquisa paradigmática, sistemas críticos e arquivo histórico. Documentos antigos permanecem disponíveis para rastreabilidade, mas não competem com o contrato central nesta página.

## Próximo marco

Há duas linhas de evolução. No produto, o próximo marco é hardware real para medir RF, energia, interação, armazenamento e experiência do comunicador. Na pesquisa crítica, é fechar a ponte entre intenção e implementação: substituir a auditoria lexical por AST/grafo de chamadas/dominância, ligar sinks a claims e evidências frescas, medir custo em um perfil C11 próximo do ESP32-S3 e somente então avaliar a integração embarcada do Guardian e da interface humana.

O objetivo de impacto continua ambicioso. O método permanece incremental: **construir, estabilizar, observar, encontrar a brecha, corrigir, medir e preservar**.

## Status
O estado de referência do produto permanece em `main`: comunicador semântico pessoal, memória soberana e documentação de produto. A branch `integration/herus-symbolic-hardening-2026-08`, acompanhada pelo [PR #46](https://github.com/SummaArs/herus/pull/46), contém a expansão de pesquisa em verificação e assurance crítico. O projeto continua pré-hardware para as alegações físicas, e cada resultado positivo é local ao contrato e ao ambiente que o produziu.

## Licença

Proprietary. Copyright © 2026 Gustavo Gonçalves. Todos os direitos reservados — veja [`LICENSE`](LICENSE).
