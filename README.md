# HERUS

**Adaptive Symbiotic Architecture (ASA): uma infraestrutura finita de significado, assurance e coordenação verificável.**

O HERUS é um núcleo computacional que pode **habitar diferentes hospedeiros**, descobrir capacidades e restrições observáveis, adaptar sua representação dentro de um orçamento finito e propor coordenação sem transformar descoberta em autoridade.

O hospedeiro inicial será um wearable de pulso. Ele não define o HERUS inteiro: é o primeiro corpo físico no qual a arquitetura será medida. Outros hospedeiros podem ser um celular, notebook, servidor, robô, sensor, rede ou ambiente externo de evidência. Cada hospedeiro precisa ser identificado, observado e validado separadamente.

> **O HERUS não confunde capacidade com autoridade, proposta com execução, dados com evidência ou conexão com confiança.**

## O que o HERUS é agora

O HERUS possui um núcleo comum e uma primeira aplicação pessoal.

O **ASA Core** mantém uma identidade persistente, vincula-se a um hospedeiro por um perfil verificável, observa um mundo local, compõe Skills finitas e opera sob contratos de autoridade explícitos. O mesmo núcleo pode ser desvinculado e rebindado a outro hospedeiro sem carregar automaticamente o contexto, as Skills ou a autoridade do hospedeiro anterior.

O **primeiro produto físico** é um comunicador pessoal de pulso. Ele oferece interação curta, confirmação física, memória seletiva, comunicação essencial e feedback háptico. O wearable é o primeiro hospedeiro do simbionte, não a definição completa da arquitetura.

A arquitetura também foi exercitada com dados reais locais em três hospedeiros de evidência: um gateway semântico, um auditor do corpus MIntRec e um observador financeiro baseado em artefato OFR. Nos três casos, o HERUS propôs; nenhum executou.

## Como a simbiose funciona

```text
identificar o hospedeiro
        ↓
observar capacidades e restrições
        ↓
registrar evidência e proveniência
        ↓
negociar uma representação finita
        ↓
compor ou sintetizar uma Skill limitada
        ↓
verificar casos visíveis e adversariais
        ↓
propor uma coordenação
        ↓
executar somente com autoridade externa comprovada
```

Quando o HERUS muda de hospedeiro, o `herus_id` permanece, mas o vínculo físico e o contexto local mudam. O `rebind` limpa o mundo observado e o repertório de Skills do hospedeiro anterior. Deriva de perfil, digest inválido, ambiguidade, replay, orçamento excedido ou autoridade ausente levam a `ABSTAIN`.

Um celular ou notebook pode atuar como hospedeiro auxiliar de pesquisa ou computação. A Internet pode fornecer evidência, mas nunca autoridade implícita. O HERUS não executa código baixado nem transforma texto externo em permissão.

## Princípios

| Princípio | Consequência prática |
|---|---|
| **Adaptação verificável** | O núcleo pode aprender sobre um ambiente, mas precisa registrar fonte, digest, orçamento e limite. |
| **Identidade separada do hospedeiro** | O simbionte continua o mesmo; capacidades e restrições pertencem ao vínculo atual. |
| **Rebind sem vazamento** | Memória contextual e Skills do hospedeiro anterior não atravessam automaticamente a troca. |
| **Proposta separada de execução** | Uma Skill pode gerar uma proposta sem possuir permissão para executá-la. |
| **Fail-closed** | Incerteza, deriva, ausência de evidência ou autoridade insuficiente bloqueiam o avanço. |
| **Significado antes de mensagem** | O núcleo trabalha com intenções, estados e cartões de contexto finitos. |
| **Pessoa no comando** | Memória, transmissão, compra, publicação, controle e ação externa exigem a autoridade definida pelo contrato. |
| **Conectar não significa confiar** | USB, BLE, LoRa, Wi‑Fi e Internet são canais; nenhum canal concede autoridade sozinho. |

## Hospedeiros e papéis

| Hospedeiro | Papel do ASA |
|---|---|
| **Wearable de pulso** | Primeiro corpo físico: gesto, confirmação, estado, comunicação essencial e LRA. |
| **Celular ou notebook** | Hospedeiro auxiliar para pesquisa, gateway, armazenamento ou computação delegada. |
| **Robô ou dispositivo legado** | Ambiente de descoberta: observar sensores e atuadores antes de qualquer proposta de controle. |
| **Servidor** | Ambiente de observação, saúde, rollback e canary sob contrato explícito. |
| **Dados financeiros** | Ambiente de observação e sandbox; negociação, transferência e aconselhamento ativo permanecem proibidos. |
| **Internet** | Fonte externa potencial de evidência; nunca uma autoridade do HERUS. |

Esses papéis são ambientes de validação, não modos fechados do ASA. A pergunta central é sempre: **o que foi observado, qual evidência existe, o que ainda falta e qual ação pode ser proposta sem ultrapassar o contrato?**

## Evidência atual

A etapa host-only demonstrou:

- identidade persistente entre hospedeiros;
- descoberta e perfis finitos de capacidades;
- proveniência por digest de artefatos reais locais;
- simbiose com comunicação, dados multimodais e observação financeira;
- `rebind` com limpeza do contexto e das Skills anteriores;
- síntese e verificação de Skills sem promoção automática de autoridade;
- bloqueio de execução sem autorização externa;
- contratos de memória, intenção, rádio, confiança, recuperação e assurance;
- bench simulado com adversários e distância.

A suíte atual passou com **286 testes Python**, além do gate integral de firmware, simulação e invariantes. O resultado host-only não prova energia, latência, rádio, temperatura, memória física, ergonomia, LRA ou segurança de um dispositivo real.

## Estado atual: freeze pré-hardware

O repositório está em `pre_hardware_frozen`. Isso significa que a arquitetura host-only foi endurecida e que novas alegações relevantes devem depender de medições físicas. Não serão publicados percentuais de consumo, autonomia ou desempenho embarcado derivados apenas de simulação.

O primeiro hospedeiro físico é o LilyGO T3-S3 com rádio SX1262-915 MHz. O trabalho físico começa pelos gates B1 e B2:

| Gate | Objetivo |
|---|---|
| **B1 — Identificação** | Confirmar placa, revisão, variante de rádio, componentes e pinagem. |
| **B2 — Boot** | Confirmar gravação, boot serial, reset, identidade e leitura local. |
| **B3–B5** | Medir memória, latência, comunicação e descoberta real. |
| **B6–B8** | Integrar LRA, feedback, energia e interação humana. |
| **B9–B10** | Repetir adaptação, deriva, renegociação e ponte com hospedeiros auxiliares. |

Até B1/B2 passarem, o manifesto permanece `pre_hardware` e o firmware físico só poderá operar nos modos `OBSERVE`, `PROPOSE` e `HAPTIC_FEEDBACK`.

## Documentação principal

| Documento | Conteúdo |
|---|---|
| [Arquitetura ASA](docs/62-SYMBIOTIC-ARCHITECTURE.md) | Definição formal da Adaptive Symbiotic Architecture. |
| [Modelo triplo e identidade persistente](docs/65-MODELO-TRIPLO-E-IDENTIDADE-PERSISTENTE.md) | World Model, Host Model, Self Model e identidade. |
| [Prova real multi-hospedeiro](docs/67-PROVA-SIMBIOSE-REAL-MULTI-HOST.md) | Evidência usando comunicação, MIntRec e OFR. |
| [Freeze pré-hardware](docs/68-FREEZE-PRE-HARDWARE.md) | Limite explícito do que pode avançar sem placa física. |
| [Critério GOFAI](docs/64-CRITERIO-SUPERACAO-GOFAI.md) | Critério falsificável de adaptação contra baseline simbólico. |
| [Descoberta de hospedeiros](docs/59-AUTONOMOUS-HOST-DISCOVERY.md) | Sondagem finita sem perfil completo pré-fornecido. |
| [Arquitetura finita e linguagem](docs/48-ARQUITETURA-FINITA-E-LINGUAGEM.md) | Vocabulário finito e papel futuro de uma LLM local. |
| [API do Symbiont v2](docs/51-API-SIMBIONTE-V2.md) | Núcleo host-only para identidade, descoberta, verificação e transferência entre hospedeiros. |
| [Definição de simbiose útil](docs/52-DEFINICAO-SIMBIOSE-UTIL.md) | Contrato congelado que separa mecanismo técnico de benefício humano mensurável. |
| [Entrada em hardware](research/evidence/hardware_entry_gate_v1.md) | Gates normativos B1–B10. |
| [Manifesto de prontidão](research/evidence/hardware_readiness_manifest.json) | Estado da placa, instrumentação e gates físicos. |
| [Lista de compra](research/evidence/hardware_purchase_list_v1.md) | Componentes do primeiro hospedeiro. |

## Verificação local

```bash
./prove.sh --quiet
PYTHONPATH=research python3 -m unittest discover -s research -p 'test_*.py'
```

Um resultado positivo confirma contratos de software e bench controlado. Não constitui prova de desempenho físico, autonomia, alcance de rádio ou segurança de campo.

## O que o HERUS ainda não é

O HERUS ainda não é uma AGI, não resolve NLU/NLG aberto, não substitui modelos de linguagem atuais e não possui adaptação comprovada a qualquer dispositivo. VSA/HDC permanece restrito ao vocabulário finito dos cartões de contexto. A memória é governada por regras e estados tipados. Uma LLM local futura poderá atuar como camada linguística somente depois de orçamento medido em hardware real e sem autoridade sobre memória, rádio ou confirmação física.

A ambição é ampla; as afirmações permanecem proporcionais às evidências. O próximo salto não é uma nova frase no README. É a placa real, a medição real e a capacidade de falhar corretamente diante de um hospedeiro físico.

## Licença

Proprietary. Copyright © 2026 Gustavo Gonçalves. Todos os direitos reservados — veja [LICENSE](LICENSE).
