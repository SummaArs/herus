# Índice editorial do HERUS

Este índice é a forma recomendada de navegar pela documentação. O [README raiz](../README.md) é o ponto de entrada para entender o HERUS como produto comunicador e como pesquisa em expansão. O estado de referência do produto permanece em `main`; a camada de assurance crítico pertence à branch de integração e ao PR #46. Os documentos antigos continuam preservados para rastreabilidade, mas não têm todos o mesmo status normativo.

## 1. Direção atual e pesquisa crítica

| Documento | Papel |
|---|---|
| [`57-CONTRATO-CENTRAL-HERUS.md`](57-CONTRATO-CENTRAL-HERUS.md) | Definição normativa atual, autoridade, finitude, verdictos e critérios de evolução |
| [`53-PREVISAO-DEGRADACAO-E-SISTEMAS-CRITICOS.md`](53-PREVISAO-DEGRADACAO-E-SISTEMAS-CRITICOS.md) | Virada conceitual para verificação e síntese de sistemas críticos |
| [`54-MVP-SINTESE-VERIFICACAO-MAQUINAS-ESTADO.md`](54-MVP-SINTESE-VERIFICACAO-MAQUINAS-ESTADO.md) | Especificação do MVP de síntese, verificação e refinamento |
| [`55-AUDITORIA-HCAE-V5.md`](55-AUDITORIA-HCAE-V5.md) | Auditoria e integração da proposta Critical Assurance Engine V5 |
| [`56-GUARDIAN-INTERFACE-HUMANA.md`](56-GUARDIAN-INTERFACE-HUMANA.md) | Guardian, risco determinístico, alertas e fronteira humana |
| [`58-AUDITORIA-CAMINHOS-CRITICOS.md`](58-AUDITORIA-CAMINHOS-CRITICOS.md) | Auditoria estrutural limitada de callers e handoffs críticos |
| [`59-REFINAMENTO-DE-POLITICAS.md`](59-REFINAMENTO-DE-POLITICAS.md) | Ligação formal entre políticas abstratas e concretas |
| [`60-CERTIFICADO-COMPOSTO-ASSURANCE.md`](60-CERTIFICADO-COMPOSTO-ASSURANCE.md) | Composição auditável de verificações, refinamentos e caminhos críticos |
| [`61-CASO-ASSURANCE-REPRODUZIVEL.md`](61-CASO-ASSURANCE-REPRODUZIVEL.md) | Fixture, runner, certificado serializado e mutações reproduzíveis |
| [`62-ASSURANCE-MEMORY-VAULT-REAL.md`](62-ASSURANCE-MEMORY-VAULT-REAL.md) | Primeiro caso composto derivado de um subsistema real do firmware |
| [`63-EXTRACAO-ESTRUTURAL-MEMORY-VAULT.md`](63-EXTRACAO-ESTRUTURAL-MEMORY-VAULT.md) | Comparação automática limitada entre C11 real e caso declarativo |

## 2. Produto e experiência

| Documento | Papel |
|---|---|
| [`04-PRODUCT.md`](04-PRODUCT.md) | Tese de produto, comunicação semântica e posicionamento original |
| [`38-PRODUTO-DESEJAVEL-E-ADOCAO.md`](38-PRODUTO-DESEJAVEL-E-ADOCAO.md) | HERUS One, Núcleo/Dock, adoção e valor individual |
| [`47-HERUS-INDISPENSAVEL-E-INTELIGENCIA-PROPRIA.md`](47-HERUS-INDISPENSAVEL-E-INTELIGENCIA-PROPRIA.md) | Revisão estratégica de produto e inteligência local |
| [`06-NUCLEO.md`](06-NUCLEO.md) | Papel do Núcleo e evolução de computação local |
| [`07-VOZ-HAPTICA.md`](07-VOZ-HAPTICA.md) | Voz e háptica como interface local |
| [`08-RUNTIME-INTERACAO.md`](08-RUNTIME-INTERACAO.md) | Runtime de interação e autoridade humana |

## 3. Firmware, protocolo e segurança

| Documento | Papel |
|---|---|
| [`05-FIRMWARE.md`](05-FIRMWARE.md) | Arquitetura C11, custos, errata e limites pré-hardware |
| [`02-PROTOCOL.md`](02-PROTOCOL.md) | HCP, Weave, Beat, criptografia e orçamento de airtime |
| [`03-BUILD-GUIDE.md`](03-BUILD-GUIDE.md) | Construção, bancada e critérios de interrupção |
| [`09-VALIDACAO-FISICA.md`](09-VALIDACAO-FISICA.md) | Plano de validação física |
| [`11-GATEWAY-CONFIANCA.md`](11-GATEWAY-CONFIANCA.md) | Gateway e confiança |
| [`12-ENLACE-CORE-NUCLEO.md`](12-ENLACE-CORE-NUCLEO.md) | Enlace autenticado entre Core e Núcleo |
| [`13-CICLO-DE-CONFIANCA.md`](13-CICLO-DE-CONFIANCA.md) | Ciclo de confiança, pareamento e revogação |
| [`25-MODELO-AMEACAS-EXECUTAVEL.md`](25-MODELO-AMEACAS-EXECUTAVEL.md) | Modelo de ameaças executável |
| [`29-PROVENIENCIA-LOCAL-BUILD.md`](29-PROVENIENCIA-LOCAL-BUILD.md) | Proveniência local e limites de atestação |
| [`51-HARDENING-CI-E-VALIDACAO.md`](51-HARDENING-CI-E-VALIDACAO.md) | Gates de validação e disciplina de CI |
| [`SECURITY.md`](../SECURITY.md) | Status de segurança, lacunas físicas e reporte |

## 4. Memória seletiva e recuperação

| Documento | Papel |
|---|---|
| [`17-MEMORIA-SELETIVA.md`](17-MEMORIA-SELETIVA.md) | Política de memória mínima e consentida |
| [`18-SESSAO-CAPTURA-MEMORIA.md`](18-SESSAO-CAPTURA-MEMORIA.md) | Sessão física de captura |
| [`19-EXTRACAO-CANDIDATOS.md`](19-EXTRACAO-CANDIDATOS.md) | Extração local e tipada de candidatos |
| [`20-COFRE-MEMORIA.md`](20-COFRE-MEMORIA.md) | Cofre cifrado e autorização de escrita |
| [`21-CONSOLIDACAO-HUMANA.md`](21-CONSOLIDACAO-HUMANA.md) | Revisão e consolidação humana |
| [`22-RECUPERACAO-SEMANTICA.md`](22-RECUPERACAO-SEMANTICA.md) | Recuperação finita e abstencionista |
| [`23-INTERFACE-RECUPERACAO-HUMANA.md`](23-INTERFACE-RECUPERACAO-HUMANA.md) | Apresentação segura do resultado |
| [`24-GRAND-FINALE-MEMORIA.md`](24-GRAND-FINALE-MEMORIA.md) | Composição da cadeia de memória |
| [`26-COLECAO-MEMORIA.md`](26-COLECAO-MEMORIA.md) | Coleção multi-cartão |
| [`27-INDICE-PRIVADO-COLECAO.md`](27-INDICE-PRIVADO-COLECAO.md) | Índice privado e consulta tipada |
| [`28-RECUPERACAO-TRANSACIONAL.md`](28-RECUPERACAO-TRANSACIONAL.md) | Recuperação após interrupção |
| [`30-GRAND-FINALE-COLECAO.md`](30-GRAND-FINALE-COLECAO.md) | Composição multi-cartão |
| [`31-SESSAO-FISICA-PROPOSITO.md`](31-SESSAO-FISICA-PROPOSITO.md) | Sessões vinculadas a propósito |
| [`32-RECUPERACAO-RESERVA-SESSAO.md`](32-RECUPERACAO-RESERVA-SESSAO.md) | Recuperação de reservas pós-reboot |
| [`33-QUARENTENA-BOOT-SESSAO.md`](33-QUARENTENA-BOOT-SESSAO.md) | Quarentena e reconstrução do gate |
| [`34-GRAN-FINALE-PRE-HARDWARE.md`](34-GRAN-FINALE-PRE-HARDWARE.md) | Composição pré-hardware |
| [`35-PROVA-DE-FOGO-HOST.md`](35-PROVA-DE-FOGO-HOST.md) | Campanha hostil de recuperação e bootstrap |

## 5. Paradigma simbólico e linguagem finita

| Documento | Papel |
|---|---|
| [`01-ALGEBRA.md`](01-ALGEBRA.md) | Álgebra, limites e decisões de implementação |
| [`48-ARQUITETURA-FINITA-E-LINGUAGEM.md`](48-ARQUITETURA-FINITA-E-LINGUAGEM.md) | Vocabulário finito, memória por regras e papel futuro de LLM local |
| [`49-DESAFIO-RACIOCINIO-GENERATIVO.md`](49-DESAFIO-RACIOCINIO-GENERATIVO.md) | Limites e hipótese do raciocínio simbólico generativo |
| [`50-INTENT-COMPILER-E-SEMANTIC-IR.md`](50-INTENT-COMPILER-E-SEMANTIC-IR.md) | Compilação de intenção e Semantic IR |
| [`52-RACIOCINIO-LIVRE-SIMBOLICO.md`](52-RACIOCINIO-LIVRE-SIMBOLICO.md) | Pesquisa de saturação, tipos e hipóteses simbólicas |
| [`40-LLM-LOCAL-ESP32-E-HERUS.md`](40-LLM-LOCAL-ESP32-E-HERUS.md) | Orçamento e limites de LLM local |
| [`41-PROPOSTA-TIPADA-E-INVARIANTES.md`](41-PROPOSTA-TIPADA-E-INVARIANTES.md) | Fronteiras tipadas entre modelo, memória e autoridade |

O código correspondente está em [`research/`](../research/), especialmente `free_reasoner/`, `generative_lab/`, `semantic_ir.py`, `critical_state_verifier.py`, `critical_state_synthesis.py` e `critical_state_refinement.py`.

## 6. Histórico, pesquisa e evidências

| Recurso | Papel |
|---|---|
| [`44-ATLAS-NODE-APRENDIZADOS.md`](44-ATLAS-NODE-APRENDIZADOS.md) | Aprendizados de uma arquitetura ESP32/BLE/rádio externa |
| [`10-INVESTIGACAO-PREREGISTRADA.md`](10-INVESTIGACAO-PREREGISTRADA.md) | Estudos e critérios pré-registrados |
| [`16-GRAND-FINALE.md`](16-GRAND-FINALE.md) | Consolidação histórica anterior |
| [`research/evidence/`](../research/evidence/) | Evidências de rodadas, benchmarks e limites |
| [`research/software_provenance_manifest.json`](../research/software_provenance_manifest.json) | Ledger de hashes e proveniência local |
| [`research/datasets_manifest.json`](../research/datasets_manifest.json) | Proveniência e limites de datasets |
| [`ANALISE_CRITICA_HERUS.md`](../ANALISE_CRITICA_HERUS.md) | Auditoria crítica histórica; não é o status atual |

## 7. Como ler

Para entender o HERUS como produto, leia primeiro `README.md`, `04-PRODUCT.md`, `38-PRODUTO-DESEJAVEL-E-ADOCAO.md`, `06-NUCLEO.md` e `05-FIRMWARE.md`. Para entender a expansão de pesquisa da branch de integração, leia depois `57-CONTRATO-CENTRAL-HERUS.md`, `53-PREVISAO-DEGRADACAO-E-SISTEMAS-CRITICOS.md`, `54-MVP-SINTESE-VERIFICACAO-MAQUINAS-ESTADO.md`, `55-AUDITORIA-HCAE-V5.md` e `56-GUARDIAN-INTERFACE-HUMANA.md`.

Para entender o produto, leia `04-PRODUCT.md` e `38-PRODUTO-DESEJAVEL-E-ADOCAO.md`. Para entender o firmware, leia `05-FIRMWARE.md`, `02-PROTOCOL.md`, `51-HARDENING-CI-E-VALIDACAO.md` e `SECURITY.md`. Para reproduzir a pesquisa, use o `Makefile` em `research/` e os comandos do README.

Documentos históricos não são apagados. A regra editorial é simples: o README descreve o presente; o índice classifica o acervo; os documentos normativos registram contratos; as evidências registram o que foi realmente executado.
